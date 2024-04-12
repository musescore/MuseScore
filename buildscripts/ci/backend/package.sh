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
echo "Package MuseScore"
trap 'echo Package failed; exit 1' ERR

df -h .

BUILD_TOOLS=$HOME/build_tools
ARTIFACTS_DIR=build.artifacts
BUILD_MODE=""
BUILD_DIR=build.release
INSTALL_DIR="$(cat $BUILD_DIR/PREFIX.txt)" # MuseScore was installed here

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --build_mode) BUILD_MODE="$2"; shift ;;
        -v|--version) BUILD_VERSION="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

source $BUILD_TOOLS/environment.sh

if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$BUILD_VERSION" ]; then BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

if [ -z "$BUILD_MODE" ]; then echo "error: not set BUILD_MODE"; exit 1; fi
if [ -z "$BUILD_VERSION" ]; then echo "error: not set BUILD_VERSION"; exit 1; fi


MAJOR_VERSION="${BUILD_VERSION%%.*}"

echo "BUILD_MODE: $BUILD_MODE"
echo "BUILD_VERSION: $BUILD_VERSION"
echo "MAJOR_VERSION: $MAJOR_VERSION"
echo "INSTALL_DIR: $INSTALL_DIR"

# Constants
HERE="$(cd "$(dirname "$0")" && pwd)"
ORIGIN_DIR=${PWD}
ROOT_DIR=${HERE}/../../..

APP_IMAGE_NAME=MuseScoreTemporary
ARTIFACT_NAME=MuseScore-${BUILD_VERSION}

# Make AppImage
bash ./buildscripts/ci/linux/tools/make_appimage.sh "${INSTALL_DIR}" "${APP_IMAGE_NAME}.AppImage"
mv "${INSTALL_DIR}/../${APP_IMAGE_NAME}.AppImage" "${ARTIFACTS_DIR}/"

cd $ARTIFACTS_DIR

# Unpack AppImage
APP_DIR="./$ARTIFACT_NAME"
rm -rf "$APP_DIR"
rm -rf squashfs-root
chmod +x "${APP_IMAGE_NAME}.AppImage"
"./${APP_IMAGE_NAME}.AppImage" --appimage-extract

mv squashfs-root "$APP_DIR"

# Add run file
cp $HERE/convertor.in $APP_DIR/convertor
chmod 775 $APP_DIR/convertor

# Pack to 7z
7z a "$ARTIFACT_NAME.7z" "$APP_DIR/*"
chmod a+rw "$ARTIFACT_NAME.7z"

# Clean up
rm -f "${APP_IMAGE_NAME}.AppImage"
rm -rf $APP_DIR

cd $ORIGIN_DIR

bash ./buildscripts/ci/tools/make_artifact_name_env.sh "$ARTIFACT_NAME.7z"

df -h .

echo "Package has finished!"
