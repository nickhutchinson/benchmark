#!/bin/bash
set -euo pipefail

if [[ ! -d "_deps/boost_1_56_0-msvc-10.0-64" ]]; then
    set -o xtrace

    mkdir -p _deps
    pushd _deps
    cinst innoextract
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
    # - remove libs linked against static CRt
    pushd boost_1_56_0-msvc-10.0-64
    rm -fr doc libs tools
    rm -fr lib64-msvc-10.0/boost*
    rm -fr lib64-msvc-10.0/*vc100-{s,mt-s,sgd,mt-sgd}-*
    popd

    popd

    set +o xtrace
fi
