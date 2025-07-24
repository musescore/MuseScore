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
    void enableHighlightCursorNote(bool highlight);
    void enableKeyboardPlay(bool enable);
    void move(muse::midi::tick_t tick, bool isPlaying = true);

    bool visible() const;
    void setVisible(bool arg);

    const muse::RectF& rect() const;

    const bool adjust_nm_rect() const;
    const muse::RectF& nm_rect() const;

    int hit_measure_no();
    Measure* hit_measure();

    void setHitMeasureNo(int m_no);
    void setHitMeasure(Measure* m);
    Q_OBJECT
signals:
    void lingeringCursorUpdate(double x, double y, double width, double height) const;
    void lingeringCursorUpdate1() const;

private:
    QColor color() const;
    muse::RectF resolveCursorRectByTick(muse::midi::tick_t tick) const;
    muse::RectF resolveCursorRectByTick1(muse::midi::tick_t tick, bool isPlaying = true);
    void processOttava(mu::engraving::Score* score, bool isPlaying = true);
    void processOttavaAsync(mu::engraving::Score* score);

    void processCursorSpannerRenderStatus(Measure* measure, Fraction tick, bool recover, bool isPlaying);
    void processCursorSpannerRenderStatusAsync(Measure* measure, Fraction tick, bool recover, bool isPlaying);

    void processCursorNoteRenderStatus(Measure* measure, int curr_ticks);
    void processCursorNoteRenderStatusAsync(Measure* measure, int curr_ticks);

    void processCursorNoteRenderRecover(EngravingItem* engravingItem, int curr_ticks);
    void processCursorNoteRenderRecoverAsync(EngravingItem* engravingItem, int curr_ticks);

    bool m_visible = false;
    muse::RectF m_rect;
    bool m_adjust_nm_rect = false;
    int m_nm_no = -1;
    muse::RectF m_nm_rect;

    INotationPtr m_notation;

    int m_hit_measure_no = -1;
    Measure* m_hit_measure = nullptr;
    std::map<const Note*, int> ottava_map;
    std::map<EngravingItem*, EngravingItem*> chordrest_fermata_map;
    std::map<int, std::set<EngravingItem*> > measure_spanner_map;
    std::map<EngravingItem*, std::map<int, int> > spanner_ticks_map;

    std::map<EngravingItem*, Note*> score_trill_map;
    std::map<EngravingItem*, int> score_trill_type_map;
    std::map<EngravingItem*, int> score_trill_st_map;
    std::map<EngravingItem*, int> score_trill_dt_map;
    std::map<EngravingItem*, int> score_trill_tdt_map;
    std::map<EngravingItem*, int> score_trill_tt_map;
    std::map<EngravingItem*, int> score_trill_ot_map;
    std::map<Note*, bool> score_trill_tie_map;
    std::map<EngravingItem*, Note*> score_trill_map1;
    std::map<EngravingItem*, int> score_trill_type_map1;
    std::map<EngravingItem*, int> score_trill_st_map1;
    std::map<EngravingItem*, int> score_trill_dt_map1;
    std::map<EngravingItem*, int> score_trill_tdt_map1;
    std::map<EngravingItem*, int> score_trill_tt_map1;
    std::map<EngravingItem*, int> score_trill_ot_map1;
    std::map<Note*, bool> score_trill_tie_map1;

    std::map<EngravingItem*, std::vector<Note*> > score_arpeggio_map;
    std::map<EngravingItem*, int> score_arpeggio_st_map;
    std::map<EngravingItem*, int> score_arpeggio_dt_map;
    std::map<EngravingItem*, int> score_arpeggio_ot_map;

    std::map<EngravingItem*, Note*> score_glissando_startnote_map;
    std::map<EngravingItem*, int> score_glissando_st_map;
    std::map<EngravingItem*, int> score_glissando_dt_map;
    std::map<EngravingItem*, int> score_glissando_ot_map;
    std::map<EngravingItem*, std::vector<Note*> > score_glissando_endnotes_map;

    std::future<void> m_ottavaProcessFuture;
    std::atomic<bool> m_isOttavaProcessed{ false };

    std::future<void> m_cursorSpannerRenderStatusProcessFuture;
    std::future<void> m_cursorNoteRenderStatusProcessFuture;

    std::future<void> m_cursorNoteRenderRecoverFuture;

    std::map<int, std::set<uint> > clefKeySigsKeysMap;

    std::map<int, int> mnRestSTicksMap;
    std::map<int, int> mnRestETicksMap;
    std::map<int, muse::RectF> mnRestRectFMap;
    std::map<int, double> mnCursorWidthMap;
    std::map<int, double> mnSpatiumMap;

    int curr_seg_ticks = 0;
    int preProcessScore = false;
    bool highlightCursorNote = true;
    bool pianoKeyboardPlaybackEnable = true;
};
}

#endif // MU_NOTATION_PLAYBACKCURSOR_H
