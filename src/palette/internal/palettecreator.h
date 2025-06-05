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
class PaletteCreator
{
    INJECT_STATIC(IPaletteConfiguration, configuration)

public:
    static PalettePtr newTempoPalette(bool defaultPalette = false);
    static PalettePtr newTextPalette(bool defaultPalette = false);
    static PalettePtr newTimePalette(bool defaultPalette = false);
    static PalettePtr newRepeatsPalette(bool defaultPalette = false);
    static PalettePtr newBeamPalette();
    static PalettePtr newDynamicsPalette(bool defaultPalette = false);
    static PalettePtr newLayoutPalette(bool defaultPalette = false);
    static PalettePtr newFingeringPalette(bool defaultPalette = false);
    static PalettePtr newTremoloPalette();
    static PalettePtr newNoteHeadsPalette();
    static PalettePtr newArticulationsPalette(bool defaultPalette = false);
    static PalettePtr newOrnamentsPalette(bool defaultPalette = false);
    static PalettePtr newAccordionPalette();
    static PalettePtr newBracketsPalette();
    static PalettePtr newBreathPalette(bool defaultPalette = false);
    static PalettePtr newArpeggioPalette();
    static PalettePtr newClefsPalette(bool defaultPalette = false);
    static PalettePtr newGraceNotePalette();
    static PalettePtr newBagpipeEmbellishmentPalette();
    static PalettePtr newKeySigPalette();
    static PalettePtr newAccidentalsPalette(bool defaultPalette = false);
    static PalettePtr newBarLinePalette(bool defaultPalette = false);
    static PalettePtr newLinesPalette(bool defaultPalette = false);
    static PalettePtr newFretboardDiagramPalette(bool defaultPalette = false);
    static PalettePtr newGuitarPalette(bool defaultPalette = false);
    static PalettePtr newKeyboardPalette();
    static PalettePtr newPitchPalette(bool defaultPalette = false);
    static PalettePtr newHarpPalette();

    static PaletteTreePtr newMasterPaletteTree();
    static PaletteTreePtr newDefaultPaletteTree();
};
}

#endif // MU_PALETTE_PALETTECREATOR_H
