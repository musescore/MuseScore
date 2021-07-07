#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
echo "Build Linux MuseScore AppImage"

#set -x
trap 'echo Build failed; exit 1' ERR

df -h .

ARTIFACTS_DIR=build.artifacts
TELEMETRY_TRACK_ID=""
CRASH_REPORT_URL=""
BUILD_MODE=""
SUFFIX="" # appended to `mscore` command name to avoid conflicts (e.g. `mscore-dev`)
YOUTUBE_API_KEY=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --telemetry) TELEMETRY_TRACK_ID="$2"; shift ;;
        --crash_log_url) CRASH_REPORT_URL="$2"; shift ;;
        --build_mode) BUILD_MODE="$2"; shift ;;
        --youtube_api_key) YOUTUBE_API_KEY="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$TELEMETRY_TRACK_ID" ]; then TELEMETRY_TRACK_ID=""; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$YOUTUBE_API_KEY" ]; then YOUTUBE_API_KEY=""; fi

MUSESCORE_BUILD_CONFIG=dev
BUILD_UNIT_TESTS=OFF
case "${BUILD_MODE}" in
"devel_build")   MUSESCORE_BUILD_CONFIG=dev; SUFFIX=-dev;;
"nightly_build") MUSESCORE_BUILD_CONFIG=dev; SUFFIX=-nightly;;
"testing_build") MUSESCORE_BUILD_CONFIG=testing; SUFFIX=-testing;;
"stable_build")  MUSESCORE_BUILD_CONFIG=release; SUFFIX="";;
"mtests")        MUSESCORE_BUILD_CONFIG=dev; BUILDTYPE=installdebug; OPTIONS="USE_SYSTEM_FREETYPE=ON UPDATE_CACHE=FALSE PREFIX=$ARTIFACTS_DIR/software";;
esac

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "TELEMETRY_TRACK_ID: $TELEMETRY_TRACK_ID"
echo "CRASH_REPORT_URL: $CRASH_REPORT_URL"
echo "BUILD_MODE: $BUILD_MODE"
echo "YOUTUBE_API_KEY: $YOUTUBE_API_KEY"

echo "=== ENVIRONMENT === "

cat ./../musescore_environment.sh
source ./../musescore_environment.sh

echo " "
${CXX} --version
${CC} --version
echo " "
cmake --version
echo " "
echo "VST3_SDK_PATH: $VST3_SDK_PATH"
if [ -z "$VST3_SDK_PATH" ]; then 
    echo "warning: not set VST3_SDK_PATH, build VST module disabled"
    BUILD_VST=OFF
else
    BUILD_VST=ON
fi

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

# Build portable AppImage
MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG \
MUSESCORE_INSTALL_SUFFIX=$SUFFIX \
MUSESCORE_BUILD_NUMBER=$BUILD_NUMBER \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
MUSESCORE_TELEMETRY_ID=$TELEMETRY_TRACK_ID \
MUSESCORE_CRASHREPORT_URL=$CRASH_REPORT_URL \
MUSESCORE_BUILD_VST=$BUILD_VST \
MUSESCORE_VST3_SDK_PATH=$VST3_SDK_PATH \
MUSESCORE_YOUTUBE_API_KEY=$YOUTUBE_API_KEY \
bash ./ninja_build.sh -t appimage


bash ./build/ci/tools/make_release_channel_env.sh -c $MUSESCORE_BUILD_CONFIG
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh

df -h .
