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

#include "libmscore/element.h"

#include "paintdebugger.h"

using namespace mu::engraving;
using namespace Ms;

void Paint::paintElement(mu::draw::Painter& painter, const Ms::Element* element)
{
    element->itemDiscovered = false;
    PointF elementPosition(element->pagePos());

    auto originalProvider = painter.provider();
    auto debugger = std::make_shared<PaintDebugger>(originalProvider);
    painter.setProvider(debugger, false);

    debugger->setDebugPenColor(draw::Color(255, 0, 0));

    painter.translate(elementPosition);
    element->draw(&painter);
    painter.translate(-elementPosition);

    debugger->restorePenColor();
}

void Paint::paintElements(mu::draw::Painter& painter, const QList<Element*>& elements)
{
    QList<Ms::Element*> sortedElements = elements;

    std::sort(sortedElements.begin(), sortedElements.end(), [](Ms::Element* e1, Ms::Element* e2) {
        if (e1->z() == e2->z()) {
            if (e1->selected()) {
                return false;
            } else if (e1->visible()) {
                return false;
            } else if (e2->selected()) {
                return true;
            } else if (e1->visible()) {
                return true;
            }

            return e1->track() > e2->track();
        }

        return e1->z() < e2->z();
    });

    for (const Element* element : sortedElements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElement(painter, element);
    }
}
