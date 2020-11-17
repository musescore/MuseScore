#!/usr/bin/env bash

echo "Build Linux MuseScore AppImage"

#set -x
trap 'echo Build failed; exit 1' ERR

df -h .

TELEMETRY_TRACK_ID=""
ARTIFACTS_DIR=build.artifacts
BUILD_MODE=""
BUILDTYPE=portable # portable build is the default build
OPTIONS=""
BUILD_UI_MU4=ON    # not used, only for easier synchronization and compatibility

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --telemetry) TELEMETRY_TRACK_ID="$2"; shift ;;
        --build_mode) BUILD_MODE="$2"; shift ;;
        --build_mu4) BUILD_UI_MU4="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$TELEMETRY_TRACK_ID" ]; then TELEMETRY_TRACK_ID=""; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi

MUSESCORE_BUILD_CONFIG=dev
if [ "$BUILD_MODE" == "devel_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "testing_build" ]; then MUSESCORE_BUILD_CONFIG=testing; fi
if [ "$BUILD_MODE" == "stable_build" ]; then MUSESCORE_BUILD_CONFIG=release; fi
if [ "$BUILD_MODE" == "mtests" ]; then MUSESCORE_BUILD_CONFIG=dev; BUILDTYPE=installdebug; OPTIONS="USE_SYSTEM_FREETYPE=ON UPDATE_CACHE=FALSE PREFIX=$ARTIFACTS_DIR/software"; fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "TELEMETRY_TRACK_ID: $TELEMETRY_TRACK_ID"
echo "BUILD_MODE: $BUILD_MODE"
echo "BUILDTYPE: $BUILDTYPE"
echo "OPTIONS: $OPTIONS"
echo "BUILD_UI_MU4: $BUILD_UI_MU4"

echo "=== ENVIRONMENT === "

cat ./../musescore_environment.sh
source ./../musescore_environment.sh

echo " "
${CXX} --version 
${CC} --version
echo " "
cmake --version
echo " "

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

make CPUS=2 $OPTIONS \
     MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG \
     MUSESCORE_REVISION=$MUSESCORE_REVISION \
     BUILD_NUMBER=$BUILD_NUMBER \
     TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID \
     $BUILDTYPE


bash ./build/ci/tools/make_release_channel_env.sh -c $MUSESCORE_BUILD_CONFIG
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh

df -h .
