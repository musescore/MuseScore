//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
