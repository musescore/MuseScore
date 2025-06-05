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

export MACOSX_DEPLOYMENT_TARGET=10.15

# Install build tools
echo "Install build tools"
brew install cmake ninja --formula --quiet

# Download dependencies
echo "Download dependencies"

wget -q --show-progress -O musescore_deps_macos.tar.gz https://raw.githubusercontent.com/cbjeukendrup/musescore_deps/main/musescore_deps_macos.tar.gz
mkdir -p $HOME/musescore_deps_macos
tar xf musescore_deps_macos.tar.gz -C $HOME/musescore_deps_macos
rm musescore_deps_macos.tar.gz

echo "Setup script done"
