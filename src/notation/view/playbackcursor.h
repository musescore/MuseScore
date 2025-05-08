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
    void move(muse::midi::tick_t tick);

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

    // alex::
    std::vector<EngravingItem*>& hit_elements();
    int hit_measure_no();
    int seg_note_duration_tree();
    std::vector<std::vector<EngravingItem*>> seg_records();
    std::vector<int> staffindex_curr_at_segindex_records();

    void setHitElements(std::vector<EngravingItem*>& el);
    void setHitMeasureNo(int m_no);
    void setSegNoteDurationTree(int m_tree);
    void pushSegRecords(std::vector<EngravingItem*> item);
    void pushStaffindexCurrAtSegindexRecords(int m_seg_index);
    void updateStaffindexCurrAtSegindex(int m_staffindex, int m_seg_atindex);

    void highlightAt(int seg_index, int seg_track_index, bool is_highlight);

    void clearSegRecords();

// alex::
    Q_OBJECT
signals:
    void lingeringCursorUpdate(double x, double y, double width, double height) const;
    void lingeringCursorUpdate1() const;

private:
    QColor color() const;
    muse::RectF resolveCursorRectByTick(muse::midi::tick_t tick) const;
    muse::RectF resolveCursorRectByTick1(muse::midi::tick_t tick);

    bool m_visible = false;
    muse::RectF m_rect;

    INotationPtr m_notation;

    // alex::
    std::vector<EngravingItem*> m_hit_el;
    std::vector<std::vector<EngravingItem*>> m_seg_records;
    int m_hit_measure_no = -1;
    int m_seg_note_duration_tree = 0;
    std::vector<int> m_staffindex_curr_at_segindex_records;
};
}

#endif // MU_NOTATION_PLAYBACKCURSOR_H
