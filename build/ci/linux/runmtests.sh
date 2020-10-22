#!/usr/bin/env bash

echo "Run MuseScore mtest"
trap 'echo Run tests failed; exit 1' ERR

. ./../musescore_environment.sh

cd build.debug/mtest

# run the mtests in "minimal" platform for headless systems
# enable fonts handling
export QT_QPA_PLATFORM=minimal:enable_fonts
# if AddressSanitizer was used, disable leak detection
export ASAN_OPTIONS=detect_leaks=0

make -j2

ctest -j2 --output-on-failure
