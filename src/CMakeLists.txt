add_executable(xxd
    main.cpp
)

target_link_libraries(xxd
    PRIVATE
    fmt::fmt
    spdlog::spdlog
    cxxopts::cxxopts
    brotlienc
)
