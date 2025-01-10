#!/usr/bin/env bash

echo "Notarize MacOS .dmg"
trap 'echo Notarize failed; exit 1' ERR

ARTIFACTS_DIR="build.artifacts"
APPLE_USERNAME=""
APPLE_PASSWORD=""
APPLE_TEAM_ID=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -u|--user) APPLE_USERNAME="$2"; shift ;;
        -p|--password) APPLE_PASSWORD="$2"; shift ;;
        -t|--team) APPLE_TEAM_ID="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$APPLE_USERNAME" ]; then echo "error: not set APPLE_USERNAME"; exit 1; fi
if [ -z "$APPLE_PASSWORD" ]; then echo "error: not set APPLE_PASSWORD"; exit 1; fi
if [ -z "$APPLE_TEAM_ID" ]; then echo "error: not set APPLE_TEAM_ID"; exit 1; fi

echo "APPLE_USERNAME: $APPLE_USERNAME"
echo "APPLE_PASSWORD: $APPLE_PASSWORD"
echo "APPLE_TEAM_ID: $APPLE_TEAM_ID"

ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"
echo "ARTIFACT_NAME: $ARTIFACT_NAME"

echo "Uploading to apple to notarize..."

for i in 1 2 3; do
    c=0
    xcrun notarytool submit $ARTIFACTS_DIR/$ARTIFACT_NAME \
        --apple-id $APPLE_USERNAME \
        --password $APPLE_PASSWORD \
        --team-id $APPLE_TEAM_ID \
        --wait \
        || c=$?
    if [ $c -eq 0 ]; then break; fi
    if [ $i -eq 3 ]; then
        echo "notarytool failed; exiting after 3 retries."
        exit 1
    fi
    echo "notarytool failed; retrying in 30s"
    sleep 30
done

echo "Stapling and running packaging up"
xcrun stapler staple $ARTIFACTS_DIR/$ARTIFACT_NAME
echo "Staple finished!"
xcrun stapler validate $ARTIFACTS_DIR/$ARTIFACT_NAME
