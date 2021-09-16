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
#include "debugpaint.h"

#include "paintdebugger.h"

#include "accessibility/accessibleitem.h"
#include "libmscore/page.h"
#include "libmscore/score.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::accessibility;
using namespace Ms;

static const mu::draw::Color DEBUG_ELTREE_SELECTED_COLOR(164, 0, 0);

void DebugPaint::paintPageDiagnostic(mu::draw::Painter& painter, Ms::Page* page)
{
    TRACEFUNC;

    // Setup debug provider
    auto originalProvider = painter.provider();
    std::shared_ptr<PaintDebugger> debugger = std::make_shared<PaintDebugger>(originalProvider);
    painter.setProvider(debugger, false);

    // Get children
    const mu::diagnostics::EngravingObjectList& elements = elementsProvider()->elements();

    std::list<const Ms::EngravingItem*> children;
    for (const Ms::EngravingObject* el : elements) {
        if (!el->isEngravingItem()) {
            continue;
        }

        const Ms::EngravingObject* p = el->parent(true);
        while (p) {
            if (p == page) {
                children.push_back(Ms::toEngravingItem(el));
                break;
            }
            p = p->parent(true);
        }
    }

    // Draw children
    paintItems(painter, children, debugger);

    // Restore provider
    debugger->restorePenColor();
    painter.setProvider(debugger->realProvider(), false);
}

void DebugPaint::paintItems(mu::draw::Painter& painter, const std::list<const Ms::EngravingItem*>& items,
                            std::shared_ptr<PaintDebugger>& debugger)
{
    TRACEFUNC;
    for (const Ms::EngravingItem* item : items) {
        //! NOTE Here we can configure the debugger depending on the conditions

        // Elements tree
        bool isDiagnosticSelected = elementsProvider()->isSelected(item);
        if (isDiagnosticSelected) {
            // Overriding pen
            debugger->setDebugPenColor(DEBUG_ELTREE_SELECTED_COLOR);
        }

        // Accessible
//        AccessibleItem* accessible = item->accessible();
//        if (accessible) {
//            if (accessible->registred() && accessible->accessibleState(IAccessible::State::Focused)) {
//                debugger->setDebugPenColor(draw::Color(255, 0, 0));
//            }
//        }
        // ----------

        PointF pos(item->pagePos());
        painter.translate(pos);
        item->draw(&painter);

        if (isDiagnosticSelected) {
            static draw::Pen borderPen(DEBUG_ELTREE_SELECTED_COLOR, 4);

            // Draw bbox
            painter.setPen(borderPen);
            painter.setBrush(draw::BrushStyle::NoBrush);
            painter.drawRect(item->bbox());
        }

        painter.translate(-pos);

        debugger->restorePenColor();
    }
}
