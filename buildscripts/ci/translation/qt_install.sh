#!/bin/bash
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
echo "Setup environment for run lupdate"
trap 'echo Setup failed; exit 1' ERR

BUILD_TOOLS=$HOME/build_tools
mkdir -p $BUILD_TOOLS

ENV_FILE=$BUILD_TOOLS/environment.sh
rm -f $ENV_FILE

echo "echo 'Setup environment for run lupdate'" >> ${ENV_FILE}

##########################################################################
# GET QT
##########################################################################
qt_version="624"
qt_dir="$BUILD_TOOLS/Qt/${qt_version}"
if [[ ! -d "${qt_dir}" ]]; then
  mkdir -p "${qt_dir}"
  qt_url="https://s3.amazonaws.com/utils.musescore.org/Qt${qt_version}_gcc64.7z"
  wget -q --show-progress -O qt.7z "${qt_url}"
  7z x -y qt.7z -o"${qt_dir}"
  rm qt.7z
fi

echo export PATH="${qt_dir}/bin:\${PATH}" >> ${ENV_FILE}

chmod +x "${ENV_FILE}"

echo "Setup script done"
