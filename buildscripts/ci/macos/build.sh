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
echo "Build MuseScore"
#set -x
trap 'echo Build failed; exit 1' ERR
SKIP_ERR=true

ARTIFACTS_DIR=build.artifacts
CRASH_REPORT_URL=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --crash_log_url) CRASH_REPORT_URL="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
MUSE_APP_BUILD_MODE=dev
if [ "$BUILD_MODE" == "devel" ]; then MUSE_APP_BUILD_MODE=dev; fi
if [ "$BUILD_MODE" == "nightly" ]; then MUSE_APP_BUILD_MODE=dev; fi
if [ "$BUILD_MODE" == "testing" ]; then MUSE_APP_BUILD_MODE=testing; fi
if [ "$BUILD_MODE" == "stable" ]; then MUSE_APP_BUILD_MODE=release; fi

if [ -z "$VST3_SDK_PATH" ]; then 
    echo "warning: not set VST3_SDK_PATH, build VST module disabled"
    BUILD_VST=OFF
else
    BUILD_VST=ON
fi

echo "MUSE_APP_BUILD_MODE: $MUSE_APP_BUILD_MODE"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "CRASH_REPORT_URL: $CRASH_REPORT_URL"
echo "VST3_SDK_PATH: $VST3_SDK_PATH"

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

MUSESCORE_MACOS_DEPS_PATH="$HOME/musescore_deps_macos" \
CMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
MUSESCORE_INSTALL_DIR="../applebuild" \
MUSE_APP_BUILD_MODE=$MUSE_APP_BUILD_MODE \
MUSESCORE_BUILD_NUMBER=$BUILD_NUMBER \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
MUSESCORE_CRASHREPORT_URL=$CRASH_REPORT_URL \
MUSESCORE_BUILD_VST_MODULE=$BUILD_VST \
MUSESCORE_VST3_SDK_PATH=$VST3_SDK_PATH \
MUSESCORE_BUILD_WEBSOCKET="ON" \
bash ./ninja_build.sh -t install

bash ./buildscripts/ci/tools/make_release_channel_env.sh -c $MUSE_APP_BUILD_MODE
bash ./buildscripts/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./buildscripts/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./buildscripts/ci/tools/make_branch_env.sh
