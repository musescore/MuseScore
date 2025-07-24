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

        // alex:: check note ticks and duration, compare with current ticks
        std::vector<EngravingItem*> engravingItemList = s->elist();
        size_t len = engravingItemList.size();
        for (size_t i = 0; i < len; i++) {
            EngravingItem* engravingItem = engravingItemList[i];
            if (engravingItem == nullptr) {
                continue;
            }  
            ChordRest *chordRest = toChordRest(engravingItem);
            // mu::engraving::TDuration duration = chordRest->durationType();
            // int duration_ticks = duration.ticks().ticks();
            int duration_ticks = chordRest->durationTypeTicks().ticks();
            // LOGALEX() << "curr_ticks: " << tick.ticks() << ", note ticks: " << t1.ticks() << ", duration_ticks: " << duration_ticks;

            if (t1.ticks() + duration_ticks < tick.ticks()) {
                engravingItem->setColor(muse::draw::Color::BLACK);
            } else {
                engravingItem->setColor(muse::draw::Color::RED);
            }
        }

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
    
    for (size_t i = 0; i < len; i++) {
        EngravingItem* engravingItem = engravingItemList[i];
        
        if (engravingItem == nullptr) {
            continue;
        }

        engravingItem->setColor(muse::draw::Color::RED);
    }

    // muse::RectF measureRect = measure->pageBoundingRect();
    int measureNo = measure->no();
    std::vector<EngravingItem*> oldHitElements = hit_elements();
    if (oldHitElements != engravingItemList) {
        setHitElements(engravingItemList);

        if (hit_measure_no() != measureNo || hit_measure() != measure) {
            Measure* prevMeasure = measure->prevMeasure();
            if (prevMeasure) {
                for (mu::engraving::Segment* segment = prevMeasure->first(mu::engraving::SegmentType::ChordRest); segment;) {
                    std::vector<EngravingItem*> engravingItemListOfPrevMeasure = segment->elist();
                    size_t prev_len = engravingItemListOfPrevMeasure.size();
                    for (size_t i = 0; i < prev_len; i++) {
                        EngravingItem* engravingItem = engravingItemListOfPrevMeasure[i];
                        if (engravingItem == nullptr) {
                            continue;
                        }
                        engravingItem->setColor(muse::draw::Color::BLACK);
                    }

                    mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
                    while (next_segment && !next_segment->visible()) {
                        next_segment = next_segment->next(mu::engraving::SegmentType::ChordRest);
                    }
                    segment = next_segment;
                }
            }

            setHitMeasureNo(measureNo);
            setHitMeasure(measure);
        }
        
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
Measure *PlaybackCursor::hit_measure() {
    return m_hit_measure;
}

void PlaybackCursor::setHitElements(std::vector<EngravingItem*>& el) { 
    m_hit_el = el;
}
void PlaybackCursor::setHitMeasureNo(int m_no) { 
    m_hit_measure_no = m_no; 
}
void PlaybackCursor::setHitMeasure(Measure *m) {
    m_hit_measure = m;
}
