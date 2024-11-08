find_package(benchmark REQUIRED)
find_package(GTest REQUIRED)
add_executable(tinydb_benchmark benchmark.cxx)
target_link_libraries(tinydb_benchmark PRIVATE tinydb_compile_opts
                                               benchmark::benchmark_main GTest::gtest)
