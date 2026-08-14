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
trap 'echo Sign failed; exit 1' ERR

S3_KEY=""
S3_SECRET=""
FILE_PATH=""
DIR_PATH=""

S3_BUCKET="muse-sign"
S3_UNSIGNED_DIR="unsigned"
S3_SIGNED_DIR="signed"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        --file_path) FILE_PATH="$2"; shift ;;
        --dir_path) DIR_PATH="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$S3_KEY" ]; then echo "error: not set S3_KEY"; exit 1; fi
if [ -z "$S3_SECRET" ]; then echo "error: not set S3_SECRET"; exit 1; fi
if [ -z "$FILE_PATH" ] && [ -z "$DIR_PATH" ]; then echo "error: not set FILE_PATH or DIR_PATH"; exit 1; fi

FILES=()
if [ -n "$DIR_PATH" ]; then
    while IFS= read -r -d '' f; do
        FILES+=("$f")
    done < <(find "$DIR_PATH" -type f \( -name '*.exe' -o -name '*.dll' \) -print0)
    if [ ${#FILES[@]} -eq 0 ]; then echo "error: no exe/dll files in $DIR_PATH"; exit 1; fi
else
    FILES=("$FILE_PATH")
fi

DUPLICATES=$(for f in "${FILES[@]}"; do basename "$f"; done | sort | uniq -d)
if [ -n "$DUPLICATES" ]; then
    echo "error: duplicate file names, the sign service keys by name:"
    echo "$DUPLICATES"
    exit 1
fi

export AWS_ACCESS_KEY_ID=$S3_KEY
export AWS_SECRET_ACCESS_KEY=$S3_SECRET
export AWS_DEFAULT_REGION=us-east-1

aws s3 ls s3://$S3_BUCKET

echo "Send ${#FILES[@]} file(s) to sign service..."
for f in "${FILES[@]}"; do
    aws s3 cp "$f" "s3://$S3_BUCKET/$S3_UNSIGNED_DIR/$(basename "$f")"
done
aws s3 ls s3://$S3_BUCKET/$S3_UNSIGNED_DIR/

# Disable exit on any error
trap '' ERR

MAX_TRIES=$((9 + ${#FILES[@]}))
PENDING=("${FILES[@]}")
for ((i = 1; i <= MAX_TRIES; i++)); do
    echo "Check sign... $i (${#PENDING[@]} pending)"
    REMAINING=()
    for f in "${PENDING[@]}"; do
        S3_SIGNED_URL="s3://$S3_BUCKET/$S3_SIGNED_DIR/$(basename "$f")"
        if aws s3 cp "$S3_SIGNED_URL" "${f}_signed"; then
            aws s3 rm "$S3_SIGNED_URL"
            mv "$f" "${f}_origin"
            mv "${f}_signed" "$f"
            rm -f "${f}_origin"
        else
            REMAINING+=("$f")
        fi
    done
    PENDING=("${REMAINING[@]}")
    if [ ${#PENDING[@]} -eq 0 ]; then break; fi
    if [ $i -eq $MAX_TRIES ]; then
        echo "Sign failed, still pending:"
        printf '%s\n' "${PENDING[@]}"
        exit 1
    fi
    echo "does not exist is normal, waiting 60 seconds"
    sleep 60
done

echo "All done"