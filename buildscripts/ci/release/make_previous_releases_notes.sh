#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
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

ARTIFACTS_DIR=build.artifacts

S3_KEY=""
S3_SECRET=""
S3_URL=""
S3_BUCKET=""

CURRENT_FILE_NAME=""
PREVIOUS_FILE_NAME=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --s3_url) S3_URL="$2"; shift ;;
        --s3_bucket) S3_BUCKET="$2"; shift ;;
        --current_file_name) CURRENT_FILE_NAME="$2"; shift ;;
        --previous_file_name) PREVIOUS_FILE_NAME="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "=== Get release info ==="

bash ./buildscripts/ci/release/get_file_from_s3.sh \
    --s3_key "${S3_KEY}" \
    --s3_secret "${S3_SECRET}" \
    --s3_url "${S3_URL}" \
    --s3_bucket "${S3_BUCKET}" \
    --local_file_name "${PREVIOUS_FILE_NAME}"

echo "=== Append release info to previous releases ==="

HERE="$(cd "$(dirname "$0")" && pwd)"
python3 "$HERE"/append_release_to_previous_releases.py ${ARTIFACTS_DIR}/"${CURRENT_FILE_NAME}" ${ARTIFACTS_DIR}/"${PREVIOUS_FILE_NAME}"
