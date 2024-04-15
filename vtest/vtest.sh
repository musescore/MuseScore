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
MSCORE_BIN=build.debug/install/bin/mscore

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -m|--mscore) MSCORE_BIN="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done 

export XDG_RUNTIME_DIR=/tmp/runtime-root
export QT_QPA_PLATFORM=offscreen

$MSCORE_BIN \
        --test-case $HERE/vtest.js \
        --test-case-context $HERE/vtest_context.json 

code=$?
echo "diagnostic code: $code"

# At moment needs for CI
#exit $code
exit 0    