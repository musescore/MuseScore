#!/usr/bin/env bash

ARTIFACTS_DIR="build.artifacts"

DATETIME=$1
if [ -z "$DATETIME" ]; then 
    DATETIME=$(date -u +%Y%m%d%H%M)
fi

if [ -z "$DATETIME" ]; then echo "error: not set DATETIME"; exit 1; fi

if [ -z "$2" ]; then OUT_DIR=$ARTIFACTS_DIR/env; fi

echo $DATETIME > $ARTIFACTS_DIR/env/build_datetime.env
cat $ARTIFACTS_DIR/env/build_datetime.env


