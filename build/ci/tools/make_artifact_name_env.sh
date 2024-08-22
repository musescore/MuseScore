#!/usr/bin/env bash

ARTIFACT_NAME=$1
if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi

OUT_DIR=$2
ARTIFACTS_DIR="build.artifacts" # default output dir

if [ -z "$2" ]; then OUT_DIR=$ARTIFACTS_DIR/env; fi

echo $ARTIFACT_NAME > $OUT_DIR/artifact_name.env
cat $OUT_DIR/artifact_name.env
