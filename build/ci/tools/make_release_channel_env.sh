#!/usr/bin/env bash

ARTIFACTS_DIR=build.artifacts
MUSESCORE_BUILD_CONFIG=dev

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--build_config) MUSESCORE_BUILD_CONFIG="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$MUSESCORE_BUILD_CONFIG" ]; then echo "error: not set MUSESCORE_BUILD_CONFIG"; exit 1; fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG" 

export MSCORE_RELEASE_CHANNEL=$(cmake -DMUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG -P config.cmake | sed -n -e 's/^.*MSCORE_RELEASE_CHANNEL  *//p')

echo ${MSCORE_RELEASE_CHANNEL} > $ARTIFACTS_DIR/env/release_channel.env
cat $ARTIFACTS_DIR/env/release_channel.env