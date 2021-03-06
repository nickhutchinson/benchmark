#include "output_test.h"
#include <iostream>
#include <map>
#include <sstream>
#include "../src/check.h"  // NOTE: check.h is for internal use only!
#include "../src/re.h"     // NOTE: re.h is for internal use only

#ifndef BENCHMARK_NO_CXX11
#include <memory>
#else
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#endif

// ========================================================================= //
// ------------------------------ Internals -------------------------------- //
// ========================================================================= //
#ifndef BENCHMARK_NO_CXX11
using std::shared_ptr;
using std::make_shared;
#else
using boost::shared_ptr;
using boost::make_shared;
#endif

namespace internal {
namespace {

typedef std::vector<TestCase> TestCaseList;

// Use a vector because the order elements are added matters during iteration.
// std::map/unordered_map don't guarantee that.
// For example:
//  SetSubstitutions({{"%HelloWorld", "Hello"}, {"%Hello", "Hi"}});
//     Substitute("%HelloWorld") // Always expands to Hello.
typedef std::vector<std::pair<std::string, std::string> > SubMap;

TestCaseList& GetTestCaseList(TestCaseID ID) {
  // Uses function-local statics to ensure initialization occurs
  // before first use.
  static TestCaseList lists[TC_NumID];
  return lists[ID];
}

SubMap& GetSubstitutions() {
  // Don't use 'dec_re' from header because it may not yet be initialized.
  static std::string dec_re = "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?";
  static SubMap map;
  if (map.empty()) {
    map.push_back(
        std::make_pair("%float", "[0-9]*[.]?[0-9]+([eE][-+][0-9]+)?"));
    map.push_back(std::make_pair("%int", "[ ]*[0-9]+"));
    map.push_back(std::make_pair(" %s ", "[ ]+"));
    map.push_back(std::make_pair("%time", "[ ]*[0-9]{1,5} ns"));
    map.push_back(std::make_pair(
        "%console_report", "[ ]*[0-9]{1,5} ns [ ]*[0-9]{1,5} ns [ ]*[0-9]+"));
    map.push_back(std::make_pair("%console_us_report",
                                 "[ ]*[0-9] us [ ]*[0-9] us [ ]*[0-9]+"));
    map.push_back(std::make_pair(
        "%csv_report", "[0-9]+," + dec_re + "," + dec_re + ",ns,,,,,"));
    map.push_back(std::make_pair(
        "%csv_us_report", "[0-9]+," + dec_re + "," + dec_re + ",us,,,,,"));
    map.push_back(std::make_pair(
        "%csv_bytes_report",
        "[0-9]+," + dec_re + "," + dec_re + ",ns," + dec_re + ",,,,"));
    map.push_back(std::make_pair(
        "%csv_items_report",
        "[0-9]+," + dec_re + "," + dec_re + ",ns,," + dec_re + ",,,"));
    map.push_back(std::make_pair("%csv_label_report_begin",
                                 "[0-9]+," + dec_re + "," + dec_re + ",ns,,,"));
    map.push_back(std::make_pair("%csv_label_report_end", ",,"));
  }
  return map;
}

std::string PerformSubstitutions(std::string source) {
  SubMap const& subs = GetSubstitutions();
  typedef std::string::size_type SizeT;
  foreach (SubMap::value_type const& KV, subs) {
    SizeT pos;
    SizeT next_start = 0;
    while ((pos = source.find(KV.first, next_start)) != std::string::npos) {
      next_start = pos + KV.second.size();
      source.replace(pos, KV.first.size(), KV.second);
    }
  }
  return source;
}

void CheckCase(std::stringstream& remaining_output, TestCase const& TC,
               TestCaseList const& not_checks) {
  std::string first_line;
  bool on_first = true;
  std::string line;
  while (remaining_output.eof() == false) {
    CHECK(remaining_output.good());
    std::getline(remaining_output, line);
    if (on_first) {
      first_line = line;
      on_first = false;
    }
    foreach (const TestCase& NC, not_checks) {
      CHECK(!NC.regex->Match(line))
          << "Unexpected match for line \"" << line << "\" for MR_Not regex \""
          << NC.regex_str << "\""
          << "\n    actual regex string \"" << TC.substituted_regex << "\""
          << "\n    started matching near: " << first_line;
    }
    if (TC.regex->Match(line)) return;
    CHECK(TC.match_rule != MR_Next)
        << "Expected line \"" << line << "\" to match regex \"" << TC.regex_str
        << "\""
        << "\n    actual regex string \"" << TC.substituted_regex << "\""
        << "\n    started matching near: " << first_line;
  }
  CHECK(remaining_output.eof() == false)
      << "End of output reached before match for regex \"" << TC.regex_str
      << "\" was found"
      << "\n    actual regex string \"" << TC.substituted_regex << "\""
      << "\n    started matching near: " << first_line;
}

void CheckCases(TestCaseList const& checks, std::stringstream& output) {
  std::vector<TestCase> not_checks;
  for (size_t i = 0; i < checks.size(); ++i) {
    const TestCase& TC = checks[i];
    if (TC.match_rule == MR_Not) {
      not_checks.push_back(TC);
      continue;
    }
    CheckCase(output, TC, not_checks);
    not_checks.clear();
  }
}

class TestReporter : public benchmark::BenchmarkReporter {
 public:
  TestReporter(std::vector<benchmark::BenchmarkReporter*> reps)
      : reporters_(reps) {}

