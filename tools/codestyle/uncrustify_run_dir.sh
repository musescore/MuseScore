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
HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

DIR="${1-.}" # use $1 or "." (current dir) if $1 is not defined

START_TIME=$(date +%s)

find $DIR -type f -regex '.*\.\(cpp\|h\|hpp\|cc\)$' | xargs -n 1 -P 16 \
    uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l CPP
find $DIR -type f -regex '.*\.\(mm\)$' | xargs -n 1 -P 16 \
    uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l OC+

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo ""
echo "time: $DIFF_TIME sec, complete: $DIR"
echo ""
