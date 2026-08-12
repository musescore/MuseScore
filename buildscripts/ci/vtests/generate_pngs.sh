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

trap 'echo Generate PNGs failed; exit 1' ERR

vtest_dir=$1

pushd $vtest_dir
chmod +x ./MuseScore-Studio-vtest.AppImage
./MuseScore-Studio-vtest.AppImage --appimage-extract
popd

MSCORE=$vtest_dir/squashfs-root/AppRun

echo Version:
$MSCORE --long-version

echo Generating PNGs:
./vtest/vtest-generate-pngs.sh -o $vtest_dir/pngs -m $MSCORE
./vtest/vtest-generate-pngs.sh -o $vtest_dir/pngs_small -m $MSCORE -s ./vtest/scores_small -d 460 -S ./vtest/small.mss
./vtest/vtest-generate-pngs.sh -o $vtest_dir/pngs_gp_small -m $MSCORE -s ./vtest/gp_small -d 460 -S ./vtest/small.mss --gp-linked
