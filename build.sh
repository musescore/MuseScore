#!/usr/bin/env bash

export PATH=$HOME/build_tools/qt598/bin:$PATH 
export LD_LIBRARY_PATH=$HOME/build_tools/qt598/lib:$LD_LIBRARY_PATH

rm -rf ./build.release 
make portable  