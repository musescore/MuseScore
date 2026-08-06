#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

trap 'echo Setup failed; exit 1' ERR

HERE="$(dirname ${BASH_SOURCE[0]})"
ROOT_DIR="$HERE/../../.."
MF_DIR_NAME="muse"
MF_DIR="$ROOT_DIR/$MF_DIR_NAME"
BUILD_TOOLS=$HOME/build_tools
ENV_FILE=$BUILD_TOOLS/environment.sh
PACKARCH="x86_64" # x86_64, aarch64, wasm
COMPILER="gcc" # gcc, clang
EMSDK_VERSION="4.0.7" # for Qt 6.11.1
BUILD_PIPEWIRE=

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --arch) PACKARCH="$2"; shift ;;
        --compiler) COMPILER="$2"; shift ;;
        --build-pipewire) BUILD_PIPEWIRE="--build-pipewire" ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

bash $MF_DIR/buildscripts/ci/linux/setup.sh --arch $PACKARCH --compiler $COMPILER $BUILD_PIPEWIRE
