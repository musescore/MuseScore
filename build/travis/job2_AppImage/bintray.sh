#!/bin/bash

# Upload files to Bintray (https://bintray.com/docs/api/)
# Inspired by: https://github.com/probonopd/AppImages/blob/master/bintray.sh

# The MIT License (MIT)
#
# Copyright (c) 2019 Peter Jonas
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set -e # exit on error
set +x # protect secrets

files=("$@") # args

if ((${#files[@]} <= 0)); then
  echo "$0: No files specified to upload" >&2
  exit 1
fi

for file in "${files[@]}"; do
  if [[ ! -r "${file}" ]]; then
    echo "$0: File is invalid or lacks read permission '${file}'" >&2
    exit 1
  fi
done

env_error_msg="Required environment variable is empty or unset."

# You need a user account on Bintray and an API key before you can upload.
BINTRAY_USER="${BINTRAY_USER:?${env_error_msg}}" # env
BINTRAY_API_KEY="${BINTRAY_API_KEY:?${env_error_msg}}" # env

# You must also create a repository (type = "generic") or be given write
# access to one owned by another Bintray user or organisation you belong to.
BINTRAY_REPO="${BINTRAY_REPO:-MuseScore}" # env, or use "MuseScore" if empty or unset
BINTRAY_REPO_OWNER="${BINTRAY_REPO_OWNER:-$BINTRAY_USER}" # env, or use $BINTRAY_USER

# Files within a repository are grouped by package and by version. These will
# be created for you automatically based on the names you provide here.
BINTRAY_PACKAGE="${BINTRAY_PACKAGE:?${env_error_msg}}" # env
BINTRAY_VERSION="${BINTRAY_VERSION:?${env_error_msg}}" # env

api="https://api.bintray.com"
repo_slug="${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}"
package_slug="${repo_slug}/${BINTRAY_PACKAGE}"
version_slug="${package_slug}/${BINTRAY_VERSION}"

function my_curl()
{
  local curl_basic_args=(
    -u "${BINTRAY_USER}:${BINTRAY_API_KEY}"
    -H "Content-Type:application/json"
    -H "Accept:application/json"
    )
  curl "${curl_basic_args[@]}" "$@" | jq . # `jq` pretty-prints JSON
}

# File uploads will fail unless the package already exists on Bintray.
# Create one now. (We assume the repository already exists.)
echo "$0: Creating package '${BINTRAY_PACKAGE}'..." >&2

my_curl -X "POST" -d "@-" "${api}/packages/${repo_slug}" <<EOF
{
  "name": "${BINTRAY_PACKAGE}",
  "desc": "Unofficial MuseScore builds. Use at your own risk!",
  "labels": ["music", "audio", "midi", "musicxml"],
  "licenses": ["GPL-2.0"],
  "vcs_url": "https://github.com/musescore/MuseScore",
  "website_url": "https://musescore.org/",
  "issue_tracker_url": "https://musescore.org/en/project/issues/musescore",
  "github_repo": "musescore/MuseScore",
  "github_release_notes_file": "README.md",
  "public_download_numbers": false,
  "maturity": "Experimental"
}
EOF

# Bintray creates versions automatically when you upload a file, but we'll do
# it ourselves so that we can give the version a meaningful description.
echo "$0: Creating version '${BINTRAY_VERSION}'..." >&2

BINTRAY_VERSION_DESCRIPTION="${BINTRAY_VERSION_DESCRIPTION:-$(git show -s --format='%h - %B')}" # env, or use commit message
BINTRAY_VERSION_VCS_TAG="${BINTRAY_VERSION_VCS_TAG:-$(git rev-parse --short HEAD)}" # env, or use commit SHA

# note: description is piped through `jq` to escape any JSON-unsafe characters
my_curl -X "POST" -d "@-" "${api}/packages/${package_slug}/versions" <<EOF
{
  "name": "${BINTRAY_VERSION}",
  "released": "$(date --utc --iso-8601=ns)",
  "desc": $(jq . -asR <<<"${BINTRAY_VERSION_DESCRIPTION}"),
  "vcs_tag": "${BINTRAY_VERSION_VCS_TAG}"
}
EOF

# Upload smallest file first to save time if it fails.
# Sort array by filesize, largest first (`ls` has no smallest first option).
readarray -t files < <(ls -S "${files[@]}")

# Upload in reverse order
counter=0
for (( i=${#files[@]}-1 ; i>=0 ; i-- )) ; do
  ((++counter))
  file="${files[i]}"
  echo "$0: Uploading file ${counter} of ${#files[@]}: '${file}'..." >&2
  filename="$(basename "${file}")"
  result="$(my_curl -T "${file}" "${api}/content/${version_slug}/${filename}")"
  jq . <<<"${result}" # print the result
  upload_successful="$(jq <<<"${result}" '.message == "success"')"
  if [[ "${upload_successful}" != "true" ]]; then
    echo "$0: Upload failed. No files will be published." >&2
    exit 1 # Cancel upload of other files and don't publish anything
  fi
done

# Upload successful, so now we make files visible to other users
echo "$0: Publishing uploaded files..." >&2
my_curl -X POST "${api}/content/${version_slug}/publish"
