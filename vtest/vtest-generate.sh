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

# We are exploring how to improve the testing of engraving draw 
# At the moment we have three options: 
# * Export to png and comparison of these png by a third-party program
# * Export to draw data (json) and their comparison (png are created only for clarity)
# * Calling the js script in which the necessary actions are written
# Now the favorite option is to call a js script. 
# It's all at the experimental stage, I don't want to delete other options at the moment


# Pngs
# $HERE/vtest-generate-pngs.sh -s $SCORES_DIR -o $OUTPUT_DIR -m $MSCORE_BIN

# Data
# $HERE/vtest-generate-data.sh -s $SCORES_DIR -o $OUTPUT_DIR -m $MSCORE_BIN

# Js
$HERE/vtest-generate-js.sh -s $SCORES_DIR -o $OUTPUT_DIR -m $MSCORE_BIN