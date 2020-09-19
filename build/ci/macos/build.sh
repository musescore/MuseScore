#!/usr/bin/env bash

echo "Build MuseScore"

BUILD_MODE=$1
BUILD_UI_MU4=OFF
if [ ${BUILD_MODE} == "mu4" ] 
then 
    BUILD_UI_MU4=ON
fi 

make -f Makefile.osx ci BUILD_UI_MU4=${BUILD_UI_MU4}