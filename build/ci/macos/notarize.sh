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
echo "Notarize MacOS .dmg"
trap 'echo Notarize failed; exit 1' ERR

ARTIFACTS_DIR="build.artifacts"
APPLE_USERNAME=""
APPLE_PASSWORD=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -u|--user) APPLE_USERNAME="$2"; shift ;;
        -p|--password) APPLE_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$APPLE_USERNAME" ]; then echo "error: not set APPLE_USERNAME"; exit 1; fi
if [ -z "$APPLE_PASSWORD" ]; then echo "error: not set APPLE_PASSWORD"; exit 1; fi

echo "APPLE_USERNAME: $APPLE_USERNAME"
echo "APPLE_PASSWORD: $APPLE_PASSWORD"

ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"
echo "ARTIFACT_NAME: $ARTIFACT_NAME"

echo "Uploading to Apple to notarize..."

success=0
xcrun notarytool submit --apple-id $APPLE_USERNAME --team-id MuseScore --password $APPLE_PASSWORD --wait $ARTIFACTS_DIR/$ARTIFACT_NAME && success=1

if [ $success -eq 1 ] ; then
	echo "Stapling and running packaging up"
	xcrun stapler staple $ARTIFACTS_DIR/$ARTIFACT_NAME
	echo "Staple finished!"
    xcrun stapler validate $ARTIFACTS_DIR/$ARTIFACT_NAME
fi
