/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_PALETTE_PALETTE_CONFIG_H
#define MU_PALETTE_PALETTE_CONFIG_H

//! TODO Added to replace the old code.
//! We should probably use configuration instead of these variables.
namespace mu::palette {
static constexpr double DPI_DISPLAY     = 96.0;  // 96 DPI nominal resolution
static constexpr double DPMM_DISPLAY    = DPI_DISPLAY / 25.4;
static constexpr double PALETTE_SPATIUM = 1.764 * DPMM_DISPLAY;
}

#endif // MU_PALETTE_PALETTE_CONFIG_H
