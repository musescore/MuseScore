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

#include "libmscore/mscore.h"

#include "palettetree.h"

#include "modularity/ioc.h"
#include "ipaletteadapter.h"
#include "ipaletteconfiguration.h"

namespace Ms {
class Palette;
class PalettePanel;
struct PaletteTree;

struct IconAction
{
    IconType subtype = IconType::NONE;
    const char* action = nullptr;
};

class PaletteCreator
{
    INJECT_STATIC(palette, mu::palette::IPaletteAdapter, adapter)
    INJECT_STATIC(palette, mu::palette::IPaletteConfiguration, configuration)

public:

    static Palette* newTempoPalette(bool defaultPalette = false);
    static Palette* newTextPalette(bool defaultPalette = false);
    static Palette* newTimePalette();
    static Palette* newRepeatsPalette();
    static Palette* newBeamPalette();
    static Palette* newDynamicsPalette(bool defaultPalette = false);
    static Palette* newFingeringPalette();
    static Palette* newTremoloPalette();
    static Palette* newNoteHeadsPalette();
    static Palette* newArticulationsPalette();
    static Palette* newOrnamentsPalette();
    static Palette* newAccordionPalette();
    static Palette* newBracketsPalette();
    static Palette* newBreathPalette();
    static Palette* newArpeggioPalette();
    static Palette* newClefsPalette(bool defaultPalette = false);
    static Palette* newGraceNotePalette();
    static Palette* newBagpipeEmbellishmentPalette();
    static Palette* newKeySigPalette();
    static Palette* newAccidentalsPalette(bool defaultPalette = false);
    static Palette* newBarLinePalette();
    static Palette* newLayoutPalette();
    static Palette* newLinesPalette();
    static Palette* newFretboardDiagramPalette();

    static PalettePanel* newTempoPalettePanel(bool defaultPalette = false);
    static PalettePanel* newTextPalettePanel(bool defaultPalette = false);
    static PalettePanel* newTimePalettePanel();
    static PalettePanel* newRepeatsPalettePanel();
    static PalettePanel* newBeamPalettePanel();
    static PalettePanel* newDynamicsPalettePanel(bool defaultPalette = false);
    static PalettePanel* newLayoutPalettePanel();
    static PalettePanel* newFingeringPalettePanel();
    static PalettePanel* newTremoloPalettePanel();
    static PalettePanel* newNoteHeadsPalettePanel();
    static PalettePanel* newArticulationsPalettePanel();
    static PalettePanel* newOrnamentsPalettePanel();
    static PalettePanel* newAccordionPalettePanel();
    static PalettePanel* newBracketsPalettePanel();
    static PalettePanel* newBreathPalettePanel();
    static PalettePanel* newArpeggioPalettePanel();
    static PalettePanel* newClefsPalettePanel(bool defaultPalette = false);
    static PalettePanel* newGraceNotePalettePanel();
    static PalettePanel* newBagpipeEmbellishmentPalettePanel();
    static PalettePanel* newKeySigPalettePanel();
    static PalettePanel* newAccidentalsPalettePanel(bool defaultPalette = false);
    static PalettePanel* newBarLinePalettePanel();
    static PalettePanel* newLinesPalettePanel();
    static PalettePanel* newFretboardDiagramPalettePanel();
    static Ms::PaletteTreePtr newMasterPaletteTree();
    static PaletteTree* newDefaultPaletteTree();
};

extern void populateIconPalette(Palette* p, const IconAction* a);
} // namespace Ms

#endif
