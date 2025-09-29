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
#pragma once

#include "modularity/imoduleinterface.h"

#include "draw/painter.h"
#include "paintoptions.h"

namespace mu::engraving {
class EngravingItem;
}

namespace mu::engraving::rendering {
class ISingleRenderer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISingleRenderer)

public:
    virtual ~ISingleRenderer() = default;

    virtual void layoutItem(EngravingItem* item) = 0;
    virtual void drawItem(const EngravingItem* item, muse::draw::Painter* p, const PaintOptions& opt) = 0;
};
}
