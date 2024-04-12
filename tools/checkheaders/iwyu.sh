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

# See https://github.com/musescore/musescore_devtools/tree/main/include-what-you-use

BUILD_DIR=$1
OUT=$2

IWYU_TOOL=$(which iwyu_tool.py)
IWYU_TOOL_DIR=$(dirname $IWYU_TOOL)
IWYU_IMP=$IWYU_TOOL_DIR/../imp/default.imp

echo "$IWYU_TOOL -p $BUILD_DIR -- -Xiwyu --mapping_file=$IWYU_IMP --transitive_includes_only --no_comments --cxx17ns > $OUT"
$IWYU_TOOL -p $BUILD_DIR -- -Xiwyu --mapping_file=$IWYU_IMP -Xiwyu --transitive_includes_only -Xiwyu --no_comments -Xiwyu --cxx17ns > $OUT
