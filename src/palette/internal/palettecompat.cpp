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

#include <memory>

#include "palettecompat.h"

#include "engraving/rw/compat/compatutils.h"

#include "engraving/dom/actionicon.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/score.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stringtunings.h"
#include "engraving/dom/capo.h"
#include "engraving/dom/marker.h"
#include "engraving/types/symid.h"
#include "engraving/types/typesconv.h"

#include "palette.h"
#include "palettecell.h"

using namespace mu::palette;
using namespace mu::engraving;

static const std::unordered_set<ActionIconType> BENDS_ACTION_TYPES = {
    ActionIconType::STANDARD_BEND,
    ActionIconType::PRE_BEND,
    ActionIconType::GRACE_NOTE_BEND,
    ActionIconType::SLIGHT_BEND
};

static const std::unordered_map<String, String> FRET_DIAGRAMS_MIGRATION_MAP = {
    { u"X[3-O][2-O]O[1-O]O", u"C" },
    { u"X-[5-O][5-O][4-O]-;B3[1-5]", u"Cm" },
    { u"X[3-O][2-O][3-O][1-O]O", u"C7" },

    { u"XXO[2-O][3-O][2-O]", u"D" },
    { u"XXO[2-O][3-O][1-O]", u"Dm" },
    { u"XXO[2-O][1-O][2-O]", u"D7" },

    { u"O[2-O][2-O][1-O]OO", u"E" },
    { u"O[2-O][2-O]OOO", u"Em" },
    { u"O[2-O]O[1-O]OO", u"E7" },

    { u"-[3-O][3-O][2-O]--;B1[0-5]", u"F" },
    { u"-[3-O][3-O]---;B1[0-5]", u"Fm" },
    { u"-[3-O]-[2-O]--;B1[0-5]", u"F7" },

    { u"[3-O][2-O]OOO[3-O]", u"G" },
    { u"-[5-O][5-O]---;B3[0-5]", u"Gm" },
    { u"[3-O][2-O]OOO[1-O]", u"G7" },

    { u"XO[2-O][2-O][2-O]O", u"A" },
    { u"XO[2-O][2-O][1-O]O", u"Am" },
    { u"XO[2-O]O[2-O]O", u"A7" },

    { u"X-[4-O][4-O][4-O]-;B2[1-5]", u"B" },
    { u"X-[4-O][4-O][3-O]-;B2[1-5]", u"Bm" },
    { u"X[2-O][1-O][2-O]O[2-O]", u"B7" }
};

void PaletteCompat::migrateOldPaletteItemIfNeeded(ElementPtr& element, Score* paletteScore)
{
    EngravingItem* item = element.get();

    if (item->isArticulation()) {
        const std::set<SymId>& ornamentIds = compat::CompatUtils::ORNAMENT_IDS;
        bool isOldOrnament = ornamentIds.find(toArticulation(item)->symId()) != ornamentIds.end();

        if (!isOldOrnament) {
            return;
        }

        Articulation* oldOrnament = toArticulation(item);
        Ornament* newOrnament = Factory::createOrnament((ChordRest*)(paletteScore->dummy()->chord()));
        newOrnament->setSymId(oldOrnament->symId());
        element.reset(newOrnament);
        return;
    }

    if (item->isStaffText() && toStaffText(item)->textStyleType() == TextStyleType::EXPRESSION) {
        StaffText* oldExpression = toStaffText(item);
        Expression* newExpression = Factory::createExpression(paletteScore->dummy()->segment());
        if (oldExpression->xmlText() == "Expression") {
            newExpression->setXmlText("expression");
        } else {
            newExpression->setXmlText(oldExpression->xmlText());
        }
        element.reset(newExpression);
        return;
    }

    if (item->isPedal()) {
        Pedal* newPedal = Factory::createPedal(paletteScore->dummy());
        Pedal* oldPedal = toPedal(item);

        newPedal->setLen(oldPedal->frontSegment()->pos2().x());
        newPedal->setLineVisible(oldPedal->lineVisible());
        newPedal->setBeginHookType(oldPedal->beginHookType());
        newPedal->setEndHookType(oldPedal->endHookType());

        newPedal->setBeginText(newPedal->propertyDefault(Pid::BEGIN_TEXT).value<String>());
        newPedal->setContinueText(newPedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>());
        newPedal->setEndText(newPedal->propertyDefault(Pid::END_TEXT).value<String>());

        element.reset(newPedal);
        return;
    }

    if (item->isFretDiagram()) {
        FretDiagram* oldFretDiagram = toFretDiagram(item);
        String oldFretDiagramPattern = FretDiagram::patternFromDiagram(oldFretDiagram);
        std::vector<String> oldFretDiagramPatternHarmonies = FretDiagram::patternHarmonies(oldFretDiagramPattern);

        String harmonyName = muse::value(FRET_DIAGRAMS_MIGRATION_MAP, oldFretDiagramPattern);
        if (harmonyName.empty() || muse::contains(oldFretDiagramPatternHarmonies, harmonyName)) {
            return;
        }

        FretDiagram* newFretDiagram = Factory::createFretDiagram(paletteScore->dummy()->segment());

        if (harmonyName.empty()) {
            newFretDiagram->clear();
        } else {
            newFretDiagram->setHarmony(harmonyName);
            newFretDiagram->updateDiagram(harmonyName);
        }

        element.reset(newFretDiagram);
        return;
    }
}

