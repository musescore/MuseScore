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

#include "accessibility/accessibleelement.h"

#include "paintdebugger.h"

#include "log.h"

#define PAINT_DEBUGGER_ENABLED

using namespace mu::engraving;
using namespace mu::accessibility;
using namespace Ms;

static const mu::draw::Color DEBUG_ELTREE_SELECTED_COLOR(164, 0, 0);

void Paint::initDebugger(mu::draw::Painter& painter, const Ms::EngravingItem* element)
{
    auto originalProvider = painter.provider();
    auto debugger = std::make_shared<PaintDebugger>(originalProvider);
    painter.setProvider(debugger, false);

    //! NOTE Here we can configure the debugger depending on the conditions

    // Elements tree
    if (elementsProvider()) {
        if (elementsProvider()->isSelected(element)) {
            debugger->setDebugPenColor(DEBUG_ELTREE_SELECTED_COLOR);
        }
    }

    // Accessible
    AccessibleElement* accessible = element->accessible();
    if (accessible) {
        if (accessible->registred() && accessible->accessibleState(IAccessible::State::Focused)) {
            debugger->setDebugPenColor(draw::Color(255, 0, 0));
        }
    }
    // ----------
}

void Paint::deinitDebugger(mu::draw::Painter& painter)
{
    auto debugger = std::static_pointer_cast<PaintDebugger>(painter.provider());
    debugger->restorePenColor();
    painter.setProvider(debugger->realProvider(), false);
}

void Paint::paintElement(mu::draw::Painter& painter, const Ms::EngravingItem* element)
{
    element->itemDiscovered = false;
    PointF elementPosition(element->pagePos());

#ifdef PAINT_DEBUGGER_ENABLED
    initDebugger(painter, element);
#endif

    painter.translate(elementPosition);
    element->draw(&painter);
    painter.translate(-elementPosition);

#ifdef PAINT_DEBUGGER_ENABLED
    deinitDebugger(painter);
#endif
}

void Paint::paintElements(mu::draw::Painter& painter, const QList<EngravingItem*>& elements)
{
    QList<Ms::EngravingItem*> sortedElements = elements;

    std::sort(sortedElements.begin(), sortedElements.end(), [](Ms::EngravingItem* e1, Ms::EngravingItem* e2) {
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

    for (const EngravingItem* element : sortedElements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElement(painter, element);
    }
}

void Paint::paintPage(mu::draw::Painter& painter, Ms::Page* page, const RectF& rect)
{
    PointF pagePosition(page->pos());
    painter.translate(pagePosition);
    painter.setClipping(true);
    painter.setClipRect(page->bbox());

    QList<EngravingItem*> elements = page->items(rect);
    paintElements(painter, elements);

    paintPageDiagnostic(painter, page);

    painter.translate(-pagePosition);
    painter.setClipping(false);
}

void Paint::paintDiagnostic(mu::draw::Painter& painter)
{
    return;
    std::list<const Ms::EngravingObject*> elements = elementsProvider()->elements();
    for (const Ms::EngravingObject* el : elements) {
        if (!el->isEngravingItem()) {
            continue;
        }

        const Ms::EngravingItem* item = Ms::toEngravingItem(el);
        paintElement(painter, item);
    }
}

void Paint::paintChildren(mu::draw::Painter& painter, const std::list<const Ms::EngravingObject*>& elements,
                          const Ms::EngravingItem* parent)
{
    for (const Ms::EngravingObject* el : elements) {
        if (!el->isEngravingItem()) {
            continue;
        }

        if (el->treeParent() == parent) {
            const Ms::EngravingItem* item = Ms::toEngravingItem(el);
            paintElement(painter, item);

            if (item->isChord()) {
                mu::RectF bbox = item->bbox();
                //if (bbox.isNull()) {
                bbox = mu::RectF(0, 0, 10, 10);
//                }
                painter.setPen(mu::draw::Color(200, 0, 0));
                painter.drawRect(bbox);
            }

            paintChildren(painter, elements, item);
        }
    }
}

void Paint::paintPageDiagnostic(mu::draw::Painter& painter, Ms::Page* page)
{
    std::list<const Ms::EngravingObject*> elements = elementsProvider()->elements();
    paintChildren(painter, elements, page);
}
