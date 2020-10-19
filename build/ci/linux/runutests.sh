#!/usr/bin/env bash

ENV_FILE=./../musescore_environment.sh
cat ${ENV_FILE}
. ${ENV_FILE}

cd build.release
export GTEST_OUTPUT=xml:test-results
export GTEST_COLOR=1
export QT_QPA_PLATFORM=minimal

ctest -V