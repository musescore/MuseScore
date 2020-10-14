#!/usr/bin/env bash

echo "Run MuseScore mtest"
trap 'echo Run tests failed; exit 1' ERR

. ./../musescore_environment.sh

cd build.release/mtest

# vnc is the only tested platform plugin that allows to run
# mscore executable in the used Travis environment.
export QT_QPA_PLATFORM=vnc
export ASAN_OPTIONS=detect_leaks=0

make -j2

xvfb-run -a ctest -j2 --output-on-failure
