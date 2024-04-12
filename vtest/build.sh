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

INSTALL_DIR="./install"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -i|--install-dir) INSTALL_DIR="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

mkdir -p $INSTALL_DIR

bash $HERE/../ninja_build.sh -t clean
MUSESCORE_INSTALL_DIR=$INSTALL_DIR \
MUSESCORE_BUILD_CONFIGURATION="vtest" \
bash ninja_build.sh -t installdebug