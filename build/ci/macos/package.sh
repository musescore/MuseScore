#!/usr/bin/env bash

echo "Package MuseScore"
trap 'echo Package failed; exit 1' ERR

ARTIFACTS_DIR="build.artifacts"
SIGN_CERTIFICATE_ENCRYPT_SECRET="''"
SIGN_CERTIFICATE_PASSWORD="''"
DEVELOPER_NAME="MuseSore"
BUILD_ARCH="Apple"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --signsecret) SIGN_CERTIFICATE_ENCRYPT_SECRET="$2"; shift ;;
        --signpass) SIGN_CERTIFICATE_PASSWORD="$2"; shift ;;
        --developer_name) DEVELOPER_NAME="$2"; shift ;;
        --arch) BUILD=ARCH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$SIGN_CERTIFICATE_ENCRYPT_SECRET" ]; then echo "warning: not set SIGN_CERTIFICATE_ENCRYPT_SECRET"; fi
if [ -z "$SIGN_CERTIFICATE_PASSWORD" ]; then echo "warning: not set SIGN_CERTIFICATE_PASSWORD"; fi

echo "SIGN_CERTIFICATE_ENCRYPT_SECRET: $SIGN_CERTIFICATE_ENCRYPT_SECRET"
echo "SIGN_CERTIFICATE_PASSWORD: $SIGN_CERTIFICATE_PASSWORD"

mkdir -p applebuild/mscore.app/Contents/Resources/Frameworks
wget -c --no-check-certificate -nv -O musescore_dependencies_macos.zip  http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_macos.zip
unzip musescore_dependencies_macos.zip -d applebuild/mscore.app/Contents/Resources/Frameworks

# install Sparkle
mkdir -p applebuild/mscore.app/Contents/Frameworks
cp -Rf ~/Library/Frameworks/Sparkle.framework applebuild/mscore.app/Contents/Frameworks

# Setup keychain for code sign
if [ "$SIGN_CERTIFICATE_ENCRYPT_SECRET" != "''" ]; then 

    7z x -y ./build/ci/macos/resources/mac_musescore.p12.enc -o./build/ci/macos/resources/ -p${SIGN_CERTIFICATE_ENCRYPT_SECRET}

    export CERTIFICATE_P12=./build/ci/macos/resources/mac_musescore.p12
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
  APP_LONGER_NAME="MuseScore $BUILD_VERSION Nightly";
  PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}b-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "testing" ]; then
  APP_LONGER_NAME="MuseScore $BUILD_VERSION Testing";
  PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}b-${BUILD_REVISION}"
fi
if [ "$BUILD_MODE" == "stable" ]; then
  APP_LONGER_NAME="MuseScore $VERSION_MAJOR";
  PACKAGE_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
fi

build/package_mac --longer_name "$APP_LONGER_NAME" --version "$PACKAGE_VERSION" --developer_name "$DEVELOPER_NAME"

DMGFILE="$(ls applebuild/*.dmg)"
echo "DMGFILE: $DMGFILE"

if [ "$BUILD_MODE" == "nightly" ]; then

  BUILD_DATETIME=$(cat $ARTIFACTS_DIR/env/build_datetime.env)
  BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
  ARTIFACT_NAME=MuseScoreNightly-${BUILD_DATETIME}-${BUILD_BRANCH}-${BUILD_REVISION}-${BUILD_ARCH}.dmg

else

  ARTIFACT_NAME=MuseScore-${BUILD_VERSION}-${BUILD_ARCH}.dmg  

fi

mv $DMGFILE $ARTIFACTS_DIR/$ARTIFACT_NAME

bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME
