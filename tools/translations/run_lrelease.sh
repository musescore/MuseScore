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

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

LRELEASE=lrelease
TS_DIR=$HERE/../../share/locale 
QM_DIR=$HERE/../../share/locale 

for f in $TS_DIR/{musescore,instruments,qt}_*.ts
do
  FILE_NAME=$(basename $f .ts)
  $LRELEASE $f -qm $QM_DIR/${FILE_NAME}.qm 
done
