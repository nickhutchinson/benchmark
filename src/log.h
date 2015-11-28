#ifndef BENCHMARK_LOG_H_
#define BENCHMARK_LOG_H_

#include "internal_macros.h"
#include <ostream>

namespace benchmark {
namespace internal {

BENCHMARK_API int GetLogLevel();
BENCHMARK_API void SetLogLevel(int level);

BENCHMARK_API std::ostream& GetNullLogInstance();
BENCHMARK_API std::ostream& GetErrorLogInstance();

inline std::ostream& GetLogInstanceForLevel(int level) {
  if (level <= GetLogLevel()) {
    return GetErrorLogInstance();
  }
  return GetNullLogInstance();
}

} // end namespace internal
} // end namespace benchmark

#define VLOG(x) (::benchmark::internal::GetLogInstanceForLevel(x) \
                 << "-- LOG(" << x << "): ")

#endif
