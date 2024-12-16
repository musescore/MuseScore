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

S3_KEY=""
S3_SECRET=""
S3_URL="s3://convertor.musescore.org"
ARTIFACTS_DIR=build.artifacts
ARTIFACT_PATH=""
STAGE="devel"
MU_VERSION=""
MU_VERSION_MAJOR_MINOR=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --artifact) ARTIFACT_PATH="$2"; shift ;;
        --stage) STAGE="$2"; shift ;;
        --mu_version) MU_VERSION="$2"; shift ;;
        --mu_version_major_minor) MU_VERSION_MAJOR_MINOR="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

bash ./buildscripts/ci/tools/s3_install.sh --s3_key ${S3_KEY} --s3_secret ${S3_SECRET}

if [ -z "$ARTIFACT_PATH" ]; then 
    ARTIFACT_NAME=$(cat $ARTIFACTS_DIR/env/artifact_name.env)
    ARTIFACT_PATH=$ARTIFACTS_DIR/$ARTIFACT_NAME
fi

ARTIFACT_NAME=$(basename $ARTIFACT_PATH)

echo "=== Publish to S3 ==="

s3cmd put --acl-public --guess-mime-type "$ARTIFACT_PATH" "$S3_URL/$ARTIFACT_NAME"

echo "=== Edit configure file ==="

CONFIGURE_FILE="configure.json"
CONFIGURE_FILE_PATH="$ARTIFACTS_DIR/${CONFIGURE_FILE}"

s3cmd get "$S3_URL/$CONFIGURE_FILE" "$CONFIGURE_FILE_PATH"

ARTIFACT_NAME_NO_EXT="${ARTIFACT_NAME%.*}"

NEW_DISTR=$ARTIFACT_NAME
NEW_IMAGE_URL="ghcr.io/musescore/converter_4:${MU_VERSION}"
NEW_STAGE=$STAGE

if jq -e "has(\"$MU_VERSION_MAJOR_MINOR\")" "$CONFIGURE_FILE_PATH" > /dev/null; then
    jq --arg version "$MU_VERSION_MAJOR_MINOR" \
       --arg distr "$NEW_DISTR" \
       --arg image_url "$NEW_IMAGE_URL" \
       --arg stage "$NEW_STAGE" \
       '(.[$version] // {}) |= {
           distr: $distr,
           image_url: $image_url,
           stage: $stage
       }' "$CONFIGURE_FILE_PATH" > "$CONFIGURE_FILE_PATH.tmp" && mv "$CONFIGURE_FILE_PATH.tmp" "$CONFIGURE_FILE_PATH"
else
    jq --arg version "$MU_VERSION_MAJOR_MINOR" \
       --arg distr "$NEW_DISTR" \
       --arg image_url "$NEW_IMAGE_URL" \
       --arg stage "$NEW_STAGE" \
       '. + {($version): {
           distr: $distr,
           image_url: $image_url,
           stage: $stage
       }}' "$CONFIGURE_FILE_PATH" > "$CONFIGURE_FILE_PATH.tmp" && mv "$CONFIGURE_FILE_PATH.tmp" "$CONFIGURE_FILE_PATH"
fi

s3cmd put --acl-public --guess-mime-type "$CONFIGURE_FILE_PATH" "$S3_URL/$CONFIGURE_FILE"