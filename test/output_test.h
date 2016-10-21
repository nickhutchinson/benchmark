#ifndef TEST_OUTPUT_TEST_H
#define TEST_OUTPUT_TEST_H

#undef NDEBUG
#include "benchmark/benchmark.h"
#include "../src/re.h"
#include <vector>
#include <string>
#include <memory>
#include <utility>

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define ADD_CASES(id, ...)                                                     \
  const TestCase CONCAT(cases, __LINE__)[] = __VA_ARGS__;                      \
  int CONCAT(dummy, __LINE__) BENCHMARK_UNUSED =                               \
      ::AddCases(id, CONCAT(cases, __LINE__))

#define SET_SUBSTITUTIONS(...)                                                 \
  const std::string CONCAT(subs, __LINE__)[][2] = __VA_ARGS__;                 \
  int CONCAT(dummy, __LINE__) BENCHMARK_UNUSED =                               \
      ::SetSubstitutions(CONCAT(subs, __LINE__))

enum MatchRules {
  MR_Default, // Skip non-matching lines until a match is found.
  MR_Next,    // Match must occur on the next line.
  MR_Not      // No line between the current position and the next match matches
              // the regex
};

struct TestCase {
  TestCase(std::string re, int rule = MR_Default);

  std::string regex_str;
  int match_rule;
  std::string substituted_regex;
  std::shared_ptr<benchmark::Regex> regex;
};

enum TestCaseID {
  TC_ConsoleOut,
  TC_ConsoleErr,
  TC_JSONOut,
  TC_JSONErr,
  TC_CSVOut,
  TC_CSVErr,

  TC_NumID // PRIVATE
};

// Add a list of test cases to be run against the output specified by
// 'ID'
int AddCases(TestCaseID ID, const TestCase il[], size_t N);

template <size_t N>
int AddCases(TestCaseID ID, const TestCase (&il)[N]) {
  return AddCases(ID, il, N);
}

// Add or set a list of substitutions to be performed on constructed regex's
// See 'output_test_helper.cc' for a list of default substitutions.
int SetSubstitutions(const std::string il[][2], size_t N);

template <size_t N>
int SetSubstitutions(const std::string (&il)[N][2]) {
  return SetSubstitutions(il, N);
}

// Run all output tests.
void RunOutputTests(int argc, char* argv[]);

// ========================================================================= //
// --------------------------- Misc Utilities ------------------------------ //
// ========================================================================= //

namespace {

const char* const dec_re = "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?";

} //  end namespace


#endif // TEST_OUTPUT_TEST_H
