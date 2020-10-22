#!/usr/bin/env bash

echo "Build MuseScore"
set -x
trap 'echo Build failed; exit 1' ERR
SKIP_ERR=true

ARTIFACTS_DIR=build.artifacts
TELEMETRY_TRACK_ID=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --telemetry) TELEMETRY_TRACK_ID="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$TELEMETRY_TRACK_ID" ]; then TELEMETRY_TRACK_ID=""; fi

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
MUSESCORE_BUILD_CONFIG=dev
if [ "$BUILD_MODE" == "devel_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "testing_build" ]; then MUSESCORE_BUILD_CONFIG=testing; fi
if [ "$BUILD_MODE" == "stable_build" ]; then MUSESCORE_BUILD_CONFIG=release; fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "TELEMETRY_TRACK_ID: $TELEMETRY_TRACK_ID"

make -f Makefile.osx ci MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG BUILD_NUMBER=$BUILD_NUMBER TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID


bash ./build/ci/tools/make_release_channel_env.sh -c $MUSESCORE_BUILD_CONFIG
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh
