#ifndef BENCHMARK_INTERNAL_MACROS_H_
#define BENCHMARK_INTERNAL_MACROS_H_

#include "benchmark/macros.h"

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__GNUC__)
#define COMPILER_GCC
#endif

#if __has_feature(cxx_attributes)
#define BENCHMARK_NORETURN [[noreturn]]
#elif defined(__GNUC__)
#define BENCHMARK_NORETURN __attribute__((noreturn))
#elif defined(COMPILER_MSVC)
#define BENCHMARK_NORETURN __declspec(noreturn)
#else
#define BENCHMARK_NORETURN
#endif

#if defined(__CYGWIN__)
#define BENCHMARK_OS_CYGWIN 1
#elif defined(_WIN32)
#define BENCHMARK_OS_WINDOWS 1
#elif defined(__APPLE__)
// TODO(ericwf) This doesn't actually check that it is a Mac OSX system. Just
// that it is an apple system.
#define BENCHMARK_OS_MACOSX 1
#elif defined(__FreeBSD__)
#define BENCHMARK_OS_FREEBSD 1
#elif defined(__linux__)
#define BENCHMARK_OS_LINUX 1
#endif

#if defined(HAVE_CXX11_RANGE_BASED_FOR)
#define foreach(x, y) for (x : y)
#else
#include <boost/foreach.hpp>
// See the following link for why the namespace is needed:
// http://www.boost.org/doc/libs/1_59_0/doc/html/foreach.html#foreach.introduction.making__literal_boost_foreach__literal__prettier
namespace boost {
namespace BOOST_FOREACH = foreach;
}
#define foreach BOOST_FOREACH
#endif

#if !defined(HAVE_CXX11_NULLPTR)
#undef nullptr
#define nullptr NULL
#endif

#if !defined(HAVE_CXX11_STATIC_ASSERT)
#include <boost/static_assert.hpp>
#undef static_assert
#if defined(__GNUC__)
#define static_assert(cond, msg) BOOST_STATIC_ASSERT_MSG(( \
    cond), (msg)) __attribute__((unused))
#else
#define static_assert(cond, msg) BOOST_STATIC_ASSERT_MSG((cond), (msg))
#endif
#endif  // HAVE_CXX11_STATIC_ASSERT

#if defined(_MSC_VER) && _MSC_VER < 1900
#define va_copy(dest, src) ((dest) = (src))
#endif

#endif  // BENCHMARK_INTERNAL_MACROS_H_
