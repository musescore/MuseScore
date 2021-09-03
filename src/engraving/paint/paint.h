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
#ifndef MU_ENGRAVING_PAINT_H
#define MU_ENGRAVING_PAINT_H

#include <QList>
#include "infrastructure/draw/painter.h"

#include "modularity/ioc.h"
#include "diagnostics/iengravingelementsprovider.h"

namespace Ms {
class EngravingItem;
class Page;
class Score;
}

namespace mu::engraving {
class Paint
{
    INJECT_STATIC(engraving, diagnostics::IEngravingElementsProvider, elementsProvider)

public:

    static void paintElement(mu::draw::Painter& painter, const Ms::EngravingItem* element);
    static void paintElements(mu::draw::Painter& painter, const QList<Ms::EngravingItem*>& elements);

    static void paintPage(mu::draw::Painter& painter, Ms::Page* page, const RectF& rect);

    static void paintDiagnostic(mu::draw::Painter& painter);
    static void paintPageDiagnostic(mu::draw::Painter& painter, Ms::Page* page);

private:

    static void initDebugger(mu::draw::Painter& painter, const Ms::EngravingItem* element);
    static void deinitDebugger(mu::draw::Painter& painter);

    static void paintChildren(mu::draw::Painter& painter, const std::list<const Ms::EngravingObject*>& els,
                              const Ms::EngravingItem* parent);
};
}

#endif // MU_ENGRAVING_PAINT_H
