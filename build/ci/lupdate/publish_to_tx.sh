#!/bin/bash
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
BUILD_TOOLS=$HOME/build_tools
mkdir -p $BUILD_TOOLS

TRANSIFEX_API_TOKEN=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--token) TRANSIFEX_API_TOKEN="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$TRANSIFEX_API_TOKEN" ]; then echo "error: not set TRANSIFEX_API_TOKEN"; exit 1; fi

# Install tx
CUR_DIR=$(pwd)
mkdir -p $BUILD_TOOLS/tx
cd $BUILD_TOOLS/tx
curl -o- https://raw.githubusercontent.com/transifex/cli/master/install.sh | bash
cd $CUR_DIR
ls $BUILD_TOOLS/tx/
TX=$BUILD_TOOLS/tx/tx

echo "tx version: $($TX --version)"
echo "tx push:"
$TX -t $TRANSIFEX_API_TOKEN push -s