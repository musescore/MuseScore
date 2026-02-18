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

ARTIFACTS_DIR="build.artifacts"
SIGN_CERTIFICATE_ENCRYPT_SECRET="''"
SIGN_CERTIFICATE_PASSWORD="''"

SIGN_ARGS=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --signsecret) SIGN_CERTIFICATE_ENCRYPT_SECRET="$2"; shift ;;
        --signpass) SIGN_CERTIFICATE_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$SIGN_CERTIFICATE_ENCRYPT_SECRET" ]; then echo "warning: not set SIGN_CERTIFICATE_ENCRYPT_SECRET"; fi
if [ -z "$SIGN_CERTIFICATE_PASSWORD" ]; then echo "warning: not set SIGN_CERTIFICATE_PASSWORD"; fi

echo "SIGN_CERTIFICATE_ENCRYPT_SECRET: $SIGN_CERTIFICATE_ENCRYPT_SECRET"
echo "SIGN_CERTIFICATE_PASSWORD: $SIGN_CERTIFICATE_PASSWORD"

# Setup keychain for code sign
if [ "$SIGN_CERTIFICATE_ENCRYPT_SECRET" != "''" ]; then

    7z x -y ./buildscripts/ci/macos/resources/mac_musescore.p12.enc -o./buildscripts/ci/macos/resources/ -p${SIGN_CERTIFICATE_ENCRYPT_SECRET}

    export CERTIFICATE_P12=./buildscripts/ci/macos/resources/mac_musescore.p12
    export KEYCHAIN=build.keychain
    security create-keychain -p ci $KEYCHAIN
    security default-keychain -s $KEYCHAIN
    security unlock-keychain -p ci $KEYCHAIN
    # Set keychain timeout to 1 hour for long builds
    # see http://www.egeek.me/2013/02/23/jenkins-and-xcode-user-interaction-is-not-allowed/
    security set-keychain-settings -t 3600 -l $KEYCHAIN
    security import $CERTIFICATE_P12 -k $KEYCHAIN -P "$SIGN_CERTIFICATE_PASSWORD" -T /usr/bin/codesign

    security set-key-partition-list -S apple-tool:,apple: -s -k ci $KEYCHAIN

    SIGN_ARGS="--sign"
fi

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

buildscripts/packaging/macOS/package.sh --app-name "$APP_NAME" --vol-name "$VOL_NAME" $SIGN_ARGS

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
