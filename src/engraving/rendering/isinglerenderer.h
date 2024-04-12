/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_ISINGLERENDERER_H
#define MU_ENGRAVING_ISINGLERENDERER_H

#include "modularity/imoduleinterface.h"

#include "draw/painter.h"

namespace mu::engraving {
class EngravingItem;
}

namespace mu::engraving::rendering {
class ISingleRenderer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISingleRenderer)

public:
    virtual ~ISingleRenderer() = default;

    void layoutItem(EngravingItem* item)
    {
        doLayoutItem(item);
    }

    void drawItem(const EngravingItem* item, muse::draw::Painter* p)
    {
        doDrawItem(item, p);
    }

protected:

    virtual void doLayoutItem(EngravingItem* item) = 0;
    virtual void doDrawItem(const EngravingItem* item, muse::draw::Painter* p) = 0;
};
}

#endif // MU_ENGRAVING_ISINGLERENDERER_H
