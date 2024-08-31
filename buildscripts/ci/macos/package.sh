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
fi

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env)
BUILD_REVISION=$(cat $ARTIFACTS_DIR/env/build_revision.env)

VERSION_MAJOR="$(cut -d'.' -f1 <<<"$BUILD_VERSION")"
VERSION_MINOR="$(cut -d'.' -f2 <<<"$BUILD_VERSION")"
VERSION_PATCH="$(cut -d'.' -f3 <<<"$BUILD_VERSION")"

APP_LONGER_NAME="MuseScore $VERSION_MAJOR"
PACKAGE_VERSION="$BUILD_VERSION"
if [ "$BUILD_MODE" == "devel" ]; then
    APP_LONGER_NAME="MuseScore $BUILD_VERSION Devel"
    PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}b-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "nightly" ]; then
    APP_LONGER_NAME="MuseScore $BUILD_VERSION Nightly"
    PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}b-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "testing" ]; then
    APP_LONGER_NAME="MuseScore $BUILD_VERSION Testing"
    PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}b-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "stable" ]; then
    APP_LONGER_NAME="MuseScore $VERSION_MAJOR"
    PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
fi

buildscripts/packaging/macOS/package.sh --longer_name "$APP_LONGER_NAME" --version "$PACKAGE_VERSION"

DMGFILE="$(ls applebuild/*.dmg)"
echo "DMGFILE: $DMGFILE"

if [ "$BUILD_MODE" == "nightly" ]; then
    BUILD_NUMBER=$(cat $ARTIFACTS_DIR/env/build_number.env)
    BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
    ARTIFACT_NAME=MuseScore-Studio-Nightly-${BUILD_NUMBER}-${BUILD_BRANCH}-${BUILD_REVISION}.dmg
else
    ARTIFACT_NAME=MuseScore-Studio-${BUILD_VERSION}.dmg
fi

mv $DMGFILE $ARTIFACTS_DIR/$ARTIFACT_NAME

bash ./buildscripts/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME
