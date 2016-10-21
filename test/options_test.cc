#include "benchmark/benchmark_api.h"

#if !defined(BENCHMARK_NO_CXX11) && \
    (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L)
#include <chrono>
#include <mutex>
#include <thread>
namespace chrono = std::chrono;
namespace this_thread = std::this_thread;
#else
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
namespace chrono = boost::chrono;
namespace this_thread = boost::this_thread;
#endif
void BM_basic(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}

void BM_basic_slow(benchmark::State& state) {
  chrono::milliseconds sleep_duration(state.range(0));
  while (state.KeepRunning()) {
    this_thread::sleep_for(
        chrono::duration_cast<chrono::nanoseconds>(sleep_duration));
  }
}

BENCHMARK(BM_basic);
BENCHMARK(BM_basic)->Arg(42);
BENCHMARK(BM_basic_slow)->Arg(10)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_basic_slow)->Arg(100)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_basic_slow)->Arg(1000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_basic)->Range(1, 8);
BENCHMARK(BM_basic)->RangeMultiplier(2)->Range(1, 8);
BENCHMARK(BM_basic)->DenseRange(10, 15);
#ifndef BENCHMARK_NO_CXX11
BENCHMARK(BM_basic)->Args({42, 42});
BENCHMARK(BM_basic)->Ranges({{64, 512}, {64, 512}});
#else
BENCHMARK(BM_basic)->ArgPair(42, 42);
BENCHMARK(BM_basic)->RangePair(64, 512, 64, 512);
#endif  // BENCHMARK_NO_CXX11
BENCHMARK(BM_basic)->MinTime(0.7);
BENCHMARK(BM_basic)->UseRealTime();
BENCHMARK(BM_basic)->ThreadRange(2, 4);
BENCHMARK(BM_basic)->ThreadPerCpu();
BENCHMARK(BM_basic)->Repetitions(3);

void CustomArgs(benchmark::internal::Benchmark* b) {
  for (int i = 0; i < 10; ++i) {
    b->Arg(i);
  }
}

BENCHMARK(BM_basic)->Apply(CustomArgs);

BENCHMARK_MAIN()
