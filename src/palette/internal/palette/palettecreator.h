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

namespace mu::palette {
class PaletteWidget;
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
    static PaletteWidget* newTempoPalette(bool defaultPalette = false);
    static PaletteWidget* newTextPalette(bool defaultPalette = false);
    static PaletteWidget* newTimePalette();
    static PaletteWidget* newRepeatsPalette();
    static PaletteWidget* newBeamPalette();
    static PaletteWidget* newDynamicsPalette(bool defaultPalette = false);
    static PaletteWidget* newFingeringPalette();
    static PaletteWidget* newTremoloPalette();
    static PaletteWidget* newNoteHeadsPalette();
    static PaletteWidget* newArticulationsPalette();
    static PaletteWidget* newOrnamentsPalette();
    static PaletteWidget* newAccordionPalette();
    static PaletteWidget* newBracketsPalette();
    static PaletteWidget* newBreathPalette();
    static PaletteWidget* newArpeggioPalette();
    static PaletteWidget* newClefsPalette(bool defaultPalette = false);
    static PaletteWidget* newGraceNotePalette();
    static PaletteWidget* newBagpipeEmbellishmentPalette();
    static PaletteWidget* newKeySigPalette();
    static PaletteWidget* newAccidentalsPalette(bool defaultPalette = false);
    static PaletteWidget* newBarLinePalette();
    static PaletteWidget* newLayoutPalette();
    static PaletteWidget* newLinesPalette();
    static PaletteWidget* newFretboardDiagramPalette();

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
    static void populateIconPalette(PaletteWidget* palette, const PaletteActionIconList& actions);
};
} // namespace Ms

#endif
