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
        -v|--version) BUILD_VERSION="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$BUILD_VERSION" ]; then BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

if [ -z "$BUILD_MODE" ]; then echo "error: not set BUILD_MODE"; exit 1; fi
if [ -z "$BUILD_VERSION" ]; then echo "error: not set BUILD_VERSION"; exit 1; fi

PACKTYPE=appimage
if [ "$BUILD_MODE" == "devel_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "testing_build" ]; then PACKTYPE=appimage; fi
if [ "$BUILD_MODE" == "stable_build" ]; then PACKTYPE=appimage; fi

MAJOR_VERSION="${BUILD_VERSION%%.*}"

echo "BUILD_MODE: $BUILD_MODE"
echo "BUILD_VERSION: $BUILD_VERSION"
echo "MAJOR_VERSION: $MAJOR_VERSION"
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
    # To enable automatic updates for AppImages, set UPDATE_INFORMATION according to
    # https://github.com/AppImage/AppImageSpec/blob/master/draft.md#update-information
    case "${BUILD_MODE}" in
    "stable_build")  export UPDATE_INFORMATION="gh-releases-zsync|musescore|MuseScore|latest|MuseScore-*x86_64.AppImage.zsync";;
    "nightly_build") export UPDATE_INFORMATION="zsync|https://ftp.osuosl.org/pub/musescore-nightlies/linux/${MAJOR_VERSION}x/nightly/MuseScoreNightly-latest-x86_64.AppImage.zsync";;
    *) unset UPDATE_INFORMATION;; # disable updates for other build modes
    esac

    bash ./build/ci/linux/tools/make_appimage.sh "${INSTALL_DIR}" "${ARTIFACT_NAME}.AppImage"
    mv "${BUILD_DIR}/${ARTIFACT_NAME}.AppImage" "${ARTIFACTS_DIR}/"
    bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME.AppImage

    if [ -v UPDATE_INFORMATION ]; then
        # zsync file contains data for automatic delta updates
        mv "${BUILD_DIR}/${ARTIFACT_NAME}.AppImage.zsync" "${ARTIFACTS_DIR}/"
    fi
fi

df -h .

echo "Package has finished!"
