#!/usr/bin/env bash

BUILD_NUMBER=$1
if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi

OUT_DIR=$2
ARTIFACTS_DIR="build.artifacts" # default output dir

if [ -z "$2" ]; then OUT_DIR=${ARTIFACTS_DIR}/env; fi

export MUSESCORE_VERSION=$(cmake -P config.cmake | sed -n -e 's/^.*MUSESCORE_VERSION_FULL  *//p')

MUSESCORE_VERSION_FULL=$MUSESCORE_VERSION.$BUILD_NUMBER

echo ${MUSESCORE_VERSION_FULL} > ${OUT_DIR}/build_version.env
cat ${OUT_DIR}/build_version.env

