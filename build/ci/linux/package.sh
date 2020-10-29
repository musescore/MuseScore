#!/usr/bin/env bash

echo "Package MuseScore"
trap 'echo Package failed; exit 1' ERR

df -h .

source ./../musescore_environment.sh

ARTIFACTS_DIR=build.artifacts
BUILD_MODE=""
BUILD_DIR=build.release
INSTALL_DIR="$(cat $BUILD_DIR/PREFIX.txt)" # MuseScore was installed here

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --build_mode) BUILD_MODE="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$BUILD_MODE" ]; then echo "error: not set BUILD_MODE"; exit 1; fi

PACKTYPE=appimage
if [ "$BUILD_MODE" == "devel_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "testing_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "stable_build" ]; then PACKTYPE=appimage; fi

echo "BUILD_MODE: $BUILD_MODE"
echo "PACKTYPE: $PACKTYPE"
echo "INSTALL_DIR: $INSTALL_DIR"

if [ "$BUILD_MODE" == "nightly_build" ]; then
  BUILD_DATETIME=$(cat $ARTIFACTS_DIR/env/build_datetime.env)
  BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
  BUILD_REVISION=$(cat $ARTIFACTS_DIR/env/build_revision.env)
  ARTIFACT_NAME=MuseScoreNightly-${BUILD_DATETIME}-${BUILD_BRANCH}-${BUILD_REVISION}-x86_64
else
  BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env)
  ARTIFACT_NAME=MuseScore-${BUILD_VERSION}-x86_64  
fi

if [ "$PACKTYPE" == "7z" ]; then
mv $INSTALL_DIR $ARTIFACT_NAME
7z a $ARTIFACTS_DIR/$ARTIFACT_NAME.7z $ARTIFACT_NAME
bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME.7z
chmod a+rw $ARTIFACT_NAME.7z
fi

if [ "$PACKTYPE" == "appimage" ]; then
bash ./build/ci/linux/tools/make_appimage.sh $INSTALL_DIR
APPIMAGE_FILE="$(ls $BUILD_DIR/*.AppImage)"
mv $APPIMAGE_FILE $ARTIFACTS_DIR/$ARTIFACT_NAME.AppImage
bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME.AppImage
fi

df -h .

echo "Package has finished!" 
