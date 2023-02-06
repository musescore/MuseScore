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
CURRENT_DIR="./current_pngs"
REFERENCE_DIR="./reference_pngs"
OUTPUT_DIR="./comparison"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--current-dir) CURRENT_DIR="$2"; shift ;;
        -r|--reference-dir) REFERENCE_DIR="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
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
# $HERE/vtest-compare-pngs.sh -c $CURRENT_DIR -r $REFERENCE_DIR -o $OUTPUT_DIR

# Data
# $HERE/vtest-compare-data.sh -c $CURRENT_DIR -r $REFERENCE_DIR -o $OUTPUT_DIR

# Js
$HERE/vtest-compare-js.sh -c $CURRENT_DIR -r $REFERENCE_DIR -o $OUTPUT_DIR