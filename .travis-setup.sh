#!/usr/bin/env bash

# Install
sudo apt-get install -qq cmake
if [ "$COMPILER" = "g++-4.8" ]; then
    sudo apt-get install -qq gcc-4.8 g++-4.8
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 90
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90
    sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-4.8 90
elif [ "$COMPILER" = "g++-4.6" ]; then
    # Work around bug in 14.04's libstdc++ that breaks
    # this_thread::sleep_for(). See:
    # http://stackoverflow.com/questions/12523122/what-is-glibcxx-use-nanosleep-all-about
    export CXXFLAGS=-D_GLIBCXX_USE_NANOSLEEP
    sudo apt-get install -qq gcc-4.6 g++-4.6
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 90
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.6 90
    sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-4.6 90
elif [ "$COMPILER" = "clang++-3.6" ]; then
    sudo apt-get install -qq clang-3.6
    sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-3.6 90
    sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.6 90

    # Workaround travis prepending a different clang version to PATH.
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
fi

if [ "$BUILD_TYPE" = "Coverage" ]; then
    sudo apt-get install -qq lcov
fi

if [ "$STD" = "c++03" ]; then
    sudo apt-get install -qq    \
        libboost1.55-dev        \
        libboost-atomic1.55-dev \
        libboost-chrono1.55-dev \
        libboost-system1.55-dev \
        libboost-thread1.55-dev
fi
