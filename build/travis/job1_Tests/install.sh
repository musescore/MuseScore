#!/bin/bash
# Run this script from MuseScore's root directory

set -e # exit on error
set -x # echo commands

make revision
export CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer"
# make debug CPUS=2 PREFIX="$HOME/software" COVERAGE=ON
make installdebug CPUS=2 PREFIX="$HOME/software" COVERAGE=ON    # make debug is already called by installdebug as a dependency
