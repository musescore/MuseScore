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

trap 'echo Build failed; exit 1' ERR

BUILD_TOOLS=$HOME/build_tools
ARTIFACTS_DIR=build.artifacts
BUILD_DIR=build.release
INSTALL_DIR=build.install

cat $BUILD_TOOLS/environment.sh
source $BUILD_TOOLS/environment.sh

# =========== Build =======================

bash ./build/ci/tools/make_build_mode_env.sh -m devel_build
bash ./build/ci/tools/make_build_number.sh
BUILD_MODE=$(cat ./$ARTIFACTS_DIR/env/build_mode.env)
BUILD_NUMBER=$(cat ./$ARTIFACTS_DIR/env/build_number.env)
MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

bash ninja_build.sh -t clean

MUSESCORE_BUILD_CONFIGURATION="vtest" \
MUSESCORE_REVISION=$MUSESCORE_REVISION \
bash ninja_build.sh -t appimage

bash ./build/ci/tools/make_release_channel_env.sh -c $BUILD_MODE
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./build/ci/tools/make_branch_env.sh

# =========== Pack ==========================

# Constants
HERE="$(cd "$(dirname "$0")" && pwd)"
ORIGIN_DIR=${PWD}

INSTALL_DIR="$(cat $BUILD_DIR/PREFIX.txt)" # MuseScore was installed here
APP_IMAGE_NAME=MuseScoreTemporary
ARTIFACT_NAME=app

# Make AppImage
bash ./build/ci/linux/tools/make_appimage.sh "${INSTALL_DIR}" "${APP_IMAGE_NAME}.AppImage"
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
cp $HERE/run.in $APP_DIR/run
chmod 775 $APP_DIR/run

# # Pack to 7z
# 7z a "$ARTIFACT_NAME.7z" "$APP_DIR/*"
# chmod a+rw "$ARTIFACT_NAME.7z"

# Clean up
rm -f "${APP_IMAGE_NAME}.AppImage"

cd $ORIGIN_DIR
