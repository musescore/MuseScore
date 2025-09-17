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

echo "Download dependencies"
trap 'code=$?; echo "error: Download dependencies: command \`$BASH_COMMAND\` exited with code $code." >&2; exit 1' ERR

wget -q --show-progress -O musescore_deps_macos.tar.gz https://raw.githubusercontent.com/cbjeukendrup/musescore_deps/main/musescore_deps_macos.tar.gz
mkdir -p $HOME/musescore_deps_macos
tar xf musescore_deps_macos.tar.gz -C $HOME/musescore_deps_macos
rm musescore_deps_macos.tar.gz

echo "Download dependencies done"