void PaletteCompat::addNewItemsIfNeeded(Palette& palette, Score* paletteScore)
{
    if (palette.type() == Palette::Type::Guitar) {
        addNewGuitarItems(palette, paletteScore);
        return;
    }

    if (palette.type() == Palette::Type::Line) {
        addNewLineItems(palette);
        return;
    }

    if (palette.type() == Palette::Type::FretboardDiagram) {
        addNewFretboardDiagramItems(palette, paletteScore);
        return;
    }

    if (palette.type() == Palette::Type::Repeat) {
        addNewRepeatItems(palette, paletteScore);
        return;
    }
}

void PaletteCompat::removeOldItemsIfNeeded(Palette& palette)
{
    if (palette.type() == Palette::Type::Articulation
        || palette.type() == Palette::Type::Guitar) {
        removeOldItems(palette);
    }
}

void PaletteCompat::addNewGuitarItems(Palette& guitarPalette, Score* paletteScore)
{
    bool containsCapo = false;
    bool containsStringTunings = false;
    bool containsGuitarBends = false;
    bool containsFFrame = false;

    for (const PaletteCellPtr& cell : guitarPalette.cells()) {
        const ElementPtr element = cell->element;
        if (!element) {
            continue;
        }

        if (element->isCapo()) {
            containsCapo = true;
        } else if (element->isStringTunings()) {
            containsStringTunings = true;
        } else if (element->isActionIcon()) {
            const ActionIcon* icon = toActionIcon(element.get());
            if (muse::contains(BENDS_ACTION_TYPES, icon->actionType())) {
                containsGuitarBends = true;
            }
            if (icon->actionType() == ActionIconType::FFRAME) {
                containsFFrame = true;
            }
        }
    }

    if (!containsCapo) {
        auto capo = std::make_shared<Capo>(paletteScore->dummy()->segment());
        capo->setXmlText(String::fromAscii(QT_TRANSLATE_NOOP("palette", "Capo")));
        int defaultPosition = std::min(7, guitarPalette.cellsCount());
        guitarPalette.insertElement(defaultPosition, capo, QT_TRANSLATE_NOOP("palette", "Capo"))->setElementTranslated(true);
    }

    if (!containsStringTunings) {
        auto stringTunings = std::make_shared<StringTunings>(paletteScore->dummy()->segment());
        stringTunings->setXmlText(u"<sym>guitarString6</sym> - D");
        stringTunings->initTextStyleType(TextStyleType::STAFF);
        int defaultPosition = std::min(8, guitarPalette.cellsCount());
        guitarPalette.insertElement(defaultPosition, stringTunings, QT_TRANSLATE_NOOP("palette", "String tunings"))->setElementTranslated(
            true);
    }

    if (!containsGuitarBends) {
        int defaultPosition = std::min(9, guitarPalette.cellsCount());
        guitarPalette.insertActionIcon(defaultPosition, ActionIconType::STANDARD_BEND, "standard-bend", 1.25);
        guitarPalette.insertActionIcon(defaultPosition, ActionIconType::PRE_BEND, "pre-bend", 1.25);
        guitarPalette.insertActionIcon(defaultPosition, ActionIconType::GRACE_NOTE_BEND, "grace-note-bend", 1.25);
        guitarPalette.insertActionIcon(defaultPosition, ActionIconType::SLIGHT_BEND, "slight-bend", 1.25);
    }

    if (!containsFFrame) {
        guitarPalette.appendActionIcon(ActionIconType::FFRAME, "insert-fretframe");
    }
}

