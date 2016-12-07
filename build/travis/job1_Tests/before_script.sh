#!/bin/bash

set -e # exit on error
set -x # echo commands

cd build.debug/mtest
#ninja # ninja is always from build directory (i.e. build.debug), never from a subdirectory
