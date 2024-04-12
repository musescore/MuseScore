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
ARTIFACTS_DIR=build.artifacts
SYMBOLS_PATH=""
SENTRY_URL=""
SENTRY_AUTH_TOKEN=""
SENTRY_ORG=""
SENTRY_PROJECT=""

SENTRY_DOWNLOAD_SCRIPT=https://sentry.io/get-cli # Don't work on Windows
SENTRY_DOWNLOAD_Windows_x86_64="https://downloads.sentry-cdn.com/sentry-cli/1.59.0/sentry-cli-Windows-x86_64.exe"  

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -s|--symbols) SYMBOLS_PATH="$2"; shift ;;
        -u|--url) SENTRY_URL="$2"; shift ;;
        -t|--token) SENTRY_AUTH_TOKEN="$2"; shift ;;
        -o|--org) SENTRY_ORG="$2"; shift ;;
        -p|--project) SENTRY_PROJECT="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

# Default
if [ -z "$SYMBOLS_PATH" ]; then SYMBOLS_PATH=$ARTIFACTS_DIR/symbols; fi
if [ -z "$SENTRY_URL" ]; then SENTRY_URL=https://sentry.musescore.org; fi
if [ -z "$SENTRY_ORG" ]; then SENTRY_ORG=musescore; fi

# Check
if [ -z "$SYMBOLS_PATH" ]; then echo "error: not set SYMBOLS_PATH"; exit 1; fi
if [ -z "$SENTRY_URL" ]; then echo "error: not set SENTRY_URL"; exit 1; fi
if [ -z "$SENTRY_AUTH_TOKEN" ]; then echo "error: not set SENTRY_AUTH_TOKEN"; exit 1; fi
if [ -z "$SENTRY_ORG" ]; then echo "error: not set SENTRY_ORG"; exit 1; fi
if [ -z "$SENTRY_PROJECT" ]; then echo "error: not set SENTRY_PROJECT"; exit 1; fi

# Print
echo "SYMBOLS_PATH: $SYMBOLS_PATH"
echo "SENTRY_URL: $SENTRY_URL"
echo "SENTRY_AUTH_TOKEN: $SENTRY_AUTH_TOKEN"
echo "SENTRY_ORG: $SENTRY_ORG"
echo "SENTRY_PROJECT: $SENTRY_PROJECT"

# Install Sentry CLI
PLATFORM=`uname -s`
if [[ $PLATFORM == CYGWIN* ]] || [[ $PLATFORM == MINGW* ]] || [[ $PLATFORM == MSYS* ]]; then
  PLATFORM="Windows"
fi
 
if [ $PLATFORM == "Windows" ]; then

    INSTALL_PATH=/c/sentry
    SENTRY_CLI=$INSTALL_PATH/sentry-cli
    if ! command -v $SENTRY_CLI &> /dev/null; then
        mkdir $INSTALL_PATH
        curl -SL --progress-bar "$SENTRY_DOWNLOAD_Windows_x86_64" > "$SENTRY_CLI"
        echo "sentry-cli now installed"
    fi

else
    SENTRY_CLI=sentry-cli
    if ! command -v $SENTRY_CLI &> /dev/null; then
        curl -sL "$SENTRY_DOWNLOAD_SCRIPT" | bash
        echo "sentry-cli now installed"
    fi
fi

# Upload symbols
export SENTRY_URL=$SENTRY_URL
export SENTRY_AUTH_TOKEN=$SENTRY_AUTH_TOKEN

$SENTRY_CLI upload-dif -o $SENTRY_ORG -p $SENTRY_PROJECT $SYMBOLS_PATH

if [ $? -eq 0 ]; then
    echo "Success symbols uploaded"
else
    echo "Failed symbols uploaded, code: $?"
fi