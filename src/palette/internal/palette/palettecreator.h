/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
    INJECT_STATIC(palette, IPaletteConfiguration, configuration)

public:
    static PalettePanelPtr newTempoPalettePanel(bool defaultPalette = false);
    static PalettePanelPtr newTextPalettePanel(bool defaultPalette = false);
    static PalettePanelPtr newTimePalettePanel();
    static PalettePanelPtr newRepeatsPalettePanel();
    static PalettePanelPtr newBeamPalettePanel();
    static PalettePanelPtr newDynamicsPalettePanel(bool defaultPalette = false);
    static PalettePanelPtr newLayoutPalettePanel();
    static PalettePanelPtr newFingeringPalettePanel();
    static PalettePanelPtr newTremoloPalettePanel();
    static PalettePanelPtr newNoteHeadsPalettePanel();
    static PalettePanelPtr newArticulationsPalettePanel();
    static PalettePanelPtr newOrnamentsPalettePanel();
    static PalettePanelPtr newAccordionPalettePanel();
    static PalettePanelPtr newBracketsPalettePanel();
    static PalettePanelPtr newBreathPalettePanel();
    static PalettePanelPtr newArpeggioPalettePanel();
    static PalettePanelPtr newClefsPalettePanel(bool defaultPalette = false);
    static PalettePanelPtr newGraceNotePalettePanel();
    static PalettePanelPtr newBagpipeEmbellishmentPalettePanel();
    static PalettePanelPtr newKeySigPalettePanel();
    static PalettePanelPtr newAccidentalsPalettePanel(bool defaultPalette = false);
    static PalettePanelPtr newBarLinePalettePanel();
    static PalettePanelPtr newLinesPalettePanel();
    static PalettePanelPtr newFretboardDiagramPalettePanel();

    static PaletteTreePtr newMasterPaletteTree();
    static PaletteTreePtr newDefaultPaletteTree();
};
}

#endif // MU_PALETTE_PALETTECREATOR_H
