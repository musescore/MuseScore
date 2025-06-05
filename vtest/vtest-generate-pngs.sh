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
echo "MuseScore VTest Generate PNGs"

set -o pipefail

HERE="$(dirname ${BASH_SOURCE[0]})"
SCORES_DIR="$HERE/scores"
OUTPUT_DIR="./vtest_pngs"
MSCORE_BIN=build.debug/install/bin/mscore
DPI=180

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -s|--scores) SCORES_DIR="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
        -m|--mscore) MSCORE_BIN="$2"; shift ;;
        -d|--dpi) DPI="$2"; shift ;;
        -S|--style) STYLE_PATH="$2"; shift ;;
        --gp-linked) GP_LINKED="--gp-linked"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "::group::Configuration:"
echo "SCORES_DIR: $SCORES_DIR"
echo "OUTPUT_DIR: $OUTPUT_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"
echo "DPI: $DPI"
echo "STYLE_PATH: $STYLE_PATH"
echo "::endgroup::"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

LOG_FILE=$OUTPUT_DIR/convert.log
JSON_FILE=$OUTPUT_DIR/vtestjob.json

echo "::group::Generating JSON job file"
echo "[" >> $JSON_FILE
SCORES_LIST=$(ls -p $SCORES_DIR | grep -v /)
for score in $SCORES_LIST ; do
    OUT_FILE=$OUTPUT_DIR/${score%.*}.png
    echo "{ \"in\" : \"$SCORES_DIR/$score\", \"out\" : \"$OUT_FILE\" }," >> $JSON_FILE;
done
echo "{}]" >> $JSON_FILE
cat $JSON_FILE
echo "::endgroup::"

echo "::group::Generating PNG files"
if [ -z "$STYLE_PATH" ]; then
    $MSCORE_BIN -j $JSON_FILE -r $DPI $GP_LINKED 2>&1 | tee $LOG_FILE && SUCCESS="true"
else
    $MSCORE_BIN -S $STYLE_PATH -j $JSON_FILE -r $DPI $GP_LINKED 2>&1 | tee $LOG_FILE && SUCCESS="true"
fi
echo "::endgroup::"

if [ -z "$SUCCESS" ]; then
    echo -e "\033[0;31mGenerating PNGs failed!\033[0m"
fi

echo "::group::Generated files:"
ls $OUTPUT_DIR
echo "::endgroup::"

if [ -z "$SUCCESS" ]; then
   exit 1
fi
