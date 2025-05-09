#!/usr/bin/env bash
echo "Run MuseScore mtest"
trap 'echo Run tests failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ENV=$BUILD_TOOLS/environment.sh
source $ENV

cd build.debug/mtest

# run the mtests in "minimal" platform for headless systems
# enable fonts handling
export QT_QPA_PLATFORM=minimal:enable_fonts
# if AddressSanitizer was used, disable leak detection
export ASAN_OPTIONS=detect_leaks=0:new_delete_type_mismatch=0

make -j2

df -h .

ctest -j2 --output-on-failure

df -h .
