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

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "notation/inotationconfiguration.h"
#include "draw/types/geometry.h"
#include "midi/miditypes.h"

#include "notation/inotation_fwd.h"

class QColor;

namespace mu::notation {
class PlaybackCursor : public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<INotationConfiguration> configuration;

public:
    PlaybackCursor(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void paint(muse::draw::Painter* painter);

    void setNotation(INotationPtr notation);
    void move(muse::midi::tick_t tick);

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

private:
    QColor color() const;

    muse::RectF resolveCursorRectByTick(int tick) const;

    struct PlaybackCursorCache {
        const engraving::System* system = nullptr;
        const engraving::Measure* measure = nullptr;
        const engraving::Segment* segment = nullptr;

        engraving::Fraction segmentStartTick;
        engraving::Fraction segmentEndTick;

        double segmentStartX = 0.0;
        double segmentEndX = 0.0;
        double systemBottomY = 0.0;

        void clear()
        {
            *this = PlaybackCursorCache();
        }
    } mutable m_cache;

    bool m_visible = false;
    muse::RectF m_rect;

    mu::notation::INotationPtr m_notation;
};
}
