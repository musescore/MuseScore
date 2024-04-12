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
echo "Generate dump symbols"

GEN_SCRIPT=tools/crashdump/generate_syms.sh
DUMPSYMS_BIN=$HOME/breakpad/dump_syms
ARTIFACTS_DIR=build.artifacts
BUILD_DIR=applebuild
SYMBOLS_DIR=$ARTIFACTS_DIR/symbols
MSCORE_BIN=$BUILD_DIR/mscore.app/Contents/MacOS/mscore

echo "GEN_SCRIPT: $GEN_SCRIPT"
echo "DUMPSYMS_BIN: $DUMPSYMS_BIN"
echo "BUILD_DIR: $BUILD_DIR"
echo "SYMBOLS_DIR: $SYMBOLS_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"

$GEN_SCRIPT --dumpsyms-bin $DUMPSYMS_BIN --build-dir $BUILD_DIR --symbols-dir $SYMBOLS_DIR --mscore-bin $MSCORE_BIN

echo "-----"
ls $SYMBOLS_DIR