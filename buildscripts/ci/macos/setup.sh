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
echo "Setup macOS build environment"

trap 'echo Setup failed; exit 1' ERR

export MACOSX_DEPLOYMENT_TARGET=10.14

# Install build tools
echo "Install build tools"
brew install cmake ninja --formula --quiet

# Download dependencies
echo "Download dependencies"

wget -q --show-progress -O musescore_deps_macos.tar.gz https://raw.githubusercontent.com/cbjeukendrup/musescore_deps/main/musescore_deps_macos.tar.gz
mkdir -p $HOME/musescore_deps_macos
tar xf musescore_deps_macos.tar.gz -C $HOME/musescore_deps_macos
rm musescore_deps_macos.tar.gz

# Qt
export QT_SHORT_VERSION=6.2.4
echo "Download Qt $QT_SHORT_VERSION"
export QT_PATH=$HOME/Qt/$QT_SHORT_VERSION/
export PATH=$PATH:$QT_PATH/macos/bin
echo "PATH=$PATH" >> $GITHUB_ENV
# r2 - added websocket support
wget -nv -O qt.7z https://s3.amazonaws.com/utils.musescore.org/Qt624_mac_r2.7z
mkdir -p $QT_PATH
7z x -y qt.7z -o$QT_PATH
rm qt.7z

# VST SDK
echo "Download VST SDK"
wget -q --show-progress -O vst_sdk.7z "https://s3.amazonaws.com/utils.musescore.org/VST3_SDK_379.7z"
7z x -y vst_sdk.7z -o"$HOME/vst"
echo "VST3_SDK_PATH=$HOME/vst/VST3_SDK" >> $GITHUB_ENV

echo "Setup script done"
