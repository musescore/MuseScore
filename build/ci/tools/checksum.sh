#!/usr/bin/env bash

echo "Checksum"

ARTIFACTS_DIR=build.artifacts

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a|--artifact) ARTIFACT_PATH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_PATH" ]; then 
    ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)";
    if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi
    ARTIFACT_PATH=$ARTIFACTS_DIR/$ARTIFACT_NAME
fi

if [ -z "$ARTIFACT_PATH" ]; then echo "error: not set ARTIFACT_PATH"; exit 1; fi

echo "ARTIFACT_PATH: $ARTIFACT_PATH"

openssl sha256 $ARTIFACT_PATH > $ARTIFACTS_DIR/checksum.txt