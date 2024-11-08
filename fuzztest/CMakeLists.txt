add_executable(tinydb_fuzztest fuzz-test.cxx)

target_compile_options(tinydb_fuzztest
    PRIVATE
    "-fsanitize=fuzzer")
target_link_libraries(tinydb_fuzztest
    PRIVATE
    tinydb_compile_opts
    # tinydb_lib
    -fsanitize=fuzzer)

message("After building, run ${CMAKE_CURRENT_BINARY_DIR}/tinydb_fuzztest")
