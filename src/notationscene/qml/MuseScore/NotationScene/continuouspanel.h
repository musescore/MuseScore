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

#pragma once

#include <vector>

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/rendering/iscorerenderer.h"

#include "draw/types/geometry.h"
#include "notation/inotation.h"

namespace mu::engraving {
class Score;
class Text;
class Clef;
class KeySig;
class TimeSig;
class BarLine;
class Segment;
}

namespace muse::draw {
class Painter;
}

namespace mu::notation {
class ContinuousPanel : public muse::Injectable
{
    muse::Inject<INotationConfiguration> notationConfiguration = { this };
    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };
    muse::Inject<engraving::rendering::IScoreRenderer> scoreRender = { this };

public:
    ContinuousPanel(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}
    ~ContinuousPanel();

    void setNotation(INotationPtr notation);

    struct NotationViewContext {
        qreal xOffset = 0.;
        qreal yOffset = 0.;
        qreal scaling = 0.;
        std::function<muse::PointF(const muse::PointF&)> fromLogical;
    };

    qreal width() const;
    void paint(muse::draw::Painter& painter, const NotationViewContext& ctx, const engraving::rendering::PaintOptions& opt);

private:
    void clearCache();
    void ensureCacheSize(size_t staffCount);

    INotationPtr m_notation;
    qreal m_width = 0;
    muse::RectF m_rect;

    mu::engraving::Text* m_cachedMeasureNumberText = nullptr;
    std::vector<mu::engraving::Text*> m_cachedStaffNameTexts;
    std::vector<mu::engraving::Clef*> m_cachedClefs;
    std::vector<mu::engraving::KeySig*> m_cachedKeySigs;
    std::vector<mu::engraving::TimeSig*> m_cachedTimeSigs;
    std::vector<mu::engraving::BarLine*> m_cachedBarLines;
};
}
