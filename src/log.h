#ifndef BENCHMARK_LOG_H_
#define BENCHMARK_LOG_H_

#include <iostream>
#include <ostream>

#include "benchmark/macros.h"

namespace benchmark {
namespace internal {

typedef std::basic_ostream<char>&(EndLType)(std::basic_ostream<char>&);

class LogType {
  friend BENCHMARK_API LogType& GetNullLogInstance();
  friend BENCHMARK_API LogType& GetErrorLogInstance();

  // FIXME: Add locking to output.
  template <class Tp>
  friend LogType& operator<<(LogType&, Tp const&);
  friend LogType& operator<<(LogType&, EndLType*);

 private:
  LogType(std::ostream* out) : out_(out) {}
  std::ostream* out_;
  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(LogType);
};

template <class Tp>
LogType& operator<<(LogType& log, Tp const& value) {
  if (log.out_) {
    *log.out_ << value;
  }
  return log;
}

inline LogType& operator<<(LogType& log, EndLType* m) {
  if (log.out_) {
    *log.out_ << m;
  }
  return log;
}

BENCHMARK_API int& LogLevel();
BENCHMARK_API LogType& GetNullLogInstance();
BENCHMARK_API LogType& GetErrorLogInstance();

inline LogType& GetLogInstanceForLevel(int level) {
  if (level <= LogLevel()) {
    return GetErrorLogInstance();
  }
  return GetNullLogInstance();
}

}  // end namespace internal
}  // end namespace benchmark

#define VLOG(x)                                                               \
  (::benchmark::internal::GetLogInstanceForLevel(x) << "-- LOG(" << x << "):" \
                                                                         " ")

#endif
