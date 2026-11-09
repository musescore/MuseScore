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
#ifndef SYSTEM_CURSOR_H
#define SYSTEM_CURSOR_H

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "draw/types/geometry.h"
#include "midi/miditypes.h"
#include "notation/inotationconfiguration.h"

#include "notation/inotation_fwd.h"

class QColor;

namespace mu::notation {
class SystemCursor : public muse::Contextable, public muse::async::Asyncable
{
protected:
    explicit SystemCursor(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx)
    {}

public:
    virtual ~SystemCursor() = default;

    void paint(muse::draw::Painter* painter);

    void setNotation(INotationPtr notation);
    void move(muse::midi::tick_t newTick);

    //! Re-resolves the rect for the current tick (e.g. after a relayout)
    void refresh();

    muse::midi::tick_t tick() const;

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

protected:
    muse::GlobalInject<INotationConfiguration> configuration;

    virtual QColor color() const = 0;
    virtual bool isShownInNormalMode() const = 0;

    muse::RectF resolveCursorRectByTick(int newTick) const;

    struct PlaybackCursorCache
    {
        const engraving::System* system = nullptr;
        const engraving::Measure* measure = nullptr;
        const engraving::Segment* segment = nullptr;

        engraving::Fraction segmentStartTick;
        engraving::Fraction segmentEndTick;

        double segmentStartX = 0.0;
        double segmentEndX = 0.0;
        double systemBottomY = 0.0;

        void clear() { *this = PlaybackCursorCache(); }
    } mutable m_cache;

private:
    bool m_visible = false;
    muse::midi::tick_t m_tick = 0;
    muse::RectF m_rect;

    INotationPtr m_notation;
};
} // namespace mu::notation
#endif
