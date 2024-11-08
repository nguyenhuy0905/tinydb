#include <print>
#include <benchmark/benchmark.h>

namespace bench {

static void test_the_bench(benchmark::State& state) {
    for(auto _ : state) {
        // doesn't do anything, but benchmark API requires the state to be passed in
    }
    std::print("Hello world\n");
}

}

BENCHMARK(bench::test_the_bench);

BENCHMARK_MAIN();
