#!/bin/bash
# Run this script from MuseScore's root directory

set -e # exit on error
set -x # echo commands

make revision
make debug CPUS=2 PREFIX="$HOME/software"
make installdebug CPUS=2 PREFIX="$HOME/software"
