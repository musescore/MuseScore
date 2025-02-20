#!/usr/bin/env bash

S3_URL=https://s3.amazonaws.com/convertor.musescore.org
MU_DIR=/musescore
MU_VERSION=x.x.x.xxxxxx

mkdir -p $MU_DIR

echo "=== Install MuseScore ${MU_VERSION} ==="
MU_DISTRO=MuseScore-${MU_VERSION}
wget --show-progress -O $MU_DIR/$MU_DISTRO.7z "$S3_URL/$MU_DISTRO.7z"
7z x -y $MU_DIR/$MU_DISTRO.7z -o"$MU_DIR/"
$MU_DIR/convertor -v

