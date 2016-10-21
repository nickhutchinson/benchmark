#ifndef BENCHMARK_TIMERS_H
#define BENCHMARK_TIMERS_H

#include <string>

#ifndef BENCHMARK_NO_CXX11
#include <chrono>
#else
#include <boost/chrono.hpp>
#endif

namespace benchmark {

// Return the CPU usage of the current process
double ProcessCPUUsage();

// Return the CPU usage of the children of the current process
double ChildrenCPUUsage();

// Return the CPU usage of the current thread
double ThreadCPUUsage();

#ifndef BENCHMARK_NO_CXX11
#if defined(HAVE_STEADY_CLOCK)
template <bool HighResIsSteady = std::chrono::high_resolution_clock::is_steady>
struct ChooseSteadyClock {
  typedef std::chrono::high_resolution_clock type;
};

template <>
struct ChooseSteadyClock<false> {
  typedef std::chrono::steady_clock type;
};
#endif

struct ChooseClockType {
#if defined(HAVE_STEADY_CLOCK)
  typedef ChooseSteadyClock<>::type type;
#else
  typedef std::chrono::high_resolution_clock type;
#endif
};
#endif  // BENCHMARK_NO_CXX11

inline double ChronoClockNow() {
#ifndef BENCHMARK_NO_CXX11
  typedef ChooseClockType::type ClockType;
  typedef std::chrono::duration<double, std::chrono::seconds::period> FpSeconds;
#else
  typedef boost::chrono::high_resolution_clock
      ClockType;  // selects either steady_clock or system_clock
  typedef boost::chrono::duration<double, boost::chrono::seconds::period>
      FpSeconds;
#endif  // BENCHMARK_NO_CXX11
  return FpSeconds(ClockType::now().time_since_epoch()).count();
}

std::string LocalDateTimeString();

}  // end namespace benchmark

#endif  // BENCHMARK_TIMERS_H
