/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_PALETTE_PALETTECREATOR_H
#define MU_PALETTE_PALETTECREATOR_H

#include "palettetree.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"

namespace mu::palette {
class PaletteCreator : public muse::Injectable
{
    muse::GlobalInject<IPaletteConfiguration> configuration;

public:

    PaletteCreator(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx)
    {
    }

    PalettePtr newTempoPalette(bool defaultPalette = false);
    PalettePtr newTextPalette(bool defaultPalette = false);
    PalettePtr newTimePalette(bool defaultPalette = false);
    PalettePtr newRepeatsPalette(bool defaultPalette = false);
    PalettePtr newBeamPalette();
    PalettePtr newDynamicsPalette(bool defaultPalette = false);
    PalettePtr newLayoutPalette(bool defaultPalette = false);
    PalettePtr newFingeringPalette(bool defaultPalette = false);
    PalettePtr newTremoloPalette();
    PalettePtr newNoteHeadsPalette();
    PalettePtr newArticulationsPalette(bool defaultPalette = false);
    PalettePtr newOrnamentsPalette(bool defaultPalette = false);
    PalettePtr newAccordionPalette();
    PalettePtr newBracketsPalette();
    PalettePtr newBreathPalette(bool defaultPalette = false);
    PalettePtr newArpeggioPalette();
    PalettePtr newClefsPalette(bool defaultPalette = false);
    PalettePtr newGraceNotePalette();
    PalettePtr newBagpipeEmbellishmentPalette();
    PalettePtr newKeySigPalette();
    PalettePtr newAccidentalsPalette(bool defaultPalette = false);
    PalettePtr newBarLinePalette(bool defaultPalette = false);
    PalettePtr newLinesPalette(bool defaultPalette = false);
    PalettePtr newFretboardDiagramPalette(bool defaultPalette = false);
    PalettePtr newGuitarPalette(bool defaultPalette = false);
    PalettePtr newKeyboardPalette();
    PalettePtr newPitchPalette(bool defaultPalette = false);
    PalettePtr newHarpPalette();
    PalettePtr newHandbellsPalette(bool defaultPalette = false);

    PaletteTreePtr newMasterPaletteTree();
    PaletteTreePtr newDefaultPaletteTree();
};
}

#endif // MU_PALETTE_PALETTECREATOR_H
