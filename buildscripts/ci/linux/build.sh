#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

BUILD_TOOLS=$HOME/build_tools
ARTIFACTS_DIR=build.artifacts
CRASH_REPORT_URL=""
BUILD_NUMBER=""
BUILD_MODE=""
SUFFIX="" # appended to `mscore` command name to avoid conflicts (e.g. `mscoredev`)

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --crash_log_url) CRASH_REPORT_URL="$2"; shift ;;
        --build_mode) BUILD_MODE="$2"; shift ;;
        --arch) PACKARCH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi

MUSE_APP_BUILD_MODE=dev

case "${BUILD_MODE}" in
"devel")   MUSE_APP_BUILD_MODE=dev; SUFFIX=dev;;
"nightly") MUSE_APP_BUILD_MODE=dev; SUFFIX=nightly;;
"testing") MUSE_APP_BUILD_MODE=testing; SUFFIX=testing;;
"stable")  MUSE_APP_BUILD_MODE=release; SUFFIX="";;
esac

echo "MUSE_APP_BUILD_MODE: $MUSE_APP_BUILD_MODE"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "CRASH_REPORT_URL: $CRASH_REPORT_URL"
echo "BUILD_MODE: $BUILD_MODE"

echo "=== ENVIRONMENT === "

cat $BUILD_TOOLS/environment.sh
source $BUILD_TOOLS/environment.sh

# disable update module due to current broken functionality
if [ "$PACKARCH" == "aarch64" ] || [ "$PACKARCH" == "armv7l" ]; then
  MUSESCORE_BUILD_UPDATE_MODULE="OFF"
fi

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

# Build portable AppImage
MUSE_APP_BUILD_MODE=$MUSE_APP_BUILD_MODE \
MUSE_APP_INSTALL_SUFFIX=$SUFFIX \
MUSESCORE_BUILD_NUMBER=$BUILD_NUMBER \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
MUSESCORE_CRASHREPORT_URL=$CRASH_REPORT_URL \
MUSESCORE_BUILD_CRASHPAD_CLIENT=${MUSESCORE_BUILD_CRASHPAD_CLIENT:-"ON"} \
MUSESCORE_BUILD_UPDATE_MODULE=${MUSESCORE_BUILD_UPDATE_MODULE:-"ON"} \
MUSESCORE_BUILD_VST_MODULE="ON" \
MUSESCORE_BUILD_WEBSOCKET="ON" \
bash ./ninja_build.sh -t appimage


bash ./buildscripts/ci/tools/make_release_channel_env.sh -c $MUSE_APP_BUILD_MODE
bash ./buildscripts/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./buildscripts/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./buildscripts/ci/tools/make_branch_env.sh

df -h .
