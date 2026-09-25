#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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

# Uploads native debug information files to Sentry: a dSYM on macOS, a PDB on
# Windows, an unstripped ELF on Linux, each together with the shipped binary.
# Sentry reads these formats directly, no intermediate symbol files needed.
#
# Sentry configuration is taken from the environment, which sentry-cli reads on
# its own: SENTRY_URL, SENTRY_AUTH_TOKEN, SENTRY_ORG, SENTRY_PROJECT.

set -euo pipefail

SENTRY_CLI_VERSION=2.33.0

if [ "$#" -eq 0 ]; then
    echo "usage: upload_debug_files.sh <file> [file...]"
    exit 1
fi

for VAR in SENTRY_URL SENTRY_AUTH_TOKEN SENTRY_ORG SENTRY_PROJECT; do
    if [ -z "${!VAR:-}" ]; then
        echo "error: $VAR is not set"
        exit 1
    fi
done

# Install sentry-cli

INSTALL_DIR=$(mktemp -d)

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        SENTRY_CLI="${INSTALL_DIR}/sentry-cli.exe"
        curl -sSfL -o "${SENTRY_CLI}" \
            "https://downloads.sentry-cdn.com/sentry-cli/${SENTRY_CLI_VERSION}/sentry-cli-Windows-x86_64.exe"
        ;;
    *)
        SENTRY_CLI="${INSTALL_DIR}/sentry-cli"
        curl -sSfL https://sentry.io/get-cli/ \
            | INSTALL_DIR="${INSTALL_DIR}" SENTRY_CLI_VERSION="${SENTRY_CLI_VERSION}" bash
        ;;
esac

chmod +x "${SENTRY_CLI}"
"${SENTRY_CLI}" --version

# Check what we are about to upload
#
# `debug-files check` fails only on files it cannot parse at all: one that
# simply carries no debug information is still reported as usable. So look at
# the features instead, and require the set as a whole to provide both halves
# of a readable crash report: `debug` for function names, files and lines,
# `unwind` for walking the stack.

HAS_DEBUG=false
HAS_UNWIND=false

for FILE in "$@"; do
    if [ ! -e "${FILE}" ]; then
        echo "error: ${FILE} not found"
        exit 1
    fi

    echo "----- ${FILE}"
    OUTPUT=$("${SENTRY_CLI}" debug-files check "${FILE}")
    echo "${OUTPUT}"

    # Only the '> symtab, debug, unwind' line, not the heading above it
    FEATURES=$(echo "${OUTPUT}" | sed -n '/Contained debug information:/,/Usable:/{ /^[[:space:]]*>/p; }')
    case "${FEATURES}" in
        *debug*) HAS_DEBUG=true ;;
    esac
    case "${FEATURES}" in
        *unwind*) HAS_UNWIND=true ;;
    esac
done

if [ "${HAS_DEBUG}" != "true" ]; then
    echo "error: none of the files carries debug information"
    echo "       the build has no debug info, or the files are stripped"
    exit 1
fi

if [ "${HAS_UNWIND}" != "true" ]; then
    echo "error: none of the files carries unwind information"
    exit 1
fi

# Upload

"${SENTRY_CLI}" debug-files upload --wait "$@"
