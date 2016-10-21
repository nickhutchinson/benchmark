
#undef NDEBUG
#include <utility>
#include "benchmark/benchmark.h"
#include "output_test.h"

// ========================================================================= //
// ---------------------- Testing Prologue Output -------------------------- //
// ========================================================================= //

ADD_CASES(TC_ConsoleOut,
          {TestCase("^Benchmark %s Time %s CPU %s Iterations$", MR_Next),
           TestCase("^[-]+$", MR_Next)});
ADD_CASES(TC_CSVOut, {TestCase("name,iterations,real_time,cpu_time,time_unit,"
                               "bytes_per_second,items_per_second,"
                               "label,error_occurred,error_message")});

// ========================================================================= //
// ------------------------ Testing Basic Output --------------------------- //
// ========================================================================= //

void BM_basic(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_basic);

ADD_CASES(TC_ConsoleOut, {TestCase("^BM_basic %console_report$")});
ADD_CASES(TC_JSONOut, {TestCase("\"name\": \"BM_basic\",$"),
                       TestCase("\"iterations\": %int,$", MR_Next),
                       TestCase("\"real_time\": %int,$", MR_Next),
                       TestCase("\"cpu_time\": %int,$", MR_Next),
                       TestCase("\"time_unit\": \"ns\"$", MR_Next),
                       TestCase("}", MR_Next)});
ADD_CASES(TC_CSVOut, {TestCase("^\"BM_basic\",%csv_report$")});

// ========================================================================= //
// ------------------------ Testing Error Output --------------------------- //
// ========================================================================= //

void BM_error(benchmark::State& state) {
  state.SkipWithError("message");
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_error);
ADD_CASES(TC_ConsoleOut, {TestCase("^BM_error[ ]+ERROR OCCURRED: 'message'$")});
ADD_CASES(TC_JSONOut, {TestCase("\"name\": \"BM_error\",$"),
                       TestCase("\"error_occurred\": true,$", MR_Next),
                       TestCase("\"error_message\": \"message\",$", MR_Next)});

ADD_CASES(TC_CSVOut, {TestCase("^\"BM_error\",,,,,,,,true,\"message\"$")});

// ========================================================================= //
// ----------------------- Testing Complexity Output ----------------------- //
// ========================================================================= //

void BM_Complexity_O1(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Complexity_O1)->Range(1, 1 << 18)->Complexity(benchmark::o1);
SET_SUBSTITUTIONS({{"%bigOStr", "[ ]*[0-9]+\\.[0-9]+ \\([0-9]+\\)"},
                   {"%RMS", "[ ]*[0-9]+ %"}});
ADD_CASES(TC_ConsoleOut,
          {TestCase("^BM_Complexity_O1_BigO %bigOStr %bigOStr[ ]*$"),
           TestCase("^BM_Complexity_O1_RMS %RMS %RMS[ ]*$")});

// ========================================================================= //
// ----------------------- Testing Aggregate Output ------------------------ //
// ========================================================================= //

// Test that non-aggregate data is printed by default
void BM_Repeat(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_Repeat)->Repetitions(3);
ADD_CASES(TC_ConsoleOut,
          {TestCase("^BM_Repeat/repeats:3 %console_report$"),
           TestCase("^BM_Repeat/repeats:3 %console_report$"),
           TestCase("^BM_Repeat/repeats:3 %console_report$"),
           TestCase("^BM_Repeat/repeats:3_mean %console_report$"),
           TestCase("^BM_Repeat/repeats:3_stddev %console_report$")});
ADD_CASES(TC_JSONOut, {TestCase("\"name\": \"BM_Repeat/repeats:3\",$"),
                       TestCase("\"name\": \"BM_Repeat/repeats:3\",$"),
                       TestCase("\"name\": \"BM_Repeat/repeats:3\",$"),
                       TestCase("\"name\": \"BM_Repeat/repeats:3_mean\",$"),
                       TestCase("\"name\": \"BM_Repeat/repeats:3_stddev\",$")});
ADD_CASES(TC_CSVOut,
          {TestCase("^\"BM_Repeat/repeats:3\",%csv_report$"),
           TestCase("^\"BM_Repeat/repeats:3\",%csv_report$"),
           TestCase("^\"BM_Repeat/repeats:3\",%csv_report$"),
           TestCase("^\"BM_Repeat/repeats:3_mean\",%csv_report$"),
           TestCase("^\"BM_Repeat/repeats:3_stddev\",%csv_report$")});

// Test that a non-repeated test still prints non-aggregate results even when
// only-aggregate reports have been requested
void BM_RepeatOnce(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_RepeatOnce)->Repetitions(1)->ReportAggregatesOnly();
ADD_CASES(TC_ConsoleOut,
          {TestCase("^BM_RepeatOnce/repeats:1 %console_report$")});
ADD_CASES(TC_JSONOut, {TestCase("\"name\": \"BM_RepeatOnce/repeats:1\",$")});
ADD_CASES(TC_CSVOut, {TestCase("^\"BM_RepeatOnce/repeats:1\",%csv_report$")});

// Test that non-aggregate data is not reported
void BM_SummaryRepeat(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_SummaryRepeat)->Repetitions(3)->ReportAggregatesOnly();
ADD_CASES(TC_ConsoleOut,
          {TestCase(".*BM_SummaryRepeat/repeats:3 ", MR_Not),
           TestCase("^BM_SummaryRepeat/repeats:3_mean %console_report$"),
           TestCase("^BM_SummaryRepeat/repeats:3_stddev %console_report$")});
ADD_CASES(TC_JSONOut,
          {TestCase(".*BM_SummaryRepeat/repeats:3 ", MR_Not),
           TestCase("\"name\": \"BM_SummaryRepeat/repeats:3_mean\",$"),
           TestCase("\"name\": \"BM_SummaryRepeat/repeats:3_stddev\",$")});
ADD_CASES(TC_CSVOut,
          {TestCase(".*BM_SummaryRepeat/repeats:3 ", MR_Not),
           TestCase("^\"BM_SummaryRepeat/repeats:3_mean\",%csv_report$"),
           TestCase("^\"BM_SummaryRepeat/repeats:3_stddev\",%csv_report$")});

// ========================================================================= //
// --------------------------- TEST CASES END ------------------------------ //
// ========================================================================= //

int main(int argc, char* argv[]) { RunOutputTests(argc, argv); }
