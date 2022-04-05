#!/usr/bin/env bash

S3_URL=https://s3.amazonaws.com/convertor.musescore.org
MU_DIR=/musescore

mkdir -p $MU_DIR

# MU2
echo "=== Install MuseScore 2 ==="
MU2_DISTRO=MuseScore-2.3.2.2022021712
wget -q --show-progress -O $MU_DIR/$MU2_DISTRO.7z "$S3_URL/$MU2_DISTRO.7z"
mkdir $MU_DIR/$MU2_DISTRO
7z x -y $MU_DIR/$MU2_DISTRO.7z -o"$MU_DIR/$MU2_DISTRO/"
$MU_DIR/$MU2_DISTRO/convertor -v

# MU3
echo "=== Install MuseScore 3 ==="
MU3_DISTRO=MuseScore-3.6.2.1853151055
wget -q --show-progress -O $MU_DIR/$MU3_DISTRO.7z "$S3_URL/$MU3_DISTRO.7z"
mkdir $MU_DIR/$MU3_DISTRO
7z x -y $MU_DIR/$MU3_DISTRO.7z -o"$MU_DIR/$MU3_DISTRO/"
$MU_DIR/$MU3_DISTRO/convertor -v

# MU4
echo "=== Install MuseScore 4 ==="
MU4_DISTRO=MuseScore-4.0.0.1853159646
wget -q --show-progress -O $MU_DIR/$MU4_DISTRO.7z "$S3_URL/$MU4_DISTRO.7z"
mkdir $MU_DIR/$MU4_DISTRO
7z x -y $MU_DIR/$MU4_DISTRO.7z -o"$MU_DIR/$MU4_DISTRO"
$MU_DIR/$MU4_DISTRO/convertor -v

