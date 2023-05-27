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

#include "libmscore/articulation.h"
#include "libmscore/chordrest.h"
#include "libmscore/engravingitem.h"
#include "libmscore/expression.h"
#include "libmscore/factory.h"
#include "libmscore/ornament.h"
#include "libmscore/score.h"
#include "libmscore/stafftext.h"
#include "engraving/types/symid.h"

using namespace mu::palette;
using namespace mu::engraving;

void PaletteCompat::migrateOldPaletteItemIfNeeded(ElementPtr& element, Score* paletteScore)
{
    if (paletteScore->mscVersion() >= 410) {
        return;
    }

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
