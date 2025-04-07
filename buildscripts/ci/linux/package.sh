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
        --arch) PACKARCH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

source $BUILD_TOOLS/environment.sh

if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$BUILD_VERSION" ]; then BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

if [ -z "$BUILD_MODE" ]; then echo "error: not set BUILD_MODE"; exit 1; fi
if [ -z "$BUILD_VERSION" ]; then echo "error: not set BUILD_VERSION"; exit 1; fi

PACKTYPE=appimage
if [ "$BUILD_MODE" == "devel" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "nightly" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "testing" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "stable" ]; then PACKTYPE=appimage; fi

MAJOR_VERSION="${BUILD_VERSION%%.*}"

if [ -z "$PACKARCH" ]; then PACKARCH="x86_64"; fi

echo "BUILD_MODE: $BUILD_MODE"
echo "BUILD_VERSION: $BUILD_VERSION"
echo "MAJOR_VERSION: $MAJOR_VERSION"
echo "PACKTYPE: $PACKTYPE"
echo "PACKARCH: $PACKARCH"
echo "INSTALL_DIR: $INSTALL_DIR"

if [ "$BUILD_MODE" == "nightly" ]; then
  BUILD_NUMBER=$(cat $ARTIFACTS_DIR/env/build_number.env)
  BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
  BUILD_REVISION=$(cat $ARTIFACTS_DIR/env/build_revision.env)
  ARTIFACT_NAME=MuseScore-Studio-Nightly-${BUILD_NUMBER}-${BUILD_BRANCH}-${BUILD_REVISION}-${PACKARCH}
else
  ARTIFACT_NAME=MuseScore-Studio-${BUILD_VERSION}-${PACKARCH}
fi

if [ "$PACKTYPE" == "7z" ]; then
    mv $INSTALL_DIR $ARTIFACT_NAME
    7z a $ARTIFACTS_DIR/$ARTIFACT_NAME.7z $ARTIFACT_NAME
    bash ./buildscripts/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME.7z
    chmod a+rw $ARTIFACT_NAME.7z
fi

if [ "$PACKTYPE" == "appimage" ]; then
    # To enable automatic updates for AppImages, set UPDATE_INFORMATION according to
    # https://github.com/AppImage/AppImageSpec/blob/master/draft.md#update-information
    case "${BUILD_MODE}" in
    "stable")  export UPDATE_INFORMATION="gh-releases-zsync|musescore|MuseScore|latest|MuseScore-Studio-*${PACKARCH}.AppImage.zsync";;
    "nightly") export UPDATE_INFORMATION="zsync|https://ftp.osuosl.org/pub/musescore-nightlies/linux/${MAJOR_VERSION}x/nightly/MuseScore-Studio-Nightly-latest-${BUILD_BRANCH}-${PACKARCH}.AppImage.zsync";;
    *) unset UPDATE_INFORMATION;; # disable updates for other build modes
    esac

    bash ./buildscripts/ci/linux/tools/make_appimage.sh "${INSTALL_DIR}" "${ARTIFACT_NAME}.AppImage" "${PACKARCH}"
    mv "${INSTALL_DIR}/../${ARTIFACT_NAME}.AppImage" "${ARTIFACTS_DIR}/"
    bash ./buildscripts/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME.AppImage

    if [ -v UPDATE_INFORMATION ]; then
        # zsync file contains data for automatic delta updates
        mv "${INSTALL_DIR}/../${ARTIFACT_NAME}.AppImage.zsync" "${ARTIFACTS_DIR}/"
    fi
fi

df -h .

echo "Package has finished!"
