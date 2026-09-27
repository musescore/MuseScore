#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

ARTIFACTS_DIR="build.artifacts"
APPLE_TEAM_ID=""
APPLE_USERNAME=""
APPLE_PASSWORD=""

SIGN_ARGS=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --sign) SIGN_ARGS="--sign" ;;
        --team-id) APPLE_TEAM_ID="$2"; shift ;;
        -u|--user) APPLE_USERNAME="$2"; shift ;;
        -p|--password) APPLE_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env)
BUILD_REVISION=$(cat $ARTIFACTS_DIR/env/build_revision.env)
BUILD_NUMBER=$(cat $ARTIFACTS_DIR/env/build_number.env)

VERSION_MAJOR="$(cut -d'.' -f1 <<<"$BUILD_VERSION")"
VERSION_MINOR="$(cut -d'.' -f2 <<<"$BUILD_VERSION")"
VERSION_PATCH="$(cut -d'.' -f3 <<<"$BUILD_VERSION")"

# TODO: rename to MuseScore Studio (https://github.com/musescore/MuseScore/issues/32235)
APP_NAME="MuseScore $VERSION_MAJOR"
if [ "$BUILD_MODE" == "devel" ]; then
    APP_NAME="MuseScore $BUILD_VERSION Development"
    VOL_NAME="MuseScore-Studio-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "nightly" ]; then
    APP_NAME="MuseScore $BUILD_VERSION Nightly"
    VOL_NAME="MuseScore-Studio-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "testing" ]; then
    APP_NAME="MuseScore $BUILD_VERSION Testing"
    VOL_NAME="MuseScore-Studio-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "stable" ]; then
    APP_NAME="MuseScore $VERSION_MAJOR"
    VOL_NAME="MuseScore-Studio-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
fi

buildscripts/packaging/macOS/package.sh --app-name "$APP_NAME" --vol-name "$VOL_NAME" --user "$APPLE_USERNAME" --password "$APPLE_PASSWORD" --team-id "$APPLE_TEAM_ID" $SIGN_ARGS

DMGFILE="$(ls applebuild/*.dmg)"
echo "DMGFILE: $DMGFILE"

if [ "$BUILD_MODE" == "nightly" ]; then
    BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
    ARTIFACT_NAME=MuseScore-Studio-Nightly-${BUILD_NUMBER}-${BUILD_BRANCH}-${BUILD_REVISION}.dmg
else
    ARTIFACT_NAME=MuseScore-Studio-${BUILD_VERSION}.dmg
fi

mv $DMGFILE $ARTIFACTS_DIR/$ARTIFACT_NAME

bash ./buildscripts/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME
