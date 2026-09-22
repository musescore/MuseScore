#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited and others
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

# Creates a keychain in $RUNNER_TEMP holding the code signing certificate, and
# exports its path as MAC_KEYCHAIN, so that the workflow can delete it again.
#
# Required environment:
#   MAC_SIGN_CERTIFICATE_ENCRYPT_SECRET  password the .p12.enc archive was encrypted with
#   MAC_SIGN_CERTIFICATE_PASSWORD        password the .p12 inside it was exported with

set -euo pipefail

: "${MAC_SIGN_CERTIFICATE_ENCRYPT_SECRET:?}"
: "${MAC_SIGN_CERTIFICATE_PASSWORD:?}"

TEMP_DIR="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
KEYCHAIN="${TEMP_DIR}/signing.keychain-db"
CERTIFICATE_P12="${TEMP_DIR}/mac_musescore.p12"

umask 077
trap 'rm -f "$CERTIFICATE_P12"' EXIT

7z x -y ./buildscripts/ci/macos/resources/mac_musescore.p12.enc \
    -o"${TEMP_DIR}" \
    -p"${MAC_SIGN_CERTIFICATE_ENCRYPT_SECRET}" \
    > /dev/null

KEYCHAIN_PASSWORD="$(openssl rand -hex 24)"

security create-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "MAC_KEYCHAIN=$KEYCHAIN" >> "$GITHUB_ENV"
fi

# Without arguments: no auto-lock. Packaging signs the DMG only after waiting
# for the app to be notarized, which can take longer than any timeout we would
# choose, and codesign cannot unlock the keychain by itself.
security set-keychain-settings "$KEYCHAIN"
security unlock-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"

security import "$CERTIFICATE_P12" -k "$KEYCHAIN" -P "$MAC_SIGN_CERTIFICATE_PASSWORD" -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple: -s -k "$KEYCHAIN_PASSWORD" "$KEYCHAIN" > /dev/null

# codesign finds identities through the search list; macdeployqt, which runs
# codesign itself, cannot be told to use a particular keychain.
SEARCH_LIST=("$KEYCHAIN")
while IFS= read -r line; do
    if [ "$line" != "$KEYCHAIN" ]; then
        SEARCH_LIST+=("$line")
    fi
done < <(security list-keychains -d user | sed -e 's/^ *"//' -e 's/"$//')
security list-keychains -d user -s "${SEARCH_LIST[@]}"

security find-identity -v -p codesigning "$KEYCHAIN"
