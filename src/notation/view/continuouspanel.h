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

#ifndef MU_NOTATION_CONTINUOUSPANEL_H
#define MU_NOTATION_CONTINUOUSPANEL_H

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/rendering/isinglerenderer.h"

#include "draw/types/geometry.h"
#include "notation/inotation.h"

namespace mu::engraving {
class Score;
}

namespace mu::draw {
class Painter;
}

namespace mu::notation {
class ContinuousPanel
{
    INJECT(INotationConfiguration, notationConfiguration)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)
    INJECT(engraving::rendering::ISingleRenderer, engravingRender)

public:
    void setNotation(INotationPtr notation);

    struct NotationViewContext {
        qreal xOffset = 0.;
        qreal yOffset = 0.;
        qreal scaling = 0.;
        std::function<PointF(const PointF&)> fromLogical;
    };

    void paint(draw::Painter& painter, const NotationViewContext& ctx);

private:
    qreal styleMM(const mu::engraving::Sid styleId) const;
    const mu::engraving::Score* score() const;

    INotationPtr m_notation;
    qreal m_width = 0;
    RectF m_rect;
};
}

#endif // MU_NOTATION_CONTINUOUSPANEL_H
