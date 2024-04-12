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
HERE="$(dirname ${BASH_SOURCE[0]})"
DIR="${1-.}" # use $1 or "." (current dir) if $1 is not defined

source "${HERE}/globals.source"

SCAN_BIN_DIR=""

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    SCAN_BIN_DIR="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    if [ "$(uname -m)" = "arm64" ]; then
        SCAN_BIN_DIR="macosx/arm64"
    else
        SCAN_BIN_DIR="macosx/x86_64"
    fi
elif [[ "$OSTYPE" == "cygwin" ]]; then
    SCAN_BIN_DIR="windows"
elif [[ "$OSTYPE" == "msys" ]]; then
    SCAN_BIN_DIR="windows"
fi

SCAN_BIN=$HERE/scan_files/bin/${SCAN_BIN_DIR}/scan_files

START_TIME=$(date +%s)

$SCAN_BIN -d $DIR -i $UNTIDY_FILE -e cpp,c,cc,hpp,h | xargs -n 1 -P 16 uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l CPP
$SCAN_BIN -d $DIR -i $UNTIDY_FILE -e mm | xargs -n 1 -P 16 uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l OC+

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo ""
echo "time: $DIFF_TIME sec, complete: $DIR"
echo ""
