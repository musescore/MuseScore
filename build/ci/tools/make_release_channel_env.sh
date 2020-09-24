#!/usr/bin/env bash

OUT_DIR=$1
ARTIFACTS_DIR="build.artifacts" # default output dir

if [ -z "$1" ]; then OUT_DIR=${ARTIFACTS_DIR}/env; fi

export MSCORE_RELEASE_CHANNEL=$(cmake -P config.cmake | sed -n -e 's/^.*MSCORE_RELEASE_CHANNEL  *//p')

echo ${MSCORE_RELEASE_CHANNEL} > ${OUT_DIR}/release_channel.env
cat ${OUT_DIR}/release_channel.env