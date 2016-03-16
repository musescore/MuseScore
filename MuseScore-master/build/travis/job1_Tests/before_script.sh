#!/bin/bash

set -e # exit on error
set -x # echo commands

cd build.debug/mtest
make -j2
