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

# Go to repository root directory regardless of where script was run from:
cd "${BASH_SOURCE%/*}/../.."

HERE="tools/codestyle" # path to dir that contains this script

source "${HERE}/globals.source"

START_TIME=$(date +%s)

for dir in "${TIDY_DIRS[@]}"; do
    "${HERE}/uncrustify_run_dir.sh" "${dir}"
done

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo "time: $DIFF_TIME sec, complete all"
