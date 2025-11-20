#!/usr/bin/env bash

S3_URL=https://s3.amazonaws.com/convertor.musescore.org
MU_DIR=/musescore
MU_VERSION=x.x.x.xxxxxx

mkdir -p $MU_DIR

echo "=== Install MuseScore ${MU_VERSION} ==="
echo "=== old"
7zz i | head -5

apt-get remove -y p7zip p7zip-full
apt-get install -y --no-install-recommends 7zip xz-utils

wget https://www.7-zip.org/a/7z2407-linux-arm64.tar.xz -O /tmp/7z.tar.xz
mkdir -p /opt/7zip
tar -xf /tmp/7z.tar.xz -C /opt/7zip
ln -sf /opt/7zip/7zz /usr/local/bin/7zz

echo "=== new"
7zz i | head -5

MU_DISTRO=MuseScore-${MU_VERSION}
wget --show-progress -O $MU_DIR/$MU_DISTRO.7z "$S3_URL/$MU_DISTRO.7z"
7zz x -y $MU_DIR/$MU_DISTRO.7z -o"$MU_DIR/"
$MU_DIR/convertor -v

