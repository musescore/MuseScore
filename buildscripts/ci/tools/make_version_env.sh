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
BUILD_NUMBER=$1
if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi

OUT_DIR=$2
ARTIFACTS_DIR="build.artifacts" # default output dir

if [ -z "$2" ]; then OUT_DIR=${ARTIFACTS_DIR}/env; fi

export MUSE_APP_VERSION=$(cmake -P version.cmake | sed -n -e 's/^.*MUSE_APP_VERSION  *//p')

MUSESCORE_VERSION_FULL=$MUSE_APP_VERSION.$BUILD_NUMBER

echo ${MUSESCORE_VERSION_FULL} > ${OUT_DIR}/build_version.env
cat ${OUT_DIR}/build_version.env
