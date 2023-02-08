#!/usr/bin/env bash
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
MUSESCORE_BUILD_UPDATE_MODULE=OFF \
MUSESCORE_BUILD_SHORTCUTS_MODULE=OFF \
MUSESCORE_BUILD_NETWORK_MODULE=OFF \
MUSESCORE_BUILD_AUDIO_MODULE=OFF \
MUSESCORE_BUILD_LEARN_MODULE=OFF \
MUSESCORE_BUILD_WORKSPACE_MODULE=OFF \
MUSESCORE_BUILD_CLOUD_MODULE=OFF \
MUSESCORE_BUILD_LANGUAGES_MODULE=OFF \
MUSESCORE_BUILD_PLUGINS_MODULE=OFF \
MUSESCORE_BUILD_PLAYBACK_MODULE=OFF \
MUSESCORE_BUILD_PALETTE_MODULE=OFF \
MUSESCORE_BUILD_INSTRUMENTSSCENE_MODULE=OFF \
MUSESCORE_BUILD_INSPECTOR_MODULE=OFF \
MUSESCORE_BUILD_MULTIINSTANCES_MODULE=OFF \
MUSESCORE_BUILD_VIDEOEXPORT_MODULE=OFF \
MUSESCORE_BUILD_IMPORTEXPORT_MODULE=OFF \
bash ninja_build.sh -t installdebug