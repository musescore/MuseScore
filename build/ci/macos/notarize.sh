#!/usr/bin/env bash

echo "Notarize MacOS .dmg"
trap 'echo Notarize failed; exit 1' ERR

ARTIFACTS_DIR="build.artifacts"
APPLE_USERNAME=""
APPLE_PASSWORD=""

# This information is public and can be extracted by anyone from the final .app file
APPLE_TEAM_ID="6EPAF2X3PR"

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

xcrun notarytool submit --apple-id $APPLE_USERNAME --team-id $APPLE_TEAM_ID --password $APPLE_PASSWORD --wait $ARTIFACTS_DIR/$ARTIFACT_NAME

echo "Stapling and running packaging up"
xcrun stapler staple $ARTIFACTS_DIR/$ARTIFACT_NAME
echo "Staple finished!"
xcrun stapler validate $ARTIFACTS_DIR/$ARTIFACT_NAME
