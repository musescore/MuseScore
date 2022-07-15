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

LUPDATE=lupdate
SRC_DIR=$HERE/../../src 
TS_FILE=$HERE/../../share/locale/musescore_en.ts
ARGS="-recursive -tr-function-alias translate+=trc -tr-function-alias translate+=mtrc -tr-function-alias translate+=qtrc -tr-function-alias qsTranslate+=qsTrc -extensions cpp,h,mm,ui,qml -no-obsolete"

# We only need to update one file, it will be sent to Transifex.
# We get .ts files for other languages from Transifex
$LUPDATE $ARGS $SRC_DIR -ts $TS_FILE
