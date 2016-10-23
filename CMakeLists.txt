cmake_minimum_required (VERSION 2.8.11)
project (benchmark)

foreach(p
    CMP0042 # MACOSX_RPATH is enabled by default.
    CMP0054 # CMake 3.1
    CMP0056 # export EXE_LINKER_FLAGS to try_run
    )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." ON)
option(BENCHMARK_ENABLE_LTO "Enable link time optimisation of the benchmark library." OFF)
option(BENCHMARK_ENABLE_CXX11 "Whether to enable C++ mode" ON)
option(BENCHMARK_USE_LIBCXX "Build and test using libc++ as the standard library." OFF)
# Make sure we can import out CMake functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Read the git tags to determine the project version
include(GetGitVersion)
get_git_version(GIT_VERSION)

# Tell the user what versions we are using
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})
message("-- Version: ${VERSION}")

# The version of the libraries
set(GENERIC_LIB_VERSION ${VERSION})
string(SUBSTRING ${VERSION} 0 1 GENERIC_LIB_SOVERSION)

# Save build-tree executable artefacts in a common location, e.g. so unit test
# binaries on Windows can locate benchmark.dll adjacent to them.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${benchmark_BINARY_DIR}")

# Import our CMake modules
include(CheckCXXCompilerFlag)
include(AddCXXCompilerFlag)
include(CXXFeatureCheck)

if (MSVC)
  # Turn compiler warnings up to 11
  string(REGEX REPLACE "[-/]W[1-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(
      -wd4251  # suppress: 'class 'X' needs to have dll-interface to be used by
               # clients of class 'Y'; this almost always going to be a noisy
               # false positive due to member variables from the STL.
  )

  # Link time optimisation
  if (BENCHMARK_ENABLE_LTO)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GL")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")

    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /GL")
    set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG")
  endif()
else()
  # Try and enable C++11. Don't use C++14 because it doesn't work in some
  # configurations.
  if (BENCHMARK_ENABLE_CXX11)
    add_cxx_compiler_flag(-std=c++11)
    if (NOT HAVE_CXX_FLAG_STD_CXX11)
      add_cxx_compiler_flag(-std=c++0x)
    endif()
  endif ()

  # Turn compiler warnings up to 11
  add_cxx_compiler_flag(-Wall)

  add_cxx_compiler_flag(-Wextra)
  add_cxx_compiler_flag(-Wshadow)
  add_cxx_compiler_flag(-Werror RELEASE)
  add_cxx_compiler_flag(-Werror RELWITHDEBINFO)
  add_cxx_compiler_flag(-Werror MINSIZEREL)
  if (BENCHMARK_ENABLE_CXX11 OR NOT CMAKE_CXX_COMPILER_ID MATCHES GNU)
    # With GCC, C++03 and -pedantic, invoking macros with no parameters triggers
    # an unkillable warning.
    add_cxx_compiler_flag(-pedantic)
    add_cxx_compiler_flag(-pedantic-errors)
  endif ()
  add_cxx_compiler_flag(-Wshorten-64-to-32)
  add_cxx_compiler_flag(-Wfloat-equal)
  if (NOT BENCHMARK_ENABLE_CXX11)
    add_cxx_compiler_flag(-Wno-variadic-macros)
    add_cxx_compiler_flag(-Wno-long-long)
  endif ()
  add_cxx_compiler_flag(-fstrict-aliasing)
  if (BENCHMARK_ENABLE_CXX11 AND NOT BENCHMARK_USE_LIBCXX)
    add_cxx_compiler_flag(-Wzero-as-null-pointer-constant)
  endif()
  if (HAVE_CXX_FLAG_FSTRICT_ALIASING)
    add_cxx_compiler_flag(-Wstrict-aliasing)
  endif()
  add_cxx_compiler_flag(-Wthread-safety)
  if (HAVE_CXX_FLAG_WTHREAD_SAFETY)
    cxx_feature_check(THREAD_SAFETY_ATTRIBUTES)
  endif()

  # On most UNIX like platforms g++ and clang++ define _GNU_SOURCE as a
  # predefined macro, which turns on all of the wonderful libc extensions.
  # However g++ doesn't do this in Cygwin so we have to define it ourselfs
  # since we depend on GNU/POSIX/BSD extensions.
  if (CYGWIN)
    add_definitions(-D_GNU_SOURCE=1)
  endif()

  # Link time optimisation
  if (BENCHMARK_ENABLE_LTO)
    add_cxx_compiler_flag(-flto)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
      find_program(GCC_AR gcc-ar)
      if (GCC_AR)
        set(CMAKE_AR ${GCC_AR})
      endif()
      find_program(GCC_RANLIB gcc-ranlib)
      if (GCC_RANLIB)
        set(CMAKE_RANLIB ${GCC_RANLIB})
      endif()
    endif()
  endif()

  # Coverage build type
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the C++ compiler during coverage builds."
    FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used for linking binaries during coverage builds."
    FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the shared libraries linker during coverage builds."
    FORCE)
  mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
  set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
    FORCE)
  add_cxx_compiler_flag(--coverage COVERAGE)
