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
#ifndef MU_ENGRAVING_DEBUGPAINT_STABLE_H
#define MU_ENGRAVING_DEBUGPAINT_STABLE_H

#include "draw/painter.h"

#include "modularity/ioc.h"
#include "diagnostics/iengravingelementsprovider.h"
#include "iengravingconfiguration.h"

namespace mu::engraving {
class EngravingItem;
class Page;
}

namespace mu::engraving::rendering::stable {
class PaintDebugger;
class DebugPaint
{
    INJECT_STATIC(IEngravingConfiguration, configuration)
    INJECT_STATIC(diagnostics::IEngravingElementsProvider, elementsProvider)

public:
    static void paintElementDebug(mu::draw::Painter& painter, const EngravingItem* item);
    static void paintPageDebug(mu::draw::Painter& painter, const Page* page, const std::vector<EngravingItem*>& items);

    static void paintPageTree(mu::draw::Painter& painter, const Page* page);
    static void paintTreeElement(mu::draw::Painter& painter, const EngravingItem* item);
};
}

#endif // MU_ENGRAVING_DEBUGPAINT_STABLE_H
