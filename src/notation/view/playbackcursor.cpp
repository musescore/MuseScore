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
#include "playbackcursor.h"

#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace muse;

void PlaybackCursor::paint(muse::draw::Painter* painter)
{
    if (!m_visible) {
        return;
    }

    painter->fillRect(m_rect, color());
}

void PlaybackCursor::setNotation(INotationPtr notation)
{
    m_notation = notation;
}

void PlaybackCursor::move(muse::midi::tick_t tick)
{
    // LOGALEX();
    // m_rect = resolveCursorRectByTick(tick);
    m_rect = resolveCursorRectByTick1(tick);
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
muse::RectF PlaybackCursor::resolveCursorRectByTick(muse::midi::tick_t _tick) const
{
    if (!m_notation) {
        return RectF();
    }

    mu::engraving::Score* score = m_notation->elements()->msScore();

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return RectF();
    }

    mu::engraving::System* system = measure->system();
    if (!system) {
        return RectF();
    }

    qreal x = 0.0;
    mu::engraving::Segment* s = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0.0;
        Fraction t2;

        mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(mu::engraving::SegmentType::ChordRest);
        }

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            mu::engraving::Segment* seg = measure->findSegment(mu::engraving::SegmentType::EndBarLine, measure->tick() + measure->ticks());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width(); // safety, should not happen
            }
        }

        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (!s) {
        return RectF();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score->style().spatium();

    double w  = 8;
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }

    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    return RectF(x, y, w, h);
}

muse::RectF PlaybackCursor::resolveCursorRectByTick1(muse::midi::tick_t _tick)
{
    if (!m_notation) {
        return RectF();
    }

    mu::engraving::Score* score = m_notation->elements()->msScore();

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return RectF();
    }

    // muse::RectF measureRect = measure->pageBoundingRect();
    // int measureNo = measure->no();

    mu::engraving::System* system = measure->system();
    if (!system) {
        return RectF();
    }

    // size_t seg_note_tree1 = seg_note_duration_tree();
    std::vector<EngravingItem*> oldHitElements = hit_elements();
    size_t len1 = oldHitElements.size();

    // if (seg_note_tree1 != 0 && hit_measure_no() != measureNo) {
    //     std::vector<size_t> staffIndexAtSegRecords = staffindex_curr_at_segindex_records();
    //     for (size_t i = 0; i < len1; i++) {
    //         int atSegIndex = staffIndexAtSegRecords[i];
    //         if ((seg_note_tree1 & (1 << i)) > 0) {
    //             highlightAt(atSegIndex, i, false);
    //         }
    //     }
    //     m_staffindex_curr_at_segindex_records.clear();
    // } 
  
    // m_seg_records.clear(); 

    qreal x = 0.0;
    mu::engraving::Segment* s = nullptr;
    mu::engraving::Segment* s1 = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        s1 = s;
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0.0;
        Fraction t2;

        // std::vector<EngravingItem*> engravingItemList = s->elist();
        // pushSegRecords(engravingItemList);

        mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(mu::engraving::SegmentType::ChordRest);
        }

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            mu::engraving::Segment* seg = measure->findSegment(mu::engraving::SegmentType::EndBarLine, measure->tick() + measure->ticks());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width(); // safety, should not happen
            }
        }

        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    // alex:: lingering cursor
    std::vector<EngravingItem*> engravingItemList = s1->elist();
    size_t len = engravingItemList.size();
    
    // size_t seg_note_tree = 0;
    for (size_t i = 0; i < len; i++) {
        EngravingItem* engravingItem = engravingItemList[i];
        
        if (engravingItem == nullptr) {
            continue;
        }

        // seg_note_tree |= (1 << i);

        engravingItem->setColor(muse::draw::Color::RED);
    }

    if (oldHitElements != engravingItemList) {
        for (size_t i = 0; i < len1; i++) {
            EngravingItem* engravingItem = oldHitElements[i];
            if (engravingItem == nullptr) {
                continue;
            }
            engravingItem->setColor(muse::draw::Color::BLACK);
        }
        setHitElements(engravingItemList);

        // if (seg_note_tree1 == 0 || hit_measure_no() != measureNo) {
        //     setSegNoteDurationTree(seg_note_tree);
        //     for (size_t i = 0; i < len1; i++) {
        //         pushStaffindexCurrAtSegindexRecords(0);
        //     }
        //     setHitMeasureNo(measureNo);
        // } else {
        //     size_t xorResult = seg_note_tree1 ^ seg_note_tree;

        //     std::vector<std::vector<EngravingItem*>> segRecords = seg_records();
        //     size_t segRecordsLen = segRecords.size();

        //     std::vector<size_t> staffIndexAtSegRecords = staffindex_curr_at_segindex_records();
            
        //     for (size_t i = 0; i < len1; i++) {
        //         if ((xorResult & (1 << i)) > 0) {
        //             size_t atSegIndex = staffIndexAtSegRecords[i];
        //             bool highlight_flag = false;
        //             size_t _atSegIndex = atSegIndex;
        //             if ((seg_note_tree1 & (1 << i)) > 0) {
        //                 highlight_flag = true;
        //             } else {
        //                 atSegIndex = segRecordsLen - 1;
        //                 updateStaffindexCurrAtSegindex(i, atSegIndex);
        //             }
                    
        //             highlightAt(_atSegIndex, i, highlight_flag);
        //         }
        //     }

        //     setSegNoteDurationTree(seg_note_tree1 | seg_note_tree);
        // }
        
        // if (measureNo < 2) {
        //     emit lingeringCursorUpdate(0.0, measureRect.y(), measureRect.width(), measureRect.height());
        // } else {
        //     emit lingeringCursorUpdate(measureRect.x(), measureRect.y(), measureRect.width(), measureRect.height());
        // }
        emit lingeringCursorUpdate1();
    }

    if (!s) {
        return RectF();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score->style().spatium();

    double w  = 8;
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }

    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    return RectF(x, y, w, h);
}


