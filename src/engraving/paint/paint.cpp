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
#include "paint.h"

#include "infrastructure/draw/painter.h"
#include "libmscore/engravingitem.h"
#include "libmscore/page.h"
#include "libmscore/score.h"

#include "debugpaint.h"

#include "log.h"
#include "config.h"

using namespace mu::engraving;

void Paint::paintElement(mu::draw::Painter& painter, const mu::engraving::EngravingItem* element)
{
    if (element->skipDraw()) {
        return;
    }
    element->itemDiscovered = false;
    PointF elementPosition(element->pagePos());

    painter.translate(elementPosition);
    element->draw(&painter);
    painter.translate(-elementPosition);
}

void Paint::paintElements(mu::draw::Painter& painter, const std::vector<EngravingItem*>& elements, bool isPrinting)
{
    std::vector<mu::engraving::EngravingItem*> sortedElements(elements.begin(), elements.end());

    std::sort(sortedElements.begin(), sortedElements.end(), mu::engraving::elementLessThan);

    for (const EngravingItem* element : sortedElements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElement(painter, element);
    }

#ifdef ENGRAVING_PAINT_DEBUGGER_ENABLED
    if (!isPrinting) {
        DebugPaint::paintElementsDebug(painter, sortedElements);
    }
#else
    UNUSED(isPrinting);
#endif
}
