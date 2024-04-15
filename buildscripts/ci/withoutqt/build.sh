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

#set -x
trap 'echo Build failed; exit 1' ERR

BUILD_TOOLS=$HOME/build_tools

cat $BUILD_TOOLS/environment.sh
source $BUILD_TOOLS/environment.sh

cd ./tools/check_build_without_qt
mkdir -p build-check_build_without_qt
cd build-check_build_without_qt

cmake .. -GNinja -DCMAKE_BUILD_TYPE="Debug" 

ninja 


