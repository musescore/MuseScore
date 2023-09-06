/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_LAYOUTINDEPENT_DEV_H
#define MU_ENGRAVING_LAYOUTINDEPENT_DEV_H

#include "dom/engravingitem.h"

#include "dom/accidental.h"
#include "dom/actionicon.h"
#include "dom/articulation.h"

#include "dom/ornament.h"

#include "layoutcontext.h"

namespace mu::engraving::rendering::dev {
class LayoutIndependent
{
public:
    LayoutIndependent();

    static bool isItemIndepended(const EngravingItem* item);
    static bool layoutItem(EngravingItem* item, const LayoutContext& ctx);
    static bool layoutItem(const EngravingItem* item, EngravingItem::LayoutData* ldata, const LayoutContext& ctx);

private:
    static bool dolayoutItem(const EngravingItem* item, EngravingItem::LayoutData* ldata, const LayoutContext& ctx, bool perform);

    static void layout(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layout(const ActionIcon* item, ActionIcon::LayoutData* ldata);
    static void layout(const Articulation* item, Articulation::LayoutData* ldata);

    static void layout(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf);
};
}

#endif // MU_ENGRAVING_LAYOUTINDEPENT_DEV_H
