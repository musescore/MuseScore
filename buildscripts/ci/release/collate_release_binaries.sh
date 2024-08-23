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

o="$(basename "$0")" # script name

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$o: Error: Please upgrade Bash."; exit 1; }

set -euo pipefail # exit on error, error on unset variable, preserve errors in pipelines

ARTIFACTS_DIR=build.artifacts
RELEASE_DIR=release
CHECKSUMS_FILE=checksums.sha256.txt

mkdir "${RELEASE_DIR}" # error if it already exists (need it to be empty)

# Move artifacts into release directory.
for path in "${ARTIFACTS_DIR}/"*"/env/artifact_name.env"; do
    artifact_dir="$(dirname "$(dirname "${path}")")"
    artifact_name="$(<"${path}")"
    artifact_path="${artifact_dir}/${artifact_name}"

    mv -v "${artifact_path}" "${RELEASE_DIR}/"
    if [[ -f "${artifact_path}.zsync" ]]; then
        mv -v "${artifact_path}.zsync" "${RELEASE_DIR}/"
    fi
done

declare -A old_checksums new_checksums # associative arrays (i.e. maps / dictionaries)

# Read old checksums calculated during the build.
echo >&2
echo >&2 "Old checksums (openssl sha256):"
for path in "${ARTIFACTS_DIR}/"*"/checksum.txt"; do
    while read -r line; do
        line="${line%$'\r'}" # openssl output has CRLF line endings on Windows
        printf '%s\n' "${line}" >&2
        # Some versions of openssl output 'SHA2-256' instead of 'SHA256'.
        if [[ ! "${line}" =~ ^SHA(2-)?256\((.*)\)=\ ([0-9a-f]{64})$ ]]; then
            echo >&2 "$o: Error: openssl line is badly formed: ${line}"
            exit 1
        fi
        filename="$(basename "${BASH_REMATCH[2]}")"
        checksum="${BASH_REMATCH[3]}"
        old_checksums["${filename}"]="${checksum}"
    done <"${path}"
done

# Calculate new checksums to be uploaded with the release.
echo >&2
cd "${RELEASE_DIR}/"
echo >&2 "New checksums (sha256sum):"
sha256sum * | tee "${CHECKSUMS_FILE}" >&2
echo >&2

# Read new checksums.
while read -r line; do
    # sha256sum prepends filenames with space ( ) if opened in text mode
    # or asterisk (*) if opened in binary mode.
    if [[ ! "${line}" =~ ^([0-9a-f]{64})\ [\ *]([^/]+)$ ]]; then
        echo >&2 "$o: Error: sha256sum line is badly formed: ${line}"
        exit 1
    fi
    checksum="${BASH_REMATCH[1]}"
    filename="${BASH_REMATCH[2]}"
    new_checksums["${filename}"]="${checksum}"
done <"${CHECKSUMS_FILE}"

# Compare old and new checksums.
# Loop over old ones because there's no old checksum for .zsync files.
status=0
for filename in "${!old_checksums[@]}"; do
    old_checksum="${old_checksums["${filename}"]}"
    new_checksum="${new_checksums["${filename}"]}"
    if [[ "${old_checksum}" != "${new_checksum}" ]]; then
        echo >&2 "$o: Error: Checksum mismatch: ${filename}"
        status=1
    fi
done

if ((status == 0)); then
    echo >&2 "All checksums match!"
fi

exit ${status}
