#!/usr/bin/env bash

source ./../musescore_environment.sh

cd build.debug

export GTEST_OUTPUT=xml:test-results
export GTEST_COLOR=1

# run the tests in "minimal" platform for headless systems
# enable fonts handling
export QT_QPA_PLATFORM=minimal:enable_fonts

# if AddressSanitizer was used, disable leak detection
export ASAN_OPTIONS=detect_leaks=0:new_delete_type_mismatch=0

ctest -V