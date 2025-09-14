#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <vector>

#include "prometheus/detail/time_window_max.h"

static const auto ITERATIONS = 1 << 20;

static void BM_Time_Window_Max_Record(benchmark::State& state) {
  using prometheus::detail::TimeWindowMax;
  int buffer_length = 3;
  auto expiry = std::chrono::milliseconds(3000);
  TimeWindowMax twm{expiry, buffer_length};

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> d(0, 1000000);

  while (state.KeepRunning()) {
    auto observation = d(gen);
    auto start = std::chrono::high_resolution_clock::now();
    twm.Record(observation);
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_seconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(elapsed_seconds.count());
  }
}
BENCHMARK(BM_Time_Window_Max_Record)->Iterations(ITERATIONS)->UseManualTime();

static void BM_Time_Window_Max_Get(benchmark::State& state) {
  using prometheus::detail::TimeWindowMax;
  int buffer_length = 3;
  auto expiry = std::chrono::milliseconds(3000);
  TimeWindowMax twm{expiry, buffer_length};
  twm.Record(42);

  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(twm.Get());
  }
}
BENCHMARK(BM_Time_Window_Max_Get)->Iterations(ITERATIONS);
