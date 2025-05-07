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

    muse::RectF measureRect = measure->pageBoundingRect();
    int measureNo = measure->no();

    mu::engraving::System* system = measure->system();
    if (!system) {
        return RectF();
    }

    qreal x = 0.0;
    mu::engraving::Segment* s = nullptr;
    mu::engraving::Segment* s1 = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        s1 = s;
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

    // alex:: lingering cursor
    std::vector<EngravingItem*> engravingItemList = s1->elist();
    int len = (int)engravingItemList.size();
    
    // int seg_note_tree = 0;
    for (int i = 0; i < len; i++) {
        EngravingItem* engravingItem = engravingItemList[(track_idx_t)i];
        
        if (engravingItem == nullptr) {
            continue;
        }

        // if (i % 4 == 0) {
        //     seg_note_tree |= 1 << (i / 4);
        // }

        engravingItem->setColor(muse::draw::Color::RED);
    }

    std::vector<EngravingItem*> oldHitElements = hit_elements();

    if (oldHitElements != engravingItemList) {
        // pushSegVectRecords(engravingItemList);
        int len1 = (int)oldHitElements.size();
        for (int i = 0; i < len1; i++) {
            EngravingItem* engravingItem = oldHitElements[(track_idx_t)i];
            if (engravingItem == nullptr) {
                continue;
            }
            engravingItem->setColor(muse::draw::Color::BLACK);
        }
        
        setHitElements(engravingItemList);

        // int seg_note_tree1 = seg_note_duration_tree();
        // if (seg_note_tree1 == 0) {
        //     setSegNoteDurationTree(seg_note_tree);
        //     // for (int i = 0; i < len; i++) {
        //     //     EngravingItem* engravingItem = engravingItemList[(track_idx_t)i];
        //     //     pushSegRecords(engravingItem);    
        //     // }
        //     // tick_records.push_back(tick);
        //     for (int i = 0; i < len1 / 4; i++) {
        //         pushStaffindexCurrAtSegindexRecords(0);
        //     }
        //     setHitMeasureNo(measureNo);
        // } else if (hit_measure_no() != measureNo) {
        //     std::vector<int> staffIndexAtSegRecords = staffindex_curr_at_segindex_records();
        //     for (int i = 0; i < len1 / 4; i++) {
        //         int atSegIndex = staffIndexAtSegRecords[i];
        //         if ((seg_note_tree1 & (1 << i)) > 0) {
        //             // highlightAt(atSegIndex, len1, 4 * i, false);
        //             highlightAt1(atSegIndex, 4 * i, false);
        //         }
        //     }
        //     clearSegRecords();
        //     setSegNoteDurationTree(seg_note_tree);
        //     // for (int i = 0; i < len; i++) {
        //     //     EngravingItem* engravingItem = engravingItemList[(track_idx_t)i];
        //     //     pushSegRecords(engravingItem);    
        //     // }
        //     // tick_records.push_back(tick);
        //     for (int i = 0; i < len1 / 4; i++) {
        //         pushStaffindexCurrAtSegindexRecords(0);
        //     }
        //     setHitMeasureNo(measureNo);
        // } else {
        //     // for (int i = 0; i < len; i++) {
        //     //     EngravingItem* engravingItem = engravingItemList[(track_idx_t)i];
        //     //     pushSegRecords(engravingItem);    
        //     // }
        //     // tick_records.push_back(tick);

        //     // m_seg_records.clear();
        //     // for (int i = 0; i < (int)tick_records.size(); i++) {
        //     //     Measure* measure_n = score->tick2measureMM(tick_records[i]);
        //     //     mu::engraving::Segment* s_n = measure_n->first(mu::engraving::SegmentType::ChordRest);

        //     //     std::vector<EngravingItem*> engravingItemList_n = s_n->elist();
        //     //     int len_n = (int)engravingItemList_n.size();

        //     //     for (int j = 0; j < len_n; j++) {
        //     //         EngravingItem* engravingItem_n = engravingItemList_n[(track_idx_t)j];
        //     //         pushSegRecords(engravingItem_n);
        //     //     }
        //     // }

        //     int xorResult = seg_note_tree1 ^ seg_note_tree;

        //     std::vector<EngravingItem*> segRecords = seg_records();
        //     // int segRecordsLen = (int)segRecords.size() / len1;
        //     // int segRecordsLen = tick_records.size();
        //     int segRecordsLen = m_el_count;

        //     std::vector<int> staffIndexAtSegRecords = staffindex_curr_at_segindex_records();
            
        //     for (int i = 0; i < len1 / 4; i++) {
        //         if ((xorResult & (1 << i)) > 0) {
        //             int atSegIndex = staffIndexAtSegRecords[(size_t)i];
        //             bool highlight_flag = false;
        //             int _atSegIndex = atSegIndex;
        //             if ((seg_note_tree1 & (1 << i)) > 0) {
        //                 highlight_flag = true;
        //             } else {
        //                 highlight_flag = false;
        //                 atSegIndex = segRecordsLen - 1;
        //                 updateStaffindexCurrAtSegindex(i, atSegIndex);
        //             }
                    
        //             // highlightAt(_atSegIndex, len1, 4 * i, highlight_flag);
        //             highlightAt1(_atSegIndex, 4 * i, highlight_flag);
        //         }
        //     }

        //     setSegNoteDurationTree(seg_note_tree1 | seg_note_tree);
        // }
        
        
        if (measureNo < 2) {
            emit lingeringCursorUpdate(0.0, measureRect.y(), measureRect.width(), measureRect.height());
        } else {
            emit lingeringCursorUpdate(measureRect.x(), measureRect.y(), measureRect.width(), measureRect.height());
        }
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
std::vector<EngravingItem*>& PlaybackCursor::hit_elements() { return m_hit_el; }
int PlaybackCursor::hit_measure_no() { return m_hit_measure_no; }
int PlaybackCursor::seg_note_duration_tree() { return m_seg_note_duration_tree; }
std::vector<EngravingItem*> PlaybackCursor::seg_records() { return m_seg_records; }
std::vector<int> PlaybackCursor::staffindex_curr_at_segindex_records() { return m_staffindex_curr_at_segindex_records; }

void PlaybackCursor::setHitElements(std::vector<EngravingItem*>& el) { 
    m_hit_el = el; 
}
void PlaybackCursor::setHitMeasureNo(int m_no) { 
    m_hit_measure_no = m_no; 
}
void PlaybackCursor::setSegNoteDurationTree(int m_tree) { 
    m_seg_note_duration_tree = m_tree; 
}
void PlaybackCursor::pushSegRecords(EngravingItem* item) { 
    m_seg_records.push_back(item); 
}
void PlaybackCursor::pushSegVectRecords(std::vector<EngravingItem*> vect_record) {
    if (m_el_count >= m_el_size) {
        m_el_count = m_el_size;
        for (int i = 0; i < m_el_size - 1; i++) {
            m_el_array[i] = m_el_array[i + 1];
        }
        m_el_array[m_el_size - 1] = vect_record;
    } else {
        m_el_array[m_el_count++] = vect_record;
    }
}
void PlaybackCursor::pushStaffindexCurrAtSegindexRecords(int m_seg_index) { 
    m_staffindex_curr_at_segindex_records.push_back(m_seg_index); 
}
void PlaybackCursor::updateStaffindexCurrAtSegindex(int m_staffindex, int m_seg_atindex) { 
    m_staffindex_curr_at_segindex_records[m_staffindex] = m_seg_atindex; 
}

// tofix- system_error: recursive_mutex lock failed: Invalid argument
void PlaybackCursor::highlightAt(int seg_index, int step, int seg_track_index, bool is_highlight) {
    int _index = seg_index * step + seg_track_index;
    if (is_highlight) {
        if (_index < (int)m_seg_records.size() && m_seg_records[(size_t)_index] != nullptr) {
            m_seg_records[_index]->setColor(muse::draw::Color::RED);
        }
    } else {
        if (_index < (int)m_seg_records.size() && m_seg_records[(size_t)_index] != nullptr) {
            m_seg_records[_index]->setColor(muse::draw::Color::BLACK);
        }
    }
}

void PlaybackCursor::highlightAt1(int seg_index, int seg_track_index, bool is_highlight) {
    if (seg_index < m_el_count && m_el_array[seg_index][seg_track_index] != nullptr) {
        if (is_highlight) {
            m_el_array[seg_index][seg_track_index]->setColor(muse::draw::Color::RED);
        } else {
            m_el_array[seg_index][seg_track_index]->setColor(muse::draw::Color::BLACK);
        }

    }
}

void PlaybackCursor::clearSegRecords() { 
    m_seg_records.clear(); 
    m_seg_note_duration_tree = 0;
    m_staffindex_curr_at_segindex_records.clear();
    tick_records.clear();

    m_el_count = 0;
}
