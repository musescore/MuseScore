#!/usr/bin/env bash

echo "Build utests"

#set -x
trap 'echo Build failed; exit 1' ERR

df -h .

ARTIFACTS_DIR=build.artifacts
BUILD_NUMBER=42

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

cat ./../musescore_environment.sh
source ./../musescore_environment.sh

echo " "
${CXX} --version 
${CC} --version
echo " "
cmake --version
echo " "
echo "VST3_SDK_PATH: $VST3_SDK_PATH"
if [ -z "$VST3_SDK_PATH" ]; then 
    echo "warning: not set VST3_SDK_PATH, build VST module disabled"
    BUILD_VST=OFF
else
    BUILD_VST=ON
fi

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

make CPUS=2 \
    MUSESCORE_BUILD_CONFIG=dev \
    MUSESCORE_REVISION=$MUSESCORE_REVISION \
    BUILD_NUMBER=$BUILD_NUMBER \
    BUILD_VST=$BUILD_VST \
    VST3_SDK_PATH=$VST3_SDK_PATH \
    BUILD_UNIT_TESTS=ON \
    USE_SYSTEM_FREETYPE=ON \
    utests

df -h .
