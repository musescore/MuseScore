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

# This is script for generate dump symbols.
# Used on CI and can be used locally 

HERE="${BASH_SOURCE%/*}"
DUMSYMS_BIN=$(which dump_syms)
BUILD_DIR=""
SYMBOLS_DIR="$HERE/../../build.symbols"
MSCORE_BIN=""
ARCH=""
CLEAR="--clear"
SHOW_HELP=0

GEN_SCRIPT="$HERE/posix/generate_breakpad_symbols.py"
if [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
    GEN_SCRIPT="$HERE/win/generate_breakpad_symbols.py"
fi

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --dumpsyms-bin) DUMSYMS_BIN="$2"; shift ;;
        --build-dir) BUILD_DIR="$2"; shift ;;
        --symbols-dir) SYMBOLS_DIR="$2"; shift;;
        --mscore-bin) MSCORE_BIN="$2"; shift;;
        --arch) ARCH="--arch=$2"; shift;;
        --no-clear) CLEAR="";;
        -h|--help) SHOW_HELP=1; shift;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ $SHOW_HELP -eq 1 ]; then
    echo "Usage: generate_syms.sh [OPTION]..."
    echo "Generate dump symbols"
    echo " "
    echo "    --dumpsyms-bin    path to dump_syms binary, default use path from environment path"
    echo "    --build-dir       path to build dir"  
    echo "    --symbols-dir     path to output symbols dir, default '../../build.symbols'"
    echo "    --mscore-bin      path to mscore binary"
    echo "-h, --help            display this help and exit"
    echo " "
    echo "Example:"
    echo "  generate_syms.sh --build-dir ../../build.debug --mscore-bin ../../build.debug/install/bin/mscore"
    exit 0
fi


if [ -z "$DUMSYMS_BIN" ]; then echo "error: not set DUMSYMS_BIN"; exit 1; fi
if [ -z "$BUILD_DIR" ]; then echo "error: not set BUILD_DIR"; exit 1; fi
if [ -z "$SYMBOLS_DIR" ]; then echo "error: not set SYMBOLS_DIR"; exit 1; fi
if [ -z "$MSCORE_BIN" ]; then echo "error: not set MSCORE_BIN"; exit 1; fi

echo "DUMSYMS_BIN: $DUMSYMS_BIN"
echo "BUILD_DIR: $BUILD_DIR"
echo "SYMBOLS_DIR: $SYMBOLS_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"

$GEN_SCRIPT --dumpsyms-bin=$DUMSYMS_BIN --build-dir=$BUILD_DIR --symbols-dir=$SYMBOLS_DIR --binary=$MSCORE_BIN $ARCH $CLEAR --verbose
