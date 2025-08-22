#include <cctype>     // for std::toupper
#include <cstddef>    // for std::byte
#include <cstdint>    // for std::uint8_t, std::uintmax_t
#include <filesystem> // for std::filesystem::exists, std::filesystem::file_size
#include <fstream>    // for std::ifstream, std::ofstream
#include <iomanip>    // for std::setw, std::setfill
#include <iterator>   // for std::istreambuf_iterator
#include <ranges>     // for std::ranges::subrange, std::views::take, std::views::transform
#include <string>     // for std::string
#include <vector>     // for std::vector

#include <cxxopts.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <xxd/version.hpp>

#include "compress.hpp"

int main(const int argc, const char** argv)
{
    cxxopts::Options options("xxd", fmt::format("Convert a binary file to a C++ array"));
    // clang-format off
    options.add_options()
        ("h,help", "Show help")
        ("v,version", "Show version", cxxopts::value<bool>()->default_value("false"))
        ("i,input", "Input file", cxxopts::value<std::string>())
        ("o,output", "Output file", cxxopts::value<std::string>())
        ("n,name", "Name of the generated array", cxxopts::value<std::string>()->default_value("data"))
        ("c,compress", "Compress the output using Brotli", cxxopts::value<bool>()->default_value("false"));
    // clang-format on


    const auto result = options.parse(argc, argv);

    if (result.count("version") != 0U) {
        fmt::print("xxd version {}\n", xxd::version);
        return 0;
    }

    if ((result.count("help") != 0U) || argc < 4) {
        fmt::print("{}", options.help());
        return 0;
    }

    const auto source_filename = result["input"].as<std::string>();
    if (source_filename.empty()) {
        spdlog::error("Input file name is empty");
        return 1;
    }

    const auto compress = result["compress"].as<bool>();
    if (compress) {
        spdlog::info("Compression enabled");
    } else {
        spdlog::info("Compression disabled");
    }

    if (!std::filesystem::exists(source_filename)) {
        spdlog::error("File does not exist: {}", source_filename);
        return 1;
    };

    const auto file_length{ std::filesystem::file_size(source_filename) };
    spdlog::info("File size: {} bytes", file_length);

    std::ifstream source_file{ source_filename, std::ios::binary };
    if (!source_file) {
        spdlog::critical("Error opening file: {}", source_filename);
        return 1;
    }

    using it_t = std::istreambuf_iterator<char>;

    // clang-format off
    const auto bytes = std::ranges::subrange{ it_t{ source_file }, it_t{} }
                       | std::views::take(file_length)
                       | broli::compress(compress)
                       | std::views::transform([](auto chr) { return std::byte{ static_cast<std::uint8_t>(chr) }; })
                       | std::ranges::to<std::vector<std::byte>>();
    // clang-format on

    const auto length = compress ? bytes.size() : file_length;
    if (compress) {
        spdlog::info("Compressed {} bytes to {} bytes ({:.2f}%) ",
          file_length,
          length,
          static_cast<float>(length) / static_cast<float>(file_length) * 100.0F);
    }

    if (!compress && bytes.size() != length) {
        spdlog::warn("Read {} bytes, expected {}", bytes.size(), length);
    }

    const auto output_filename = result["output"].as<std::string>();
    if (output_filename.empty()) {
        spdlog::error("Output file name is empty");
        return 1;
    }

    std::ofstream output_file(output_filename);
    if (!output_file) {
        spdlog::critical("Error opening output file: {}", output_filename);
        return 1;
    }

    const auto array_name = result["name"].as<std::string>();
    if (array_name.empty()) {
        spdlog::error("Array name is empty");
        return 1;
    }

    const auto array_name_upper = (array_name | std::views::transform([](unsigned char chr) {
        return std::toupper(chr);
    }) | std::ranges::to<std::string>())
                                  + "_SIZE";

    output_file << "// Generated from majvax/xxd\n";
    output_file << "#include <array>\n";
    output_file << "#include <cstddef>\n\n";
    output_file << "constexpr size_t " << array_name_upper << " = " << length << ";\n\n";

    output_file << "constexpr std::array<std::byte, " << array_name_upper << "> " << array_name
                << " = {\n    ";

    for (auto [index, byte] : bytes | std::views::enumerate) {
        if (index > 0) { output_file << ", "; }
        if (index % 4 == 0 && index > 0) { output_file << "\n    "; }
        const auto value = static_cast<uint8_t>(byte);
        output_file << "std::byte{ 0x" << std::hex << std::setw(2) << std::setfill('0') << +value
                    << " }";
    }
    output_file << std::dec << "};\n";
    output_file.close();

    spdlog::info("Output written to: {}", output_filename);

    return 0;
}
