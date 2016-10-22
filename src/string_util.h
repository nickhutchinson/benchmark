#ifndef BENCHMARK_STRING_UTIL_H_
#define BENCHMARK_STRING_UTIL_H_

#include <cstdarg>
#include <sstream>
#include <string>
#include <utility>
#include "internal_macros.h"

namespace benchmark {

void AppendHumanReadable(int n, std::string* str);

std::string HumanReadableNumber(double n);

std::string StringPrintF(const char* format, ...);
std::string StringPrintF(const char* msg, va_list args);

#ifndef BENCHMARK_NO_CXX11
inline std::ostream& StringCatImp(std::ostream& out) BENCHMARK_NOEXCEPT {
  return out;
}

template <class First, class... Rest>
inline std::ostream& StringCatImp(std::ostream& out, First&& f,
                                  Rest&&... rest) {
  out << std::forward<First>(f);
  return StringCatImp(out, std::forward<Rest>(rest)...);
}

template <class... Args>
inline std::string StrCat(Args&&... args) {
  std::ostringstream ss;
  StringCatImp(ss, std::forward<Args>(args)...);
  return ss.str();
}
#else

inline std::string StrCat() { return std::string(); }

template <class T>
inline std::string StrCat(const T& t) {
  std::ostringstream ss;
  ss << t;
  return ss.str();
}

template <class T, class U>
inline std::string StrCat(const T& t, const U& u) {
  std::ostringstream ss;
  ss << t << u;
  return ss.str();
}
template <class T, class U, class V>
inline std::string StrCat(const T& t, const U& u, const V& v) {
  std::ostringstream ss;
  ss << t << u << v;
  return ss.str();
}

template <class T, class U, class V, class W>
inline std::string StrCat(const T& t, const U& u, const V& v, const W& w) {
  std::ostringstream ss;
  ss << t << u << v << w;
  return ss.str();
}
#endif

void ReplaceAll(std::string* str, const std::string& from,
                const std::string& to);

}  // end namespace benchmark

#endif  // BENCHMARK_STRING_UTIL_H_
