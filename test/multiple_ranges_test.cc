#include "benchmark/benchmark.h"

#include <cassert>
#include <set>

class MultipleRangesFixture : public ::benchmark::Fixture {
 public:
  MultipleRangesFixture() {
#define ENTRY(a, b, c)            \
  do {                            \
    std::vector<int> entry;       \
    entry.push_back(a);           \
    entry.push_back(b);           \
    entry.push_back(c);           \
    expectedValues.insert(entry); \
  } while (0)
    ENTRY(1, 3, 5);
    ENTRY(1, 3, 8);
    ENTRY(1, 3, 15);
    ENTRY(2, 3, 5);
    ENTRY(2, 3, 8);
    ENTRY(2, 3, 15);
    ENTRY(1, 4, 5);
    ENTRY(1, 4, 8);
    ENTRY(1, 4, 15);
    ENTRY(2, 4, 5);
    ENTRY(2, 4, 8);
    ENTRY(2, 4, 15);
    ENTRY(1, 7, 5);
    ENTRY(1, 7, 8);
    ENTRY(1, 7, 15);
    ENTRY(2, 7, 5);
    ENTRY(2, 7, 8);
    ENTRY(2, 7, 15);
    ENTRY(7, 6, 3);
#undef ENTRY
  }

  void SetUp(const ::benchmark::State& state) {
    std::vector<int> ranges;
    ranges.push_back(state.range(0));
    ranges.push_back(state.range(1));
    ranges.push_back(state.range(2));

    assert(expectedValues.find(ranges) != expectedValues.end());

    actualValues.insert(ranges);
  }

  virtual ~MultipleRangesFixture() {
    assert(actualValues.size() == expectedValues.size());
  }

  std::set<std::vector<int> > expectedValues;
  std::set<std::vector<int> > actualValues;
};

BENCHMARK_DEFINE_F(MultipleRangesFixture, Empty)(benchmark::State& state) {
  while (state.KeepRunning()) {
    int product = state.range(0) * state.range(1) * state.range(2);
    for (int x = 0; x < product; x++) {
      benchmark::DoNotOptimize(x);
    }
  }
}

#ifndef BENCHMARK_NO_CXX11
BENCHMARK_REGISTER_F(MultipleRangesFixture, Empty)
    ->RangeMultiplier(2)
    ->Ranges({{1, 2}, {3, 7}, {5, 15}})
    ->Args({7, 6, 3});
#endif

void BM_CheckDefaultArgument(benchmark::State& state) {
  // Test that the 'range()' without an argument is the same as 'range(0)'.
  assert(state.range() == state.range(0));
  assert(state.range() != state.range(1));
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_CheckDefaultArgument)
#ifndef BENCHMARK_NO_CXX11
    ->Ranges({{1, 5}, {6, 10}});
#else
    ->RangePair(1, 5, 6, 10);
#endif

static void BM_MultipleRanges(benchmark::State& st) {
  while (st.KeepRunning()) {
  }
}
BENCHMARK(BM_MultipleRanges)
#ifndef BENCHMARK_NO_CXX11
    ->Ranges({{5, 5}, {6, 6}});
#else
    ->RangePair(5, 5, 6, 6);
#endif

BENCHMARK_MAIN()
