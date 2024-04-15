#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
echo "Build Docker MuseScore"

HERE="$(dirname ${BASH_SOURCE[0]})"
ORIGIN_DIR=${PWD}
ARTIFACTS_DIR=build.artifacts
DOCKER_WORK_DIR=$ARTIFACTS_DIR/docker
MU_VERSION="2.3.2.2022051117"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -v|--version) MU_VERSION="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$MU_VERSION" ]; then MU_VERSION=$(cat $ARTIFACTS_DIR/env/build_version.env); fi

if [ -z "$MU_VERSION" ]; then echo "Error: Version not set"; exit 1; fi

# Make MU docker files
echo "Prepare Docker files"
mkdir -p $DOCKER_WORK_DIR

cp $HERE/Dockerfile $DOCKER_WORK_DIR/Dockerfile
cp $HERE/setup.sh $DOCKER_WORK_DIR/setup.sh
cp $HERE/install_mu_template.sh $DOCKER_WORK_DIR/install_mu.sh

sed -i 's|x.x.x.xxxxxx|'${MU_VERSION}'|' $DOCKER_WORK_DIR/install_mu.sh


cd $DOCKER_WORK_DIR
echo "Build Docker"
docker build -t ghcr.io/musescore/converter_2:${MU_VERSION} .
cd $ORIGIN_DIR

echo "done!!"

