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

BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh

QT5_COMPAT="OFF"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --qt5_compat) QT5_COMPAT="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

mkdir -p $BUILD_TOOLS
rm -f $ENV_FILE

QT_DIR="/c/Qt/6.2.4"
if [[ "$QT5_COMPAT" == "ON" ]]; then
    QT_DIR="/c/Qt/5.15.2" 
fi

echo export PATH="${QT_DIR}/msvc2019_64/bin:\${PATH}" >> ${ENV_FILE}

chmod +x "$ENV_FILE"

cat $ENV_FILE