#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio Studio
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
HERE="${BASH_SOURCE%/*}"
STACKWALK_BIN=$(which minidump_stackwalk)
SYMBOLS_DIR="$HERE/../../build.symbols"
DUMP_FILE=""
OUT_FILE=""
SHOW_HELP=0

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --stackwalk-bin) STACKWALK_BIN="$2"; shift ;;
        --symbols-dir) SYMBOLS_DIR="$2"; shift;;
		-d|--dump) DUMP_FILE="$2"; shift;;
        -o|--out) OUT_FILE="$2"; shift;;
		-h|--help) SHOW_HELP=1; shift;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ $SHOW_HELP -eq 1 ]; then
	echo "Usage: view_dump.sh [OPTION]..."
	echo "Generate dump symbols"
	echo " "
  	echo "    --stackwalk-bin   path to stackwalk binary, default use path from environment path" 
	echo "    --symbols-dir     path to output symbols dir, default '../../build.symbols'"
    echo "-d, --dump            path to dump file"
    echo "-o, --out             path to out file, default stdout"
	echo "-h, --help            display this help and exit"
	echo " "
	echo "Example:"
	echo "  view_dump.sh -d ./cf55874a-2293-4fd4-86b0-9d448e640bdc.dmp"
	exit 0
fi


if [ -z "$STACKWALK_BIN" ]; then echo "error: not set STACKWALK_BIN"; exit 1; fi
if [ -z "$SYMBOLS_DIR" ]; then echo "error: not set SYMBOLS_DIR"; exit 1; fi
if [ -z "$DUMP_FILE" ]; then echo "error: not set DUMP_FILE"; exit 1; fi

echo "STACKWALK_BIN: $STACKWALK_BIN"
echo "SYMBOLS_DIR: $SYMBOLS_DIR"
echo "DUMP_FILE: $DUMP_FILE"
echo "OUT_FILE: $OUT_FILE"

if [ -z "$OUT_FILE" ]; then 
    $STACKWALK_BIN $DUMP_FILE $SYMBOLS_DIR 2>/dev/null
else
    $STACKWALK_BIN $DUMP_FILE $SYMBOLS_DIR > $OUT_FILE 2>/dev/null
fi


