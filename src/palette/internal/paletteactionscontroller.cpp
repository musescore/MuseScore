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

#include "paletteactionscontroller.h"

using namespace mu::palette;

static std::string MASTER_PALETTE_URI = "musescore://palette/masterpalette";

void PaletteActionsController::init()
{
    dispatcher()->reg(this, "masterpalette", this, &PaletteActionsController::toggleMasterPalette);
}

mu::ValCh<bool> PaletteActionsController::isMasterPaletteOpened() const
{
    ValCh<bool> result;
    result.ch = m_masterPaletteOpenChannel;
    result.val = interactive()->isOpened(MASTER_PALETTE_URI).val;
    return result;
}

void PaletteActionsController::toggleMasterPalette()
{
    if (interactive()->isOpened(MASTER_PALETTE_URI).val) {
        interactive()->close(MASTER_PALETTE_URI);
    } else {
        interactive()->open(MASTER_PALETTE_URI + "?sync=false");
    }

    m_masterPaletteOpenChannel.send(interactive()->isOpened(MASTER_PALETTE_URI).val);
}
