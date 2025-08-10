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

trap 'echo Generate PNGs failed; exit 1' ERR

sudo apt-get install libegl1 libfuse2 -y
export QT_QPA_PLATFORM=offscreen

REF_BIN=./musescore_reference/MuseScore-Studio-vtest.AppImage
chmod +x $REF_BIN

echo reference version:
$REF_BIN --long-version

CUR_BIN=./musescore_current/MuseScore-Studio-vtest.AppImage
chmod +x $CUR_BIN

echo current version:
$CUR_BIN --long-version

echo =======================
echo ==== Generate PNGs ====
echo =======================

echo Generate reference pngs: 
./vtest/vtest-generate-pngs.sh -o ./reference_pngs -m $REF_BIN
./vtest/vtest-generate-pngs.sh -o ./reference_pngs_small -m $REF_BIN -s ./vtest/scores_small -d 460 -S ./vtest/small.mss
./vtest/vtest-generate-pngs.sh -o ./reference_pngs_gp_small -m $REF_BIN -s ./vtest/gp_small -d 460 -S ./vtest/small.mss --gp-linked

echo Generate current pngs: 
./vtest/vtest-generate-pngs.sh -o ./current_pngs -m $CUR_BIN
./vtest/vtest-generate-pngs.sh -o ./current_pngs_small -m $CUR_BIN -s ./vtest/scores_small -d 460 -S ./vtest/small.mss
./vtest/vtest-generate-pngs.sh -o ./current_pngs_gp_small -m $CUR_BIN -s ./vtest/gp_small -d 460 -S ./vtest/small.mss --gp-linked
