#!/usr/bin/env bash

ARTIFACTS_DIR="build.artifacts"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -p|--platform) PLATFORM="$2"; shift ;;
        -a|--artifact) ARTIFACT_NAME="$2"; shift ;;
        -o|--output) OUT_DIR="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_NAME" ]; then ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"; fi
if [ -z "$OUT_DIR" ]; then OUT_DIR=$ARTIFACTS_DIR/env; fi

# check args
if [ -z "$PLATFORM" ]; then echo "error: not set PLATFORM"; exit 1; fi
if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi

echo "PLATFORM: $PLATFORM"
echo "ARTIFACT_NAME: $ARTIFACT_NAME"

UPLOAD_URL=https://ftp.osuosl.org/pub/musescore-nightlies/$PLATFORM/$ARTIFACT_NAME

echo $UPLOAD_URL > $OUT_DIR/publish_url.env
cat $OUT_DIR/publish_url.env