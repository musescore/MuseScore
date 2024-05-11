#!/bin/bash
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

BUILD_TOOLS=$HOME/build_tools
mkdir -p $BUILD_TOOLS
mkdir -p $BUILD_TOOLS/tx

ENV_FILE=$BUILD_TOOLS/tx/tx_environment.sh
rm -f $ENV_FILE

TRANSIFEX_API_TOKEN=""
OS="linux" # linux, windows, macos

TX_VERSION=v1.6.11
TX_LIN_URL="https://github.com/transifex/cli/releases/download/${TX_VERSION}/tx-linux-amd64.tar.gz"
TX_WIN_URL="https://github.com/transifex/cli/releases/download/${TX_VERSION}/tx-windows-386.zip"
TX_MAC_URL="https://github.com/transifex/cli/releases/download/${TX_VERSION}/tx-darwin-amd64.tar.gz"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--token) TRANSIFEX_API_TOKEN="$2"; shift ;;
        -s|--os) OS="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$TRANSIFEX_API_TOKEN" ]; then echo "error: not set TRANSIFEX_API_TOKEN"; exit 1; fi
if [ -z "$OS" ]; then echo "warn: not set OS, will install for linux"; fi

if [ "$OS" == "linux" ]; then
    wget -q --show-progress --no-check-certificate -O $BUILD_TOOLS/tx/tx-linux-amd64.tar.gz "${TX_LIN_URL}" 
    7z x $BUILD_TOOLS/tx/tx-linux-amd64.tar.gz -so | 7z x -aoa -si -ttar -o"$BUILD_TOOLS/tx/"
fi

if [ "$OS" == "windows" ]; then
    wget -q --show-progress --no-check-certificate -O $BUILD_TOOLS/tx/tx-windows-386.zip "${TX_WIN_URL}" 
    7z x -y $BUILD_TOOLS/tx/tx-windows-386.zip -o"$BUILD_TOOLS/tx/"
fi

if [ "$OS" == "macos" ]; then
    wget -q --show-progress --no-check-certificate -O $BUILD_TOOLS/tx/tx-darwin-amd64.tar.gz "${TX_MAC_URL}" 
    7z x $BUILD_TOOLS/tx/tx-darwin-amd64.tar.gz -so | 7z x -aoa -si -ttar -o"$BUILD_TOOLS/tx/"
fi

ls $BUILD_TOOLS/tx/

echo export PATH="$BUILD_TOOLS/tx/:\${PATH}" >> ${ENV_FILE}
echo export TX_TOKEN="$TRANSIFEX_API_TOKEN" >> ${ENV_FILE}

chmod +x "$BUILD_TOOLS/tx/tx"
chmod +x "${ENV_FILE}"
