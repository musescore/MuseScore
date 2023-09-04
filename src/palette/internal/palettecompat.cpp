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

#include <memory>

#include "palettecompat.h"

#include "engraving/rw/compat/compatutils.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/score.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/capo.h"
#include "engraving/types/symid.h"

#include "palette.h"
#include "palettecell.h"

using namespace mu::palette;
using namespace mu::engraving;

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
    }
}

void PaletteCompat::addNewItemsIfNeeded(Palette& palette, Score* paletteScore)
{
    if (palette.type() == Palette::Type::Guitar) {
        addNewGuitarItems(palette, paletteScore);
    }
}

void PaletteCompat::addNewGuitarItems(Palette& guitarPalette, Score* paletteScore)
{
    bool containsCapo = false;

    for (const PaletteCellPtr& cell : guitarPalette.cells()) {
        if (cell->element && cell->element->isCapo()) {
            containsCapo = true;
            break;
        }
    }

    if (!containsCapo) {
        auto capo = std::make_shared<Capo>(paletteScore->dummy()->segment());
        capo->setXmlText(String::fromAscii(QT_TRANSLATE_NOOP("palette", "Capo")));
        int defaultPosition = std::min(7, guitarPalette.cellsCount());
        guitarPalette.insertElement(defaultPosition, capo, QT_TRANSLATE_NOOP("palette", "Capo"))->setElementTranslated(true);
    }
}
