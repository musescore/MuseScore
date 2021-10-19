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

#ifndef MU_NOTATION_LOOPMARKER_H
#define MU_NOTATION_LOOPMARKER_H

#include "notation/inotationconfiguration.h"
#include "notation/inotationstyle.h"
#include "modularity/ioc.h"

#include "engraving/infrastructure/draw/geometry.h"

namespace mu::notation {
class LoopMarker
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    LoopMarker(LoopBoundaryType type);

    void setRect(const RectF& rect);
    void setVisible(bool visible);
    void setStyle(INotationStylePtr style);

    void paint(draw::Painter* painter);

private:
    LoopBoundaryType m_type = LoopBoundaryType::Unknown;
    RectF m_rect;
    bool m_visible = false;
    INotationStylePtr m_style;
};
}

#endif // MU_NOTATION_LOOPMARKER_H
