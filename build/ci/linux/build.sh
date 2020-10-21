#!/usr/bin/env bash

echo "Build Linux MuseScore AppImage"

#set -x
trap 'echo Build failed; exit 1' ERR

TELEMETRY_TRACK_ID=""
ARTIFACTS_DIR=build.artifacts

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

echo "=== ENVIRONMENT === "

ENV_FILE=./../musescore_environment.sh
cat ${ENV_FILE}
. ${ENV_FILE}

echo " "
${CXX} --version 
${CC} --version
echo " "
cmake --version
echo " "

echo "=== BUILD === "

make revision
make -j2 MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG BUILD_NUMBER=$BUILD_NUMBER TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID portable


bash ./build/ci/tools/make_release_channel_env.sh 
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh
