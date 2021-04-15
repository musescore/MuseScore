#!/usr/bin/env bash
##
## // SPDX-License-Identifier: GPL-3.0-only
## // MuseScore-CLA-applies
## //=============================================================================
## //  MuseScore
## //  Music Composition & Notation
## //
## //  Copyright (C) 2021 MuseScore BVBA and others
## //
## //  This program is free software: you can redistribute it and/or modify
## //  it under the terms of the GNU General Public License version 3 as
## //  published by the Free Software Foundation.
## //
## //  This program is distributed in the hope that it will be useful,
## //  but WITHOUT ANY WARRANTY; without even the implied warranty of
## //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## //  GNU General Public License for more details.
## //
## //  You should have received a copy of the GNU General Public License
## //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
## //=============================================================================##

ARTIFACTS_DIR=build.artifacts
MUSESCORE_BUILD_CONFIG=dev

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--build_config) MUSESCORE_BUILD_CONFIG="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$MUSESCORE_BUILD_CONFIG" ]; then echo "error: not set MUSESCORE_BUILD_CONFIG"; exit 1; fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG" 

export MSCORE_RELEASE_CHANNEL=$(cmake -DMUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG -P config.cmake | sed -n -e 's/^.*MSCORE_RELEASE_CHANNEL  *//p')

echo ${MSCORE_RELEASE_CHANNEL} > $ARTIFACTS_DIR/env/release_channel.env
cat $ARTIFACTS_DIR/env/release_channel.env