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
echo "Build Linux MuseScore"

#set -x
trap 'echo Build failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ARTIFACTS_DIR=build.artifacts
BUILD_MODE=""
BUILD_VIDEOEXPORT="OFF"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --build_mode) BUILD_MODE="$2"; shift ;;
        --build_videoexport) BUILD_VIDEOEXPORT="ON";;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi

MUSE_APP_BUILD_MODE=dev

case "${BUILD_MODE}" in
"devel")   MUSE_APP_BUILD_MODE=dev;;
"testing") MUSE_APP_BUILD_MODE=testing;;
"stable")  MUSE_APP_BUILD_MODE=release;;
esac

echo "MUSE_APP_BUILD_MODE: $MUSE_APP_BUILD_MODE"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "BUILD_MODE: $BUILD_MODE"
echo "BUILD_VIDEOEXPORT: $BUILD_VIDEOEXPORT"

echo "=== ENVIRONMENT === "

cat $BUILD_TOOLS/environment.sh
source $BUILD_TOOLS/environment.sh

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

# Build 
MUSE_APP_BUILD_MODE=$MUSE_APP_BUILD_MODE \
MUSESCORE_BUILD_NUMBER=$BUILD_NUMBER \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
MUSESCORE_BUILD_VIDEOEXPORT_MODULE=$BUILD_VIDEOEXPORT \
bash ./ninja_build.sh -t appimage


bash ./buildscripts/ci/tools/make_release_channel_env.sh -c $MUSE_APP_BUILD_MODE
bash ./buildscripts/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./buildscripts/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./buildscripts/ci/tools/make_branch_env.sh

df -h .