endif()

if (NOT BENCHMARK_ENABLE_CXX11)
  # When we don't have C++11 available, we fall back to using equivalents from
  # Boost.
  add_definitions(-DBENCHMARK_NO_CXX11)

  find_package(Boost REQUIRED COMPONENTS atomic thread chrono system)

  add_definitions(-DBOOST_THREAD_VERSION=4) # Use more C++11-conforming behaviour
  include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
  link_libraries(${Boost_LIBRARIES})

  if (CMAKE_SYSTEM_NAME MATCHES Windows)
    add_definitions(-DBOOST_ALL_NO_LIB)
  elseif (CMAKE_SYSTEM_NAME MATCHES Linux)
    link_libraries(-lrt)
  endif ()
endif ()

add_cxx_compiler_flag(-fvisibility=hidden)

# `-fvisibility-inlines-hidden` was buggy in < GCC 4.2. We do not try adding it
# in this case. See e.g. https://bugs.launchpad.net/ubuntu/+source/gcc-4.1/+bug/109262
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
    OR NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 4.2)
  add_cxx_compiler_flag(-fvisibility-inlines-hidden)
endif ()

if (BENCHMARK_USE_LIBCXX)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_cxx_compiler_flag(-stdlib=libc++)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
          "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    add_cxx_compiler_flag(-nostdinc++)
    message("libc++ header path must be manually specified using CMAKE_CXX_FLAGS")
    # Adding -nodefaultlibs directly to CMAKE_<TYPE>_LINKER_FLAGS will break
    # configuration checks such as 'find_package(Threads)'
    list(APPEND BENCHMARK_CXX_LINKER_FLAGS -nodefaultlibs)
    # -lc++ cannot be added directly to CMAKE_<TYPE>_LINKER_FLAGS because
    # linker flags appear before all linker inputs and -lc++ must appear after.
    list(APPEND BENCHMARK_CXX_LIBRARIES c++)
  else()
    message(FATAL "-DBENCHMARK_USE_LIBCXX:BOOL=ON is not supported for compiler")
  endif()
endif(BENCHMARK_USE_LIBCXX)

# C++ feature checks
# Determine the correct regular expression engine to use
cxx_feature_check(STD_REGEX)
cxx_feature_check(GNU_POSIX_REGEX)
cxx_feature_check(POSIX_REGEX)
if(NOT HAVE_STD_REGEX AND NOT HAVE_GNU_POSIX_REGEX AND NOT HAVE_POSIX_REGEX)
  message(FATAL_ERROR "Failed to determine the source files for the regular expression backend")
endif()

cxx_feature_check(CXX11_NULLPTR)
cxx_feature_check(CXX11_RANGE_BASED_FOR)
cxx_feature_check(CXX11_STATIC_ASSERT)
cxx_feature_check(STEADY_CLOCK)
# Ensure we have pthreads
find_package(Threads REQUIRED)

# Set up directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build the targets
add_subdirectory(src)

if (BENCHMARK_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
