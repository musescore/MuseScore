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
echo "Setup MacOS environment for run lupdate"
trap 'echo Setup failed; exit 1' ERR

#  Go one-up from MuseScore root dir
cd ..

# Let's remove the file with environment variables to recreate it
ENV_FILE=./musescore_lupdate_environment.sh
rm -f ${ENV_FILE}

echo "echo 'Setup MacOS environment for run lupdate'" >> ${ENV_FILE}

QT_PATH=$HOME/Qt/598/clang_64
wget -nv -O qt5.zip https://s3.amazonaws.com/utils.musescore.org/qt598_mac.zip
mkdir -p $QT_PATH
unzip -qq qt5.zip -d $QT_PATH

echo export PATH="${QT_PATH}/bin:\${PATH}" >> ${ENV_FILE}
echo export LD_LIBRARY_PATH="${QT_PATH}/lib:\${LD_LIBRARY_PATH}" >> ${ENV_FILE}
echo export QT_PATH="${QT_PATH}" >> ${ENV_FILE}
echo export QT_PLUGIN_PATH="${QT_PATH}/plugins" >> ${ENV_FILE}
echo export QML2_IMPORT_PATH="${QT_PATH}/qml" >> ${ENV_FILE}


chmod +x "${ENV_FILE}"

echo "Setup script done"