#!/usr/bin/env bash


ARTIFACTS_DIR="build.artifacts"
mkdir $ARTIFACTS_DIR
mkdir $ARTIFACTS_DIR/env

ARTIFACT_NAME=MuseScore-3.5.0.275455532.dmg

wget -q --show-progress -O $ARTIFACTS_DIR/$ARTIFACT_NAME "https://s3.amazonaws.com/utils.musescore.org/MuseScore-3.5.0.275455532.dmg"


bash ./build/ci/tools/make_artifact_name_env.sh $ARTIFACT_NAME

