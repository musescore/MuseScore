#!/usr/bin/env bash

echo "Package MuseScore"

ARTIFACTS_DIR="build.artifacts"

mkdir -p applebuild/mscore.app/Contents/Resources/Frameworks
wget -c --no-check-certificate -nv -O musescore_dependencies_macos.zip  http://utils.musescore.org.s3.amazonaws.com/musescore_dependencies_macos.zip
unzip musescore_dependencies_macos.zip -d applebuild/mscore.app/Contents/Resources/Frameworks

# install Sparkle
mkdir -p applebuild/mscore.app/Contents/Frameworks
cp -Rf ~/Library/Frameworks/Sparkle.framework applebuild/mscore.app/Contents/Frameworks

build/package_mac

DMGFILE="$(ls applebuild/*.dmg)"

echo "DMGFILE: $DMGFILE"

BUILD_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env)
ARTIFACT_NAME=MuseScore-${BUILD_VERSION}.dmg

mv $DMGFILE $ARTIFACTS_DIR/$ARTIFACT_NAME

bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME
bash ./build/ci/tools/make_publish_url_env.sh -p macosx -a $ARTIFACT_NAME

PUBLISH_URL="$(cat $ARTIFACTS_DIR/env/publish_url.env)"
bash ./build/ci/tools/sparkle_appcast_gen.sh -p macos -u $PUBLISH_URL
