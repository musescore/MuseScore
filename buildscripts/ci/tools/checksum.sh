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
echo "Checksum"

ARTIFACTS_DIR=build.artifacts

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a|--artifact) ARTIFACT_PATH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_PATH" ]; then 
    ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)";
    if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi
    ARTIFACT_PATH=$ARTIFACTS_DIR/$ARTIFACT_NAME
fi

if [ -z "$ARTIFACT_PATH" ]; then echo "error: not set ARTIFACT_PATH"; exit 1; fi

echo "ARTIFACT_PATH: $ARTIFACT_PATH"

openssl sha256 $ARTIFACT_PATH > $ARTIFACTS_DIR/checksum.txt