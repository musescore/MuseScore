/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "modularity/ioc.h"

#include "notation/inotation.h"

#include "draw/types/geometry.h"

namespace mu::notation {
class LoopMarker : public muse::Injectable
{
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    LoopMarker(LoopBoundaryType type, const muse::modularity::ContextPtr& iocCtx);

    void setNotation(INotationPtr notation);
    void setVisible(bool visible);

    void move(muse::midi::tick_t tick);

    void paint(muse::draw::Painter* painter);

private:
    muse::RectF resolveMarkerRectByTick(muse::midi::tick_t tick) const;

    LoopBoundaryType m_type = LoopBoundaryType::Unknown;
    muse::RectF m_rect;
    bool m_visible = false;
    INotationPtr m_notation;
};
}

#endif // MU_NOTATION_LOOPMARKER_H
