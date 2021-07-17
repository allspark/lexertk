//
// Created by allspark on 17/07/2021.
//

#include <benchmark/benchmark.h>

#include <lexertk/generator.hpp>

#include "lexertk_original.hpp"


static void BM_RefactoredLexer(benchmark::State& state) {
  constexpr static std::string_view expression = "{a+(b-[c*(e/{f+g}-h)*i]%[j+(k-{l*m}/n)+o]-p)*q}";

  lexertk::generator generator;

  // Perform setup here
  for (auto _ : state) {
    generator.process(expression);

    benchmark::DoNotOptimize(std::move(generator).get_token_list());
    benchmark::ClobberMemory();
  }
}

static void BM_OriginalLexer(benchmark::State& state) {
  std::string expression = "{a+(b-[c*(e/{f+g}-h)*i]%[j+(k-{l*m}/n)+o]-p)*q}";

  original::lexertk::generator generator;

  // Perform setup here
  for (auto _ : state) {
    generator.process(expression);

    benchmark::DoNotOptimize(generator);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_OriginalLexer);
BENCHMARK(BM_RefactoredLexer);

// Run the benchmark
BENCHMARK_MAIN();
