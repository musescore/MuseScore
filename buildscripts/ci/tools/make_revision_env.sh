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
REVISION=$1
ARTIFACTS_DIR=build.artifacts

if [ -z "$REVISION" ]; then 
    REVISION=$(git rev-parse --short=7 HEAD)
fi

if [ -z "$REVISION" ]; then echo "error: not set REVISION"; exit 1; fi

echo $REVISION > $ARTIFACTS_DIR/env/build_revision.env
cat $ARTIFACTS_DIR/env/build_revision.env
