/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "draw/types/geometry.h"

#include "engraving/dom/score.h"

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotation_fwd.h"

namespace mu::notation {
class LoopMarker : public muse::Contextable
{
    muse::GlobalInject<INotationConfiguration> configuration;

public:
    LoopMarker(engraving::LoopBoundaryType type, const muse::modularity::ContextPtr& iocCtx);

    void setNotation(INotationPtr notation);
    void setVisible(bool visible);

    void updatePosition(engraving::Fraction tick);

    void paint(muse::draw::Painter* painter);

private:
    muse::RectF resolveMarkerRectByTick(engraving::Fraction tick) const;

    engraving::LoopBoundaryType m_type = engraving::LoopBoundaryType::Unknown;
    muse::RectF m_rect;
    bool m_visible = false;
    INotationPtr m_notation;
};
}