void PaletteCompat::addNewLineItems(Palette& linesPalette)
{
    bool containsNoteAnchoredLine = false;
    for (const PaletteCellPtr& cell : linesPalette.cells()) {
        const ElementPtr element = cell->element;
        if (!element) {
            continue;
        }

        if (element->isActionIcon() && toActionIcon(element.get())->actionType() == ActionIconType::NOTE_ANCHORED_LINE) {
            containsNoteAnchoredLine = true;
        }
    }

    if (!containsNoteAnchoredLine) {
        int defaultPosition = std::min(20, linesPalette.cellsCount());
        linesPalette.insertActionIcon(defaultPosition, ActionIconType::NOTE_ANCHORED_LINE, "add-noteline", 2);
    }
}

void PaletteCompat::addNewFretboardDiagramItems(Palette& fretboardDiagramPalette, engraving::Score* paletteScore)
{
    bool containsBlankItem = false;
    for (const PaletteCellPtr& cell : fretboardDiagramPalette.cells()) {
        const ElementPtr element = cell->element;
        if (!element || !element->isFretDiagram()) {
            continue;
        }

        FretDiagram* fretDiagram = toFretDiagram(element.get());
        if (!fretDiagram->harmony() || fretDiagram->harmony()->harmonyName().empty()) {
            containsBlankItem = true;
            break;
        }
    }

    if (!containsBlankItem) {
        auto fret = Factory::makeFretDiagram(paletteScore->dummy()->segment());
        fret->clear();
        fretboardDiagramPalette.insertElement(0, fret, muse::TranslatableString("palette", "Blank"));
    }
}

void PaletteCompat::addNewRepeatItems(Palette& repeatPalette, engraving::Score* paletteScore)
{
    bool containsToCodaSym = false;
    for (const PaletteCellPtr& cell : repeatPalette.cells()) {
        const ElementPtr element = cell->element;
        if (!element) {
            continue;
        }

        if (element->isMarker() && toMarker(element.get())->markerType() == MarkerType::TOCODASYM) {
            containsToCodaSym = true;
        }
    }

    if (!containsToCodaSym) {
        auto marker = Factory::makeMarker(paletteScore->dummy()->measure());
        marker->setMarkerType(MarkerType::TOCODASYM);
        marker->styleChanged();
        repeatPalette.insertElement(5, marker, TConv::userName(MarkerType::TOCODASYM));
    }
}

void PaletteCompat::removeOldItems(Palette& palette)
{
    std::vector<PaletteCellPtr> cellsToRemove;

    for (const PaletteCellPtr& cell : palette.cells()) {
        const ElementPtr element = cell->element;
        if (!element) {
            continue;
        }

        if (element->isBend()) {
            cellsToRemove.emplace_back(cell);
        }

        if (element->isArticulation() && toArticulation(element.get())->isLaissezVib()) {
            cellsToRemove.emplace_back(cell);
        }
    }

    palette.removeCells(cellsToRemove);
}
