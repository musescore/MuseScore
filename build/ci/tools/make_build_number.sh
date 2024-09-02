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
ARTIFACTS_DIR="build.artifacts"

NUMBER=$1
if [ -z "$NUMBER" ]; then
    NUMBER=$(date -u +%y%j%H%M) # less than 2147483647 to fit in Int32 for WiX packaging tool
fi

if [ -z "$NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi

if [ -z "$2" ]; then OUT_DIR=$ARTIFACTS_DIR/env; fi

mkdir -p $ARTIFACTS_DIR
mkdir -p $ARTIFACTS_DIR/env

echo $NUMBER > $ARTIFACTS_DIR/env/build_number.env
echo "BUILD_NUMBER: $(cat $ARTIFACTS_DIR/env/build_number.env)"
