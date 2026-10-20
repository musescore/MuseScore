#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

# Enable exit on any error
trap 'echo "Sign failed"; exit 1' ERR

S3_KEY=""
S3_SECRET=""
FILE_PATH=""

S3_BUCKET="muse-sign"
S3_UNSIGNED_DIR="prod-unsigned"
S3_SIGNED_DIR="prod-signed"
S3_FAILED_DIR="prod-failed"

# How long to wait for the sign service (attempts x interval) 10min
CHECK_ATTEMPTS=60
CHECK_INTERVAL=10

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --file_path) FILE_PATH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$S3_KEY" ]; then echo "error: not set S3_KEY"; exit 1; fi
if [ -z "$S3_SECRET" ]; then echo "error: not set S3_SECRET"; exit 1; fi
if [ -z "$FILE_PATH" ]; then echo "error: not set FILE_PATH"; exit 1; fi

# FILE_PATH may come from a .bat script, so it can contain backslashes
FILE_NAME="$(basename "${FILE_PATH//\\//}")"

S3_UNSIGNED_URL="s3://$S3_BUCKET/$S3_UNSIGNED_DIR/$FILE_NAME"
S3_SIGNED_URL="s3://$S3_BUCKET/$S3_SIGNED_DIR/$FILE_NAME"
S3_FAILED_URL="s3://$S3_BUCKET/$S3_FAILED_DIR/$FILE_NAME"
S3_FAILED_ERROR_URL="$S3_FAILED_URL.error"

FILE_SIGNED_PATH="${FILE_PATH}_signed"
FILE_ERROR_PATH="${FILE_PATH}.error"

export AWS_ACCESS_KEY_ID=$S3_KEY
export AWS_SECRET_ACCESS_KEY=$S3_SECRET
export AWS_DEFAULT_REGION=us-east-1

# If the sign service could not sign the file, it puts the file into the failed dir
# and writes the reason into <file name>.error next to it
report_error() {
    echo "error: sign failed for $FILE_NAME"
    echo "----- begin $FILE_NAME.error -----"
    cat "$FILE_ERROR_PATH"
    echo "----- end $FILE_NAME.error -----"
    rm -f "$FILE_ERROR_PATH"

    # remove from the bucket, so that it doesn't affect the next build
    remove_failed
}

remove_failed() {
    aws s3 rm "$S3_FAILED_URL" > /dev/null 2>&1 || true
    aws s3 rm "$S3_FAILED_ERROR_URL" > /dev/null 2>&1 || true
}

echo "Sign file: $FILE_PATH"
echo "Sign file name: $FILE_NAME"

# There may be leftovers from a previous run with the same name,
# they would be detected as a failure of this run
echo "Remove leftovers from a previous run, if any"
remove_failed

echo "Send file to sign service..."
aws s3 cp "$FILE_PATH" "$S3_UNSIGNED_URL"

# Disable exit on any error
trap '' ERR

signed=-1
for (( i=1; i<=CHECK_ATTEMPTS; i++ )); do
    echo "Check sign... $i of $CHECK_ATTEMPTS"

    aws s3 cp "$S3_SIGNED_URL" "$FILE_SIGNED_PATH"
    signed=$?
    if [ $signed -eq 0 ]; then break; fi

    if aws s3 cp "$S3_FAILED_ERROR_URL" "$FILE_ERROR_PATH" > /dev/null 2>&1; then
        report_error
        exit 1
    fi

    if [ $i -eq $CHECK_ATTEMPTS ]; then
        echo "error: sign timed out, the signed file $S3_SIGNED_URL did not appear"
        exit 1
    fi

    echo "does not exist is normal, waiting $CHECK_INTERVAL seconds"
    sleep $CHECK_INTERVAL
done

echo "Signed file downloaded successfully"

# Enable exit on any error
trap 'echo "Sign failed"; exit 1' ERR

echo "Delete signed file from service"
aws s3 rm "$S3_SIGNED_URL"

echo "Rename original unsigned file"
mv "$FILE_PATH" "${FILE_PATH}_origin"

echo "Rename signed file to original name"
mv "$FILE_SIGNED_PATH" "$FILE_PATH"

echo "Delete original unsigned file"
rm -f "${FILE_PATH}_origin"

echo "All done"
