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
#ifndef MU_ENGRAVING_DEBUGPAINT_H
#define MU_ENGRAVING_DEBUGPAINT_H

#include "draw/painter.h"

#include "modularity/ioc.h"
#include "diagnostics/iengravingelementsprovider.h"
#include "iengravingconfiguration.h"

namespace mu::engraving {
class EngravingItem;
class Page;

class PaintDebugger;
class DebugPaint
{
    INJECT_STATIC(IEngravingConfiguration, engravingConfiguration)
    INJECT_STATIC(diagnostics::IEngravingElementsProvider, elementsProvider)

public:
    static void paintElementDebug(mu::draw::Painter& painter, const EngravingItem* element, std::shared_ptr<PaintDebugger>& debugger);
    static void paintElementsDebug(mu::draw::Painter& painter, const std::vector<EngravingItem*>& elements);

    static void paintPageDebug(mu::draw::Painter& painter, const Page* page);
};
}

#endif // MU_ENGRAVING_DEBUGPAINT_H
