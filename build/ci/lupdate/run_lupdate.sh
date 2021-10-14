#!/bin/bash
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
ENV_FILE=./../musescore_lupdate_environment.sh
source $ENV_FILE

# Translation routines
# update translation on transifex
# remove obsolete strings
OBSOLETE=-no-obsolete # '-noobsolete' in older QT versions

./build/gen-qt-projectfile . > mscore.pro
lupdate ${OBSOLETE} mscore.pro
./build/gen-instruments-projectfile ./share/instruments > instruments.pro
lupdate ${OBSOLETE} instruments.pro

rm mscore.pro
rm instruments.pro
