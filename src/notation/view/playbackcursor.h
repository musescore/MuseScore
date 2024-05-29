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
#ifndef MU_NOTATION_PLAYBACKCURSOR_H
#define MU_NOTATION_PLAYBACKCURSOR_H

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "draw/types/geometry.h"

#include "notation/inotation.h"

class QColor;

namespace mu::notation {
class PlaybackCursor : public muse::Injectable
{
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    PlaybackCursor(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void paint(muse::draw::Painter* painter);

    void setNotation(INotationPtr notation);
    void move(muse::midi::tick_t tick);

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

private:
    QColor color() const;
    muse::RectF resolveCursorRectByTick(muse::midi::tick_t tick) const;

    bool m_visible = false;
    muse::RectF m_rect;

    INotationPtr m_notation;
};
}

#endif // MU_NOTATION_PLAYBACKCURSOR_H
