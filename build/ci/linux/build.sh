#!/usr/bin/env bash

echo "Build Linux MuseScore AppImage"

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

echo "BUILD_NUMBER: $BUILD_NUMBER"

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
make -j2 BUILD_NUMBER=$BUILD_NUMBER TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID portable


bash ./build/ci/tools/make_release_channel_env.sh 
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh
