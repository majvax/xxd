cmake_minimum_required(VERSION 3.30)


if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)


project(
    xxd
    VERSION 1.1.0
    DESCRIPTION "A C++ implementation of xxd for c++17 style hex dumps"
    HOMEPAGE_URL "https://github.com/majvax/xxd"
    LANGUAGES CXX
)


set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Explicitly disable automatic C++20 module scanning to avoid injecting
# unsupported experimental flags like -fmodules-ts with some GCC/MinGW builds.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(Dependencies.cmake)
include(cmake/version.cmake)

# Add source tree
add_subdirectory(src)
