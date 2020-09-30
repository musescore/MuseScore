#!/usr/bin/env bash

OUT_DIR=$1
ARTIFACTS_DIR="build.artifacts" # default output dir

if [ -z "$1" ]; then OUT_DIR=${ARTIFACTS_DIR}/env; fi

export MUSESCORE_REVISION=$(cat mscore/revision.h)

echo ${MUSESCORE_REVISION} > ${OUT_DIR}/build_revision.env
cat ${OUT_DIR}/build_revision.env
