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
using namespace mu::ui;

static const mu::UriQuery MASTER_PALETTE_URI("musescore://palette/masterpalette?sync=false");

void PaletteActionsController::init()
{
    dispatcher()->reg(this, "masterpalette", this, &PaletteActionsController::toggleMasterPalette);

    interactive()->currentUri().ch.onReceive(this, [this](const Uri& uri) {
        //! NOTE If MasterPalette are not open, then it is reasonably to compare with the current uri,
        //! so as not to call the more expensive `interactive()->isOpened` method.
        //! If MasterPalette is open, then we will call `interactive()->isOpened`,
        //! in case if they suddenly did not close MasterPalette,
        //! but opened something else on top of MasterPalette.
        bool isOpened = false;
        if (!m_masterPaletteOpened.val) {
            isOpened = uri == MASTER_PALETTE_URI.uri();
        } else {
            isOpened = interactive()->isOpened(MASTER_PALETTE_URI.uri()).val;
        }

        if (isOpened != m_masterPaletteOpened.val) {
            m_masterPaletteOpened.val = isOpened;
            m_masterPaletteOpened.ch.send(isOpened);
        }
    });
}

mu::ValCh<bool> PaletteActionsController::isMasterPaletteOpened() const
{
    return m_masterPaletteOpened;
}

void PaletteActionsController::toggleMasterPalette()
{
    if (interactive()->isOpened(MASTER_PALETTE_URI.uri()).val) {
        interactive()->close(MASTER_PALETTE_URI.uri());
    } else {
        interactive()->open(MASTER_PALETTE_URI);
    }
}
