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

echo "Uploading to apple to notarize..."
RequestUUID=$(xcrun altool --notarize-app --primary-bundle-id "org.musescore.MuseScore" -u $APPLE_USERNAME -p $APPLE_PASSWORD --asc-provider MuseScore --file $ARTIFACTS_DIR/$ARTIFACT_NAME 2>&1 | grep RequestUUID | awk '{print $3'})

if  [ -z "$RequestUUID" ]
then
	echo "Notarization failed; running again to get error message"
	failure=$(xcrun altool --notarize-app --primary-bundle-id "org.musescore.MuseScore" -u $APPLE_USERNAME -p $APPLE_PASSWORD --asc-provider MuseScore --file $ARTIFACTS_DIR/$ARTIFACT_NAME 2>&1)
	echo "MESSAGE: $failure"
	exit 1
else
	echo "Got notarization request UUID: $RequestUUID"
fi

success=0
sleep 30
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
	echo "Checking progress..."
	progress=$(xcrun altool --notarization-info "${RequestUUID}" -u $APPLE_USERNAME -p $APPLE_PASSWORD 2>&1)
	echo "Progress: ${progress}"

	if [ $? -ne 0 ] || [[ "${progress}" =~ "Invalid" ]]; then
		echo "Error with notarization. Exiting"
	fi

	if [[ "${progress}" =~ "success" ]]; then
		success=1
		break
	else
		echo "Not completed yet. Sleeping for 30 seconds."
	fi
	sleep 30
done

if [ $success -eq 1 ] ; then
	echo "Stapling and running packaging up"
	xcrun stapler staple $ARTIFACTS_DIR/$ARTIFACT_NAME
	echo "Staple finished!"
    xcrun stapler validate $ARTIFACTS_DIR/$ARTIFACT_NAME
fi
