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

ARTIFACTS_DIR=build.artifacts

S3_KEY=""
S3_SECRET=""
S3_URL=""
S3_BUCKET=""

FILE_NAME=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --s3_url) S3_URL="$2"; shift ;;
        --s3_bucket) S3_BUCKET="$2"; shift ;;
        --file_name) FILE_NAME="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

command -v s3cmd >/dev/null 2>&1
if [[ $? -ne 0 ]]; then
    echo "=== Install tools ==="

    apt install python3-setuptools

    echo "Install s3cmd"
    pip3 install s3cmd
fi

cat >~/.s3cfg <<EOL
[default]
access_key = ${S3_KEY}
secret_key = ${S3_SECRET}
host_base = ${S3_BUCKET}
host_bucket = ${S3_BUCKET}
website_endpoint = https://${S3_BUCKET}
EOL

echo "=== Publish to S3 ==="

s3cmd put --acl-public --guess-mime-type "$ARTIFACTS_DIR/$FILE_NAME" "$S3_URL"