bool PlaybackCursor::visible() const
{
    return m_visible;
}

void PlaybackCursor::setVisible(bool arg)
{
    m_visible = arg;
}

const muse::RectF& PlaybackCursor::rect() const
{
    return m_rect;
}

QColor PlaybackCursor::color() const
{
    return configuration()->playbackCursorColor();
}

// alex::
std::vector<EngravingItem*>& PlaybackCursor::hit_elements() { 
    return m_hit_el;
}
int PlaybackCursor::hit_measure_no() { return m_hit_measure_no; }
size_t PlaybackCursor::seg_note_duration_tree() { return m_seg_note_duration_tree; }
std::vector<std::vector<EngravingItem*>> PlaybackCursor::seg_records() { return m_seg_records; }
std::vector<size_t> PlaybackCursor::staffindex_curr_at_segindex_records() { return m_staffindex_curr_at_segindex_records; }

void PlaybackCursor::setHitElements(std::vector<EngravingItem*>& el) { 
    m_hit_el = el;
}
void PlaybackCursor::setHitMeasureNo(int m_no) { 
    m_hit_measure_no = m_no; 
}
void PlaybackCursor::setSegNoteDurationTree(size_t m_tree) { 
    m_seg_note_duration_tree = m_tree; 
}
void PlaybackCursor::pushSegRecords(std::vector<EngravingItem*> item) { 
    m_seg_records.push_back(item); 
}
void PlaybackCursor::pushStaffindexCurrAtSegindexRecords(size_t m_seg_index) { 
    m_staffindex_curr_at_segindex_records.push_back(m_seg_index); 
}
void PlaybackCursor::updateStaffindexCurrAtSegindex(size_t m_staffindex, size_t m_seg_atindex) { 
    m_staffindex_curr_at_segindex_records[m_staffindex] = m_seg_atindex; 
}

void PlaybackCursor::highlightAt(size_t seg_index, size_t seg_track_index, bool is_highlight) {
    if (seg_index < 0) {
        return;
    }
    if (seg_track_index < 0) {
        return;
    }
    if (seg_index >= m_seg_records.size()) {
        return;
    }
    if (seg_track_index >= m_seg_records[seg_index].size()) {
        return;
    }
    if (m_seg_records[seg_index][seg_track_index] != nullptr) {
        if (is_highlight) {
            m_seg_records[seg_index][seg_track_index]->setColor(muse::draw::Color::RED);
        } else {
            m_seg_records[seg_index][seg_track_index]->setColor(muse::draw::Color::BLACK);
        } 
    }
}

void PlaybackCursor::clearSegRecords() { 
    m_seg_records.clear(); 
    m_seg_note_duration_tree = 0;
    m_staffindex_curr_at_segindex_records.clear();
}