  virtual bool ReportContext(const Context& context) {
    bool last_ret = false;
    bool first = true;
    foreach (benchmark::BenchmarkReporter* rep, reporters_) {
      bool new_ret = rep->ReportContext(context);
      CHECK(first || new_ret == last_ret)
          << "Reports return different values for ReportContext";
      first = false;
      last_ret = new_ret;
    }
    return last_ret;
  }

  void ReportRuns(const std::vector<Run>& report) {
    foreach (benchmark::BenchmarkReporter* rep, reporters_)
      rep->ReportRuns(report);
  }

  void Finalize() {
    foreach (benchmark::BenchmarkReporter* rep, reporters_)
      rep->Finalize();
  }

 private:
  std::vector<benchmark::BenchmarkReporter*> reporters_;
};
}
}  // end namespace internal

// ========================================================================= //
// -------------------------- Public API Definitions------------------------ //
// ========================================================================= //

TestCase::TestCase(std::string re, int rule)
    : regex_str(re),
      match_rule(rule),
      substituted_regex(internal::PerformSubstitutions(regex_str)),
      regex(make_shared<benchmark::Regex>()) {
  std::string err_str;
  regex->Init(substituted_regex, &err_str);
  CHECK(err_str.empty()) << "Could not construct regex \"" << substituted_regex
                         << "\""
                         << "\n    originally \"" << regex_str << "\""
                         << "\n    got error: " << err_str;
}

int AddCases(TestCaseID ID, const TestCase il[], size_t N) {
  internal::TestCaseList& L = internal::GetTestCaseList(ID);
  L.insert(L.end(), il, il + N);
  return 0;
}

int SetSubstitutions(const std::string il[][2], size_t N) {
  internal::SubMap& subs = internal::GetSubstitutions();
  for (size_t i = 0; i < N; ++i) {
    bool exists = false;
    std::string KV[2] = {il[i][0], internal::PerformSubstitutions(il[i][1])};
    foreach (internal::SubMap::value_type& EKV, subs) {
      if (EKV.first == KV[0]) {
        EKV.second = KV[1];
        exists = true;
        break;
      }
    }
    if (!exists) subs.push_back(std::make_pair(KV[0], KV[1]));
  }
  return 0;
}

struct ReporterTest {
  const char* name;
  std::vector<TestCase>& output_cases;
  std::vector<TestCase>& error_cases;
  benchmark::BenchmarkReporter& reporter;
  shared_ptr<std::stringstream> out_stream;
  shared_ptr<std::stringstream> err_stream;

  ReporterTest(const char* n, std::vector<TestCase>& out_tc,
               std::vector<TestCase>& err_tc, benchmark::BenchmarkReporter& br)
      : name(n),
        output_cases(out_tc),
        error_cases(err_tc),
        reporter(br),
        out_stream(new std::stringstream),
        err_stream(new std::stringstream) {
    reporter.SetOutputStream(out_stream.get());
    reporter.SetErrorStream(err_stream.get());
  }
};

void RunOutputTests(int argc, char* argv[]) {
  using internal::GetTestCaseList;
  benchmark::Initialize(&argc, argv);
  benchmark::ConsoleReporter CR(benchmark::ConsoleReporter::OO_None);
  benchmark::JSONReporter JR;
  benchmark::CSVReporter CSVR;
  ReporterTest TestCases[] = {
      ReporterTest("ConsoleReporter", GetTestCaseList(TC_ConsoleOut),
                   GetTestCaseList(TC_ConsoleErr), CR),
      ReporterTest("JSONReporter", GetTestCaseList(TC_JSONOut),
                   GetTestCaseList(TC_JSONErr), JR),
      ReporterTest("CSVReporter", GetTestCaseList(TC_CSVOut),
                   GetTestCaseList(TC_CSVErr), CSVR),
  };

  // Create the test reporter and run the benchmarks.
  std::cout << "Running benchmarks...\n";

  std::vector<benchmark::BenchmarkReporter*> reporters;
  reporters.push_back(&CR);
  reporters.push_back(&JR);
  reporters.push_back(&CSVR);
  internal::TestReporter test_rep(reporters);
  benchmark::RunSpecifiedBenchmarks(&test_rep);

  foreach (ReporterTest& rep_test, TestCases) {
    std::string msg = std::string("\nTesting ") + rep_test.name + " Output\n";
    std::string banner(msg.size() - 1, '-');
    std::cout << banner << msg << banner << "\n";

    std::cerr << rep_test.err_stream->str();
    std::cout << rep_test.out_stream->str();

    internal::CheckCases(rep_test.error_cases, *rep_test.err_stream);
    internal::CheckCases(rep_test.output_cases, *rep_test.out_stream);

    std::cout << "\n";
  }
}
