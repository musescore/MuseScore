#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

S3_KEY=""
S3_SECRET=""
S3_URL="s3://convertor.musescore.org"
ARTIFACTS_DIR=build.artifacts
ARTIFACT_PATH=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --artifact) ARTIFACT_PATH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

sudo bash ./build/ci/tools/s3_install.sh --s3_key ${S3_KEY} --s3_secret ${S3_SECRET}

if [ -z "$ARTIFACT_PATH" ]; then 
    ARTIFACT_NAME=$(cat $ARTIFACTS_DIR/env/artifact_name.env)
    ARTIFACT_PATH=$ARTIFACTS_DIR/$ARTIFACT_NAME
fi

ARTIFACT_NAME=$(basename $ARTIFACT_PATH)

echo "=== Publish to S3 ==="

s3cmd put --acl-public --guess-mime-type "$ARTIFACT_PATH" "$S3_URL/$ARTIFACT_NAME"
