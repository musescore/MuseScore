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

#ifndef __PALETTECREATER_H__
#define __PALETTECREATER_H__

#include "libmscore/actionicon.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "ui/iuiactionsregister.h"

namespace Ms {
class Palette;
}

namespace mu::palette {
struct PaletteActionIcon
{
    Ms::ActionIconType actionType = Ms::ActionIconType::UNDEFINED;
    actions::ActionCode actionCode;
};
using PaletteActionIconList = std::vector<PaletteActionIcon>;

class PaletteCreator
{
    INJECT_STATIC(palette, IPaletteConfiguration, configuration)
    INJECT_STATIC(palette, ui::IUiActionsRegister, actionsRegister)

public:
    static Ms::Palette* newTempoPalette(bool defaultPalette = false);
    static Ms::Palette* newTextPalette(bool defaultPalette = false);
    static Ms::Palette* newTimePalette();
    static Ms::Palette* newRepeatsPalette();
    static Ms::Palette* newBeamPalette();
    static Ms::Palette* newDynamicsPalette(bool defaultPalette = false);
    static Ms::Palette* newFingeringPalette();
    static Ms::Palette* newTremoloPalette();
    static Ms::Palette* newNoteHeadsPalette();
    static Ms::Palette* newArticulationsPalette();
    static Ms::Palette* newOrnamentsPalette();
    static Ms::Palette* newAccordionPalette();
    static Ms::Palette* newBracketsPalette();
    static Ms::Palette* newBreathPalette();
    static Ms::Palette* newArpeggioPalette();
    static Ms::Palette* newClefsPalette(bool defaultPalette = false);
    static Ms::Palette* newGraceNotePalette();
    static Ms::Palette* newBagpipeEmbellishmentPalette();
    static Ms::Palette* newKeySigPalette();
    static Ms::Palette* newAccidentalsPalette(bool defaultPalette = false);
    static Ms::Palette* newBarLinePalette();
    static Ms::Palette* newLayoutPalette();
    static Ms::Palette* newLinesPalette();
    static Ms::Palette* newFretboardDiagramPalette();

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

    static void populateIconPalettePanel(PalettePanelPtr palettePanel, const PaletteActionIconList& actions);

    // Used in NoteGroups widget
    static void populateIconPalette(Ms::Palette* palette, const PaletteActionIconList& actions);
};
} // namespace Ms

#endif
