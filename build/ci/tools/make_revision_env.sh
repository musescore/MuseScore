#!/usr/bin/env bash

REVISION=$1
ARTIFACTS_DIR=build.artifacts

if [ -z "$REVISION" ]; then 
    REVISION=$(git rev-parse --short=7 HEAD)
fi

if [ -z "$REVISION" ]; then echo "error: not set REVISION"; exit 1; fi

echo $REVISION > $ARTIFACTS_DIR/env/build_revision.env
cat $ARTIFACTS_DIR/env/build_revision.env
