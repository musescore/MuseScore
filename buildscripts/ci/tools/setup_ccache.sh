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
BASE_DIR=$1

echo "Install ccache"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    sudo apt install ccache
elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew install ccache
else
    echo "Unsupported OS: $OSTYPE"; exit 1;
fi

echo "Setup config"

mkdir -p ~/.ccache
echo "base_dir = ${BASE_DIR}" > ~/.ccache/ccache.conf
echo "compression = true" >> ~/.ccache/ccache.conf
echo "compression_level = 6" >> ~/.ccache/ccache.conf
echo "max_size = 2G" >> ~/.ccache/ccache.conf
echo "sloppiness=pch_defines,time_macros" >> ~/.ccache/ccache.conf
cat ~/.ccache/ccache.conf
ccache -s
ccache -z      