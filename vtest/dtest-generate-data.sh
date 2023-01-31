#!/usr/bin/env bash
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
echo "MuseScore DTest Generate draw data"

set -o pipefail

HERE="$(dirname ${BASH_SOURCE[0]})"
SCORES_DIR="$HERE/scores"
OUTPUT_DIR="./current_datas"
MSCORE_BIN=build.debug/install/bin/mscore

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -s|--scores) SCORES_DIR="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
        -m|--mscore) MSCORE_BIN="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "::group::Configuration:"
echo "SCORES_DIR: $SCORES_DIR"
echo "OUTPUT_DIR: $OUTPUT_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"
echo "::endgroup::"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

LOG_FILE=$OUTPUT_DIR/convert.log

echo "::group::Generating Draw Data"
$MSCORE_BIN --diagnostic-gen-drawdata $SCORES_DIR --diagnostic-output $OUTPUT_DIR 2>&1 | tee $LOG_FILE && SUCCESS="true"
echo "::endgroup::"

if [ -z "$SUCCESS" ]; then
    echo -e "\033[0;31mGenerating Draw Datas failed!\033[0m"
fi

echo "::group::Generated files:"
ls $OUTPUT_DIR
echo "::endgroup::"

if [ -z "$SUCCESS" ]; then
    exit 1
fi
