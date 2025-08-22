include(cmake/CPM.cmake)



cpmaddpackage("gh:fmtlib/fmt#11.2.0")
cpmaddpackage(URI "gh:gabime/spdlog@1.15.3" OPTIONS "SPDLOG_FMT_EXTERNAL ON")
cpmaddpackage("gh:jarro2783/cxxopts@3.1.1")
cpmaddpackage(URI "gh:google/brotli@1.1.0" OPTIONS "BROTLI_DISABLE_TESTS ON" "BUILD_SHARED_LIBS OFF")
