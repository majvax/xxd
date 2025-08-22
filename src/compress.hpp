#include <cstddef>  // for std::byte
#include <cstdint>  // for std::uint8_t, std::uintmax_t
#include <iterator> // for std::istreambuf_iterator
#include <ranges>   // for std::ranges::subrange, std::views::take, std::views::transform
#include <vector>   // for std::vector

#include "brotli/encode.h"


namespace broli::details {


inline std::vector<std::uint8_t> brotli_compress(std::span<const std::uint8_t> input,
  int quality = BROTLI_DEFAULT_QUALITY,
  int lgwin = BROTLI_DEFAULT_WINDOW,
  BrotliEncoderMode mode = BROTLI_DEFAULT_MODE)
{
    size_t max_out = BrotliEncoderMaxCompressedSize(input.size());
    std::vector<std::uint8_t> out(max_out);
    size_t out_size = max_out;
    if (BrotliEncoderCompress(
          quality, lgwin, mode, input.size(), input.data(), &out_size, out.data())
        == 0) {
        throw std::runtime_error("BrotliEncoderCompress failed");
    }
    out.resize(out_size);
    return out;
}

struct compress_view
{
    explicit constexpr compress_view(bool enabled = true) : enabled(enabled) {}
    bool enabled;
    [[nodiscard]] constexpr auto operator()(bool flag) const -> compress_view
    {
        return compress_view{ flag };
    }

    template<std::ranges::input_range R>
    auto operator()(R&& range) const
    {
        auto view = std::views::all(std::forward<R>(range));
        // Collect input as bytes (unsigned char semantics).
        std::vector<std::uint8_t> input;
        if constexpr (std::ranges::sized_range<decltype(view)>) {
            input.reserve(static_cast<std::size_t>(std::ranges::size(view)));
        }
        for (auto&& x : view) {
            // Accept char, unsigned char, uint8_t, std::byte, etc.
            // We only need to convert to uint8_t for compression.
            if constexpr (std::is_same_v<std::remove_cvref_t<decltype(x)>, std::byte>) {
                input.push_back(static_cast<std::uint8_t>(x));
            } else {
                input.push_back(static_cast<std::uint8_t>(static_cast<unsigned char>(x)));
            }
        }

        std::vector<std::uint8_t> out =
          enabled ? brotli_compress(std::span<const std::uint8_t>{ input }) : input;


        // Return an owning view so it's safe to pipe further.
        return std::ranges::owning_view{ std::move(out) };
    }

    // Make it pipeable: range | broli::compress
    template<class R>
    friend auto operator|(R&& range, const compress_view& self)
    {
        return self(std::forward<R>(range));
    }
};
} // namespace broli::details
namespace broli {
inline constexpr details::compress_view compress;
} // namespace broli
