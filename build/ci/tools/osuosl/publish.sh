#!/usr/bin/env bash

echo "Publish MuseScore"

ARTIFACTS_DIR=build.artifacts

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a|--artifact) ARTIFACT_NAME="$2"; shift ;;
        -s|--secret) OSUOSL_SSH_ENCRYPT_SECRET="$2"; shift ;;
        --os) OS="$2"; shift ;;
        --arch) PACKARCH="$2"; shift ;;
        -m|--mode) BUILD_MODE="$2"; shift ;;
        -v|--version) BUILD_VERSION="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_NAME" ]; then ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"; fi
if [ -z "$BUILD_MODE" ]; then BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env); fi
if [ -z "$BUILD_VERSION" ]; then BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

MAJOR_VERSION="${BUILD_VERSION%%.*}"

if [ -z "$PACKARCH" ]; then PACKARCH="x86_64"; fi

echo "ARTIFACT_NAME: $ARTIFACT_NAME"
echo "SECRET: $OSUOSL_SSH_ENCRYPT_SECRET"
echo "OS: $OS"
echo "PACKARCH: $PACKARCH"
echo "BUILD_MODE: $BUILD_MODE"
echo "BUILD_VERSION: $BUILD_VERSION"
echo "MAJOR_VERSION: $MAJOR_VERSION"

OS_IS_VALID=0
if [[ "$OS" == "linux" || "$OS" == "windows" || "$OS" == "macos" ]]; then OS_IS_VALID=1; fi
if [ "$OS_IS_VALID" == "0" ]; then echo "error: Not valid OS: $OS, allowed: 'linux', 'windows', 'macos'"; exit 1; fi

BUILD_DIR=""
if [ "$BUILD_MODE" == "nightly" ]; then BUILD_DIR="nightly"; else
if [ "$BUILD_MODE" == "testing" ]; then BUILD_DIR="testing"; else
if [ "$BUILD_MODE" == "stable" ]; then BUILD_DIR="stable"; else
echo "error: Not valid BUILD_MODE: $BUILD_MODE, allowed: 'nightly', 'testing', 'stable'"; exit 1;
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

file_extension="${ARTIFACT_NAME##*.}"
LATEST_NAME="MuseScoreNightly-latest-${PACKARCH}.${file_extension}"

echo "Copy ${ARTIFACTS_DIR}/${ARTIFACT_NAME} to $FTP_PATH"
scp -oStrictHostKeyChecking=no -C -i $SSH_KEY $ARTIFACTS_DIR/$ARTIFACT_NAME musescore-nightlies@ftp-osl.osuosl.org:~/ftp/$FTP_PATH

# For Linux, we also need to send a .zsync file, if exists
if [ "$OS" == "linux" ]; then
    if [ -f "$ARTIFACTS_DIR/${ARTIFACT_NAME}.zsync" ]; then
        echo "Copy ${ARTIFACTS_DIR}/${ARTIFACT_NAME}.zsync to $FTP_PATH"
        scp -oStrictHostKeyChecking=no -C -i $SSH_KEY $ARTIFACTS_DIR/${ARTIFACT_NAME}.zsync musescore-nightlies@ftp-osl.osuosl.org:~/ftp/$FTP_PATH
        if [ "$BUILD_MODE" == "stable" ]; then
            : # Do nothing. zsync file is in the right place on OSUOSL, but don't forget to upload it to GitHub Releases with the AppImage.
        elif [ "$BUILD_MODE" == "nightly" ]; then
            # zsync file must be available at stable URL. We don't need historic versions of this file so overwrite previous 'latest' zsync.
            ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/$FTP_PATH; mv -f ${ARTIFACT_NAME}.zsync ${LATEST_NAME}.zsync"
        fi
    fi
fi

PUBLISH_URL=https://ftp.osuosl.org/pub/musescore-nightlies/$FTP_PATH
echo $PUBLISH_URL > $ARTIFACTS_DIR/env/publish_url.env
cat $ARTIFACTS_DIR/env/publish_url.env

# Create link to latest
if [ "$BUILD_MODE" == "nightly" ]; then
    echo "Create/update link to latest"
    ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/$FTP_PATH; ln -sf $ARTIFACT_NAME $LATEST_NAME"
fi

# Delete old files
if [ "$BUILD_MODE" == "nightly" ]; then
    echo "Delete old MuseScoreNightly files"
    number_to_keep=42 # includes the one we just uploaded and the symlink to it
    if [ "$OS" == "linux" ]; then
      ((++number_to_keep)) # one extra for the zsync file
    fi
    ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/$FTP_PATH; ls MuseScoreNightly* -t | tail -n +${number_to_keep} | xargs rm -f"
fi

# Sending index.html
# !! The page is automatically sending to the FTP only in the master
# scp -oStrictHostKeyChecking=no -C -i $SSH_KEY build/ci/tools/osuosl/index.html musescore-nightlies@ftp-osl.osuosl.org:~/ftp/

# Trigger
ssh -o StrictHostKeyChecking=no -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "~/trigger-musescore-nightlies"
