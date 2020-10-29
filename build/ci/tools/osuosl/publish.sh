#!/usr/bin/env bash

echo "Publish MuseScore"

ARTIFACTS_DIR=build.artifacts

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a|--artifact) ARTIFACT_NAME="$2"; shift ;;
        -s|--secret) OSUOSL_SSH_ENCRYPT_SECRET="$2"; shift ;;
        --os) OS="$2"; shift ;;
        -m|--mode) BUILD_MODE="$2"; shift ;;
        -v|--version) MAJOR_VERSION="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_NAME" ]; then ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi

echo "ARTIFACT_NAME: $ARTIFACT_NAME"
echo "SECRET: $OSUOSL_SSH_ENCRYPT_SECRET"
echo "OS: $OS"
echo "BUILD_MODE: $BUILD_MODE"
echo "MAJOR_VERSION: $MAJOR_VERSION"

OS_IS_VALID=0
if [[ "$OS" == "linux" || "$OS" == "windows" || "$OS" == "macos" ]]; then OS_IS_VALID=1; fi
if [ "$OS_IS_VALID" == "0" ]; then echo "error: Not valid OS: $OS, allowed: 'linux', 'windows', 'macos'"; exit 1; fi

BUILD_DIR=""
if [ "$BUILD_MODE" == "nightly_build" ]; then BUILD_DIR="nightly"; else
if [ "$BUILD_MODE" == "testing_build" ]; then BUILD_DIR="testing"; else
if [ "$BUILD_MODE" == "stable_build" ]; then BUILD_DIR="stable"; else
echo "error: Not valid BUILD_MODE: $BUILD_MODE, allowed: 'nightly_build', 'testing_build', 'stable_build'"; exit 1;
fi fi fi

if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi
if [ -z "$OSUOSL_SSH_ENCRYPT_SECRET" ]; then echo "error: not set OSUOSL_SSH_ENCRYPT_SECRET"; exit 1; fi

7z x -y build/ci/tools/osuosl/osuosl_nighlies_rsa.enc -obuild/ci/tools/osuosl/ -p$OSUOSL_SSH_ENCRYPT_SECRET

SSH_KEY=build/ci/tools/osuosl/osuosl_nighlies_rsa

#if [ "$OS" == "windows" ]; then 
#Icacls $SSH_KEY
#Icacls $SSH_KEY /Inheritance:r
#Icacls $SSH_KEY /Grant:r "$(whoami)":'(R,W)'
#Icacls $SSH_KEY
#else
chmod 600 $SSH_KEY
#fi

FTP_PATH=${OS}/${MAJOR_VERSION}x/${BUILD_DIR}

echo "Copy ${ARTIFACTS_DIR}/${ARTIFACT_NAME} to $FTP_PATH"
scp -oStrictHostKeyChecking=no -C -i $SSH_KEY $ARTIFACTS_DIR/$ARTIFACT_NAME musescore-nightlies@ftp-osl.osuosl.org:~/ftp/$FTP_PATH

PUBLISH_URL=https://ftp.osuosl.org/pub/musescore-nightlies/$FTP_PATH
echo $PUBLISH_URL > $ARTIFACTS_DIR/env/publish_url.env
cat $ARTIFACTS_DIR/env/publish_url.env

if [ "$BUILD_MODE" == "nightly_build" ]; then 
    # Delete old files
    echo "Delete old MuseScoreNightly files"
    ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/$FTP_PATH; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"
fi

# Sending index.html
scp -oStrictHostKeyChecking=no -C -i $SSH_KEY build/ci/tools/osuosl/index.html musescore-nightlies@ftp-osl.osuosl.org:~/ftp/

# Trigger 
ssh -o StrictHostKeyChecking=no -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "~/trigger-musescore-nightlies"



