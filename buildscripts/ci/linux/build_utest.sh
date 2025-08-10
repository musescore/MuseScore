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
echo "Build utests"

#set -x
trap 'echo Build failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ARTIFACTS_DIR=build.artifacts
BUILD_NUMBER=42
ENABLE_CODE_COVERAGE="false"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --enable_code_coverage) ENABLE_CODE_COVERAGE="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

cat $BUILD_TOOLS/environment.sh
source $BUILD_TOOLS/environment.sh

TESTS_ENABLE_CODE_COVERAGE=OFF
TESTS_COMPILE_USE_UNITY=ON

if [[ "$ENABLE_CODE_COVERAGE" == "true" ]]; then
    TESTS_ENABLE_CODE_COVERAGE=ON
    TESTS_COMPILE_USE_UNITY=OFF
else
    TESTS_ENABLE_CODE_COVERAGE=OFF
    TESTS_COMPILE_USE_UNITY=ON
fi

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

MUSESCORE_BUILD_CONFIGURATION=utest \
MUSE_APP_BUILD_MODE=dev \
MUSESCORE_BUILD_NUMBER=$BUILD_NUMBER \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
MUSESCORE_DOWNLOAD_SOUNDFONT=OFF \
MUSESCORE_BUILD_UNIT_TESTS=ON \
MUSESCORE_UNIT_TESTS_ENABLE_CODE_COVERAGE=$TESTS_ENABLE_CODE_COVERAGE \
MUSESCORE_COMPILE_USE_UNITY=$TESTS_COMPILE_USE_UNITY \
bash ./ninja_build.sh -t installdebug

df -h .
