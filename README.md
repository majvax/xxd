# xxd

Convert a binary file into a C++ source snippet: a `constexpr std::array<std::byte, N>` and a matching `SIZE` constant. Useful for embedding firmware, assets, or test fixtures directly into your program.

This is a tiny, modern C++ replacement for the classic idea behind `xxd`, producing ready-to-include C++23 code.

## Features

- Outputs a `constexpr std::array<std::byte, N> name = { … }`
- Also emits `constexpr size_t NAME_SIZE = N` (uppercased name + `_SIZE`)
- Preserves raw bytes (no formatting conversions)
- Simple CLI with `-i`, `-o`, and `-n` flags
- Single static binary; third-party deps are fetched automatically via CPM.cmake

## Build

Requirements:

- CMake ≥ 3.30
- A C++23-capable toolchain with ranges support, including `std::views::enumerate` and `std::ranges::to`
  
> [!NOTE]
> tested with GCC 15.2.0, clang 20.1.8, MSVC 19.44.35213.0

The project uses CPM.cmake to fetch dependencies (`fmt`, `spdlog`, `cxxopts`) at configure time.

On Windows PowerShell:

```powershell
# from the repo root
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
# binary will be placed in .\bin\xxd.exe
```

On Linux/macOS (optional):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# binary appears in ./bin/xxd
```

## Usage

```text
Convert a binary file to a C++ array
Usage:
	xxd [OPTION...]

	-h, --help        Show help
	-i, --input arg   Input file
	-o, --output arg  Output file
	-n, --name arg    Name of the generated array (default: data)
    -c, --compress    Compress output using Brotli (default: false)
```

### Example

Generate a header from a binary file:

```powershell
# produce includeable header with array name "logo"
xxd.exe -i ./assets/logo.png -o ./include/logo.hpp -n logo
```

Sample of the generated output (truncated):

```cpp
// Generated from majvax/xxd
#include <array>
#include <cstddef>

constexpr size_t LOGO_SIZE = 1234;

constexpr std::array<std::byte, LOGO_SIZE> logo = {
		std::byte{ 0x89 }, std::byte{ 0x50 }, std::byte{ 0x4E }, std::byte{ 0x47 },
		// ...
};
```

Then, in your code:

```cpp
#include "logo.hpp"

void use_data() {
		auto image = load_from_memory(logo.data(), LOGO_SIZE);
        ...
}
```

> [!NOTE]
> Large binaries will generate large source files and may slow your C++ compilation

## Dependencies

- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [cxxopts](https://github.com/jarro2783/cxxopts)

Fetched automatically via CPM.cmake at configure time; no manual setup required.


## Future Work
- [x] Add support for compressed output (e.g., gzip) to reduce binary size
- [ ] Add the decompression into the generated code
- [ ] Add support for multiple compressing algorithms (e.g., Brotli, zlib)
- [ ] Support more output formats (e.g., C-style structs)
- [ ] Easy integration with CMake for automatic header generation

## License

This project is licensed under the MIT License. See the [LICENSE.md](LICENSE.md) file for details.
