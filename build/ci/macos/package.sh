#!/usr/bin/env bash

echo "Package MuseScore"

ARTIFACTS_DIR="build.artifacts"
SIGN_CERTIFICATE_ENCRYPT_SECRET=""
SIGN_CERTIFICATE_PASSWORD=""

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

mkdir -p applebuild/mscore.app/Contents/Resources/Frameworks
wget -c --no-check-certificate -nv -O musescore_dependencies_macos.zip  http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_macos.zip
unzip musescore_dependencies_macos.zip -d applebuild/mscore.app/Contents/Resources/Frameworks

# install Sparkle
mkdir -p applebuild/mscore.app/Contents/Frameworks
cp -Rf ~/Library/Frameworks/Sparkle.framework applebuild/mscore.app/Contents/Frameworks

# Setup keychain for code sign
if [ -n "$SIGN_CERTIFICATE_ENCRYPT_SECRET" ]; then 

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

build/package_mac

DMGFILE="$(ls applebuild/*.dmg)"

echo "DMGFILE: $DMGFILE"

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
if [ "$BUILD_MODE" == "nightly_build" ]; then

  BUILD_DATETIME=$(cat $ARTIFACTS_DIR/env/build_datetime.env)
  BUILD_BRANCH=$(cat $ARTIFACTS_DIR/env/build_branch.env)
  BUILD_REVISION=$(cat $ARTIFACTS_DIR/env/build_revision.env)
  ARTIFACT_NAME=MuseScoreNightly-${BUILD_DATETIME}-${BUILD_BRANCH}-${BUILD_REVISION}.dmg

else

  BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env)
  ARTIFACT_NAME=MuseScore-${BUILD_VERSION}.dmg  

fi

mv $DMGFILE $ARTIFACTS_DIR/$ARTIFACT_NAME

bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME
bash ./build/ci/tools/make_publish_url_env.sh -p macosx -a $ARTIFACT_NAME

PUBLISH_URL="$(cat $ARTIFACTS_DIR/env/publish_url.env)"
bash ./build/ci/tools/sparkle_appcast_gen.sh -p macos -u $PUBLISH_URL
