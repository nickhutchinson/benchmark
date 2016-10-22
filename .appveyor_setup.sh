#!/bin/bash
set -euo pipefail
set -o xtrace

if ! type innoextract >/dev/null 2>&1; then
    mkdir -p _deps
    pushd _deps
    curl -fLO http://constexpr.org/innoextract/files/innoextract-1.6-windows.zip
    7z x innoextract-1.6-windows.zip -obin innoextract.exe
    innoextract --version
    rm innoextract-1.6-windows.zip
    popd
fi

if ! type ninja >/dev/null 2>&1; then
    mkdir -p _deps
    pushd _deps
    curl -fLO https://github.com/ninja-build/ninja/releases/download/v1.7.1/ninja-win.zip
    7z x ninja-win.zip -obin ninja.exe
    ninja --version
    rm ninja-win.zip
    popd
fi

if [[ $COMPILER =~ msvc-16 && ! -d "_deps/boost_1_56_0-msvc-10.0-64" ]]; then
    mkdir -p _deps
    pushd _deps
    curl -fLO 'http://netix.dl.sourceforge.net/project/boost/boost-binaries/1.56.0/boost_1_56_0-msvc-10.0-64.exe'

    # Only extract headers and libraries so it completes quicker. (It's not
    # perfect, because innoextract doesn't appear to let you only etract
    # particular directories. But it does seem to cut down on the amount of
    # unwanted docs/source files we waste time extracting.)
    innoextract boost_1_56_0-msvc-10.0-64.exe -I boost -I lib64-msvc-10.0 -s -p
    rm boost_1_56_0-msvc-10.0-64.exe

    # innoextract writes out a folder called 'app'. Rename it to something useful.
    mv app boost_1_56_0-msvc-10.0-64

    # Try to keep under Appveyor cache limit by removing anything we don't need.
    # - documentation and source files
    # - dlls and import libs
    # - remove libs linked against static CRT
    pushd boost_1_56_0-msvc-10.0-64
    rm -fr doc libs tools
    rm -fr lib64-msvc-10.0/boost*
    rm -fr lib64-msvc-10.0/*vc100-{s,mt-s,sgd,mt-sgd}-*
    popd

    popd
fi
