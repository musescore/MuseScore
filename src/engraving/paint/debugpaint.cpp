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

void DebugPaint::paintElementDebug(mu::draw::Painter& painter, const Ms::EngravingItem* item, std::shared_ptr<PaintDebugger>& debugger)
{
    // Elements tree
    bool isDiagnosticSelected = elementsProvider()->isSelected(item);
    if (isDiagnosticSelected) {
        // Overriding pen
        debugger->setDebugPenColor(DEBUG_ELTREE_SELECTED_COLOR);
    }

    PointF pos(item->pagePos());
    painter.translate(pos);

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

void DebugPaint::paintElementsDebug(mu::draw::Painter& painter, const QList<Ms::EngravingItem*>& elements)
{
    // Setup debug provider
    auto originalProvider = painter.provider();
    std::shared_ptr<PaintDebugger> debugger = std::make_shared<PaintDebugger>(originalProvider);
    painter.setProvider(debugger, false);

    for (const EngravingItem* element : elements) {
        if (!element->isInteractionAvailable()) {
            continue;
        }

        paintElementDebug(painter, element, debugger);
    }

    // Restore provider
    debugger->restorePenColor();
    painter.setProvider(debugger->realProvider(), false);
}
