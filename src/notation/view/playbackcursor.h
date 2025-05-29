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

#include <QObject>

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"
#include "draw/types/geometry.h"

#include "notation/inotation.h"
#include <future>

class QColor;

namespace mu::notation {
class PlaybackCursor : public QObject, public muse::Injectable
{
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    PlaybackCursor(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void paint(muse::draw::Painter* painter);

    void setNotation(INotationPtr notation);
    void move(muse::midi::tick_t tick, bool isPlaying = true);

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

    // alex::
    int hit_measure_no();
    Measure* hit_measure();

    void setHitMeasureNo(int m_no);
    void setHitMeasure(Measure* m);

// alex::
    Q_OBJECT
signals:
    void lingeringCursorUpdate(double x, double y, double width, double height) const;
    void lingeringCursorUpdate1() const;

private:
    QColor color() const;
    muse::RectF resolveCursorRectByTick(muse::midi::tick_t tick) const;
    muse::RectF resolveCursorRectByTick(muse::midi::tick_t tick, bool isPlaying = true);
    void processOttava(mu::engraving::Score* score);
    void processOttavaAsync(mu::engraving::Score* score);

    bool m_visible = false;
    muse::RectF m_rect;

    INotationPtr m_notation;

    // alex::
    int m_hit_measure_no = -1;
    Measure* m_hit_measure = nullptr;
    std::set<Note*> curr_measure_trill_notes;
    Note *curr_trill_note = nullptr;
    std::map<Note*, int> tremolo_type_map;
    std::map<Note*, bool> tremolo_half_map;
    Note *curr_trill_note1 = nullptr;
    std::map<const Note*, int> ottava_map;

    std::future<void> m_ottavaProcessFuture;
    std::atomic<bool> m_isOttavaProcessed{ false };

    std::set<ClefType> clefTypes;
    std::unordered_set<ClefType> curr_clefTypes;
    std::map<int, ClefType> stash_clefType;
    std::set<mu::engraving::Key> keySigKeys;
};
}

#endif // MU_NOTATION_PLAYBACKCURSOR_H
