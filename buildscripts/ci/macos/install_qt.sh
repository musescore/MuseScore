#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

echo "Install Qt"
trap 'code=$?; echo "error: Install Qt: command \`$BASH_COMMAND\` exited with code $code." >&2; exit 1' ERR

QT_DIR=$HOME/Qt/6.10.2/macos

wget -q --show-progress -O Qt-6.10.2-macOS-10.15.7z 'https://github.com/cbjeukendrup/musescore_build_qt/releases/download/v21960860930/Qt-6.10.2-macOS-10.15.7z'
7z x Qt-6.10.2-macOS-10.15.7z -o$QT_DIR
rm Qt-6.10.2-macOS-10.15.7z

echo $QT_DIR/bin >> $GITHUB_PATH

echo QT_ROOT_DIR=$QT_DIR >> $GITHUB_ENV
echo QT_PLUGIN_PATH=$QT_DIR/plugins >> $GITHUB_ENV
echo QML2_IMPORT_PATH=$QT_DIR/qml >> $GITHUB_ENV

echo "Install Qt done"
