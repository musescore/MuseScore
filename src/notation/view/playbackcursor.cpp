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
#include "src/notation/notationtypes.h"
#include "src/engraving/dom/ornament.h"
#include "src/engraving/dom/trill.h"
#include "src/engraving/dom/tremolobar.h"
#include "src/engraving/dom/arpeggio.h"
#include "src/engraving/dom/ottava.h"
#include "src/engraving/dom/spanner.h"
#include "src/engraving/dom/rest.h"
#include "src/engraving/dom/mmrest.h"
#include "src/engraving/dom/notedot.h"
#include "src/engraving/dom/accidental.h"
#include "src/engraving/dom/stem.h"
#include "src/engraving/dom/hook.h"
#include "src/engraving/dom/beam.h"
#include "src/engraving/dom/dynamic.h"
#include "src/engraving/dom/hairpin.h"

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
    if (m_notation) {
        mu::engraving::Score* score = m_notation->elements()->msScore();
        if (score) {
            if (score->nstaves() > 6) {
                preProcessScore = true;
                processOttava(score, false);
            } else if (score->nstaves() > 1) {
                processOttava(score, false);
            }
        }
    }
}

void PlaybackCursor::enableHighlightCursorNote(bool highlight)
{
    highlightCursorNote = highlight;
}

void PlaybackCursor::enableKeyboardPlay(bool enable)
{
    pianoKeyboardPlaybackEnable = enable;
}

void PlaybackCursor::move(muse::midi::tick_t tick, bool isPlaying)
{
    // LOGALEX();
    if (highlightCursorNote) {
        m_rect = resolveCursorRectByTick1(tick, isPlaying);
    } else {
        m_rect = resolveCursorRectByTick(tick);
    }
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
bool compare_by_chord_x(Chord* a, Chord* b) {
    return a->canvasPos().x() < b->canvasPos().x();
}

void PlaybackCursor::processOttava(mu::engraving::Score* score, bool isPlaying) {
    if (m_isOttavaProcessed && isPlaying) {
        return;
    }
    if (m_ottavaProcessFuture.valid()) {
        m_ottavaProcessFuture.wait();
    }
    m_ottavaProcessFuture = std::async(std::launch::async, [this, score]() {
        processOttavaAsync(score);
        m_isOttavaProcessed = true;
    });
}

void PlaybackCursor::processOttavaAsync(mu::engraving::Score* score) {
    std::map<int, int> staff_stick_map;
    std::map<int, int> staff_etick_map;
    std::map<int, int> staff_ottatype_map;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (mu::engraving::Segment* s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
            std::vector<EngravingItem*> engravingItemList = s->elist();
            for (size_t i = 0; i < engravingItemList.size(); i++) {
                EngravingItem* engravingItem = engravingItemList[i];
                if (engravingItem == nullptr) {
                    continue;
                }  
                EngravingItemList itemList = engravingItem->childrenItems(false);
                for (size_t j = 0; j < itemList.size(); j++) {
                    EngravingItem* item_ = itemList.at(j);
                    if (item_ == nullptr) {
                        continue;
                    }
                    if (item_->type() == mu::engraving::ElementType::NOTE) {
                        Note* note_ = toNote(item_);
                        Staff* noteStaff = note_->staff();
                        int noteStaffIndex = noteStaff->idx();
                        bool isOttavaStartd = false;
                        const std::set<Spanner*> starttingSpanners_ = note_->chord()->startingSpanners();
                        for (const Spanner* _spanner : starttingSpanners_) {
                            if (_spanner->isOttava()) {
                                int startTicks = _spanner->tick().ticks();
                                int endTicks = _spanner->tick2().ticks();
                                staff_stick_map[noteStaffIndex] = startTicks;
                                staff_etick_map[noteStaffIndex] = endTicks;
                                int _ottavaType = (int)toOttava(_spanner)->ottavaType() + 100;
                                staff_ottatype_map[noteStaffIndex] = _ottavaType;
                                ottava_map[note_] = _ottavaType;
                                isOttavaStartd = true;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }
                                // forward check some notes
                                EngravingItemList forwardList = measure->childrenItems(true);
                                for (size_t fj = 0; fj < forwardList.size(); fj++) {
                                    EngravingItem* forward_item = forwardList.at(fj);
                                    if (forward_item == nullptr) {
                                        continue;
                                    }
                                    if (forward_item->type() == mu::engraving::ElementType::NOTE) {
                                        Note* forward_note = toNote(forward_item);
                                        if (forward_note->staff()->idx() == noteStaffIndex && forward_note->canvasPos().x() <= note_->canvasPos().x()) {
                                            if (forward_note->tick().ticks() >= startTicks && forward_note->tick().ticks() <= endTicks) {
                                                ottava_map[forward_note] = _ottavaType;
                                                if (forward_note->chord()) {
                                                    std::vector<Note*> chordNotes = forward_note->chord()->notes();
                                                    for (Note* note_in_chord : chordNotes) {
                                                        ottava_map[note_in_chord] = _ottavaType;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }

                        const std::set<Spanner*> endingSpanners_ = note_->chord()->endingSpanners();
                        for (const Spanner* _spanner : endingSpanners_) {
                            if (_spanner->isOttava()) {
                                int startTicks = staff_stick_map[noteStaffIndex];
                                int endTicks = staff_etick_map[noteStaffIndex];
                                staff_stick_map[noteStaffIndex] = 0;
                                staff_etick_map[noteStaffIndex] = 0;
                                isOttavaStartd = false;
                                int _ottavaType = staff_ottatype_map[noteStaffIndex];
                                staff_ottatype_map[noteStaffIndex] = 0;
                                ottava_map[note_] = _ottavaType;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }
                                // backward check some notes
                                EngravingItemList backwardList = measure->childrenItems(true);
                                for (size_t fj = 0; fj < backwardList.size(); fj++) {
                                    EngravingItem* backward_item = backwardList.at(fj);
                                    if (backward_item == nullptr) {
                                        continue;
                                    }
                                    if (backward_item->type() == mu::engraving::ElementType::NOTE) {
                                        Note* backward_note = toNote(backward_item);
                                        if (backward_note->staff()->idx() == noteStaffIndex && backward_note->canvasPos().x() >= note_->canvasPos().x()) {
                                            if (backward_note->tick().ticks() >= startTicks && backward_note->tick().ticks() <= endTicks) {
                                                ottava_map[backward_note] = _ottavaType;
                                                if (backward_note->chord()) {
                                                    std::vector<Note*> chordNotes = backward_note->chord()->notes();
                                                    for (Note* note_in_chord : chordNotes) {
                                                        ottava_map[note_in_chord] = _ottavaType;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }

                        if (!isOttavaStartd && staff_stick_map[noteStaffIndex] > 0 && staff_etick_map[noteStaffIndex] > 0) {
                            if (note_->tick().ticks() >= staff_stick_map[noteStaffIndex] && note_->tick().ticks() <= staff_etick_map[noteStaffIndex]) {
                                int _ottavaType = staff_ottatype_map[noteStaffIndex];
                                ottava_map[note_] = _ottavaType;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
            while (ns && !ns->visible()) {
                ns = ns->next(mu::engraving::SegmentType::ChordRest);
            }
            s = ns;
        }
    }

    staff_stick_map.clear();
    staff_etick_map.clear();
    staff_ottatype_map.clear();
    for (const Measure* measure = score->lastMeasure(); measure; measure = measure->prevMeasure()) {
        for (mu::engraving::Segment* s = measure->last(mu::engraving::SegmentType::ChordRest); s;) {
            std::vector<EngravingItem*> engravingItemList = s->elist();
            for (size_t i = 0; i < engravingItemList.size(); i++) {
                EngravingItem* engravingItem = engravingItemList[i];
                if (engravingItem == nullptr) {
                    continue;
                }  
                EngravingItemList itemList = engravingItem->childrenItems(false);
                for (size_t j = 0; j < itemList.size(); j++) {
                    EngravingItem* item_ = itemList.at(j);
                    if (item_ == nullptr) {
                        continue;
                    }
                    if (item_->type() == mu::engraving::ElementType::NOTE) {
                        Note* note_ = toNote(item_);
                        Staff* noteStaff = note_->staff();
                        int noteStaffIndex = noteStaff->idx();

                        bool isOttavaStartd = false;
                        const std::set<Spanner*> endingSpanners_ = note_->chord()->endingSpanners();
                        for (const Spanner* _spanner : endingSpanners_) {
                            if (_spanner->isOttava()) {
                                int startTicks = _spanner->tick().ticks();
                                int endTicks = _spanner->tick2().ticks();
                                staff_stick_map[noteStaffIndex] = startTicks;
                                staff_etick_map[noteStaffIndex] = endTicks;
                                int _ottavaType = (int)toOttava(_spanner)->ottavaType() + 100;
                                staff_ottatype_map[noteStaffIndex] = _ottavaType;
                                ottava_map[note_] = _ottavaType;
                                isOttavaStartd = true;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }

                                // backward check some notes
                                EngravingItemList backwardList = measure->childrenItems(true);
                                for (size_t fj = 0; fj < backwardList.size(); fj++) {
                                    EngravingItem* backward_item = backwardList.at(fj);
                                    if (backward_item == nullptr) {
                                        continue;
                                    }
                                    if (backward_item->type() == mu::engraving::ElementType::NOTE) {
                                        Note* backward_note = toNote(backward_item);
                                        if (backward_note->staff()->idx() == noteStaffIndex && backward_note->canvasPos().x() >= note_->canvasPos().x()) {
                                            if (backward_note->tick().ticks() >= startTicks && backward_note->tick().ticks() <= endTicks) {
                                                ottava_map[backward_note] = _ottavaType;
                                                if (backward_note->chord()) {
                                                    std::vector<Note*> chordNotes = backward_note->chord()->notes();
                                                    for (Note* note_in_chord : chordNotes) {
                                                        ottava_map[note_in_chord] = _ottavaType;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        

                        const std::set<Spanner*> starttingSpanners_ = note_->chord()->startingSpanners();
                        for (const Spanner* _spanner : starttingSpanners_) {
                            if (_spanner->isOttava()) {
                                int startTicks = staff_stick_map[noteStaffIndex];
                                int endTicks = staff_etick_map[noteStaffIndex];
                                staff_stick_map[noteStaffIndex] = 0;
                                staff_etick_map[noteStaffIndex] = 0;
                                isOttavaStartd = false;
                                int _ottavaType = staff_ottatype_map[noteStaffIndex];
                                staff_ottatype_map[noteStaffIndex] = 0;
                                ottava_map[note_] = _ottavaType;
                                isOttavaStartd = true;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }

                                // forward check some notes
                                EngravingItemList forwardList = measure->childrenItems(true);
                                for (size_t fj = 0; fj < forwardList.size(); fj++) {
                                    EngravingItem* forward_item = forwardList.at(fj);
                                    if (forward_item == nullptr) {
                                        continue;
                                    }
                                    if (forward_item->type() == mu::engraving::ElementType::NOTE) {
                                        Note* forward_note = toNote(forward_item);
                                        if (forward_note->staff()->idx() == noteStaffIndex && forward_note->canvasPos().x() <= note_->canvasPos().x()) {
                                            if (forward_note->tick().ticks() >= startTicks && forward_note->tick().ticks() <= endTicks) {
                                                ottava_map[forward_note] = _ottavaType;
                                                if (forward_note->chord()) {
                                                    std::vector<Note*> chordNotes = forward_note->chord()->notes();
                                                    for (Note* note_in_chord : chordNotes) {
                                                        ottava_map[note_in_chord] = _ottavaType;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }

                        if (!isOttavaStartd && staff_stick_map[noteStaffIndex] > 0 && staff_etick_map[noteStaffIndex] > 0) {
                            if (note_->tick().ticks() >= staff_stick_map[noteStaffIndex] && note_->tick().ticks() <= staff_etick_map[noteStaffIndex]) {
                                int _ottavaType = staff_ottatype_map[noteStaffIndex];
                                ottava_map[note_] = _ottavaType;
                                if (note_->chord()) {
                                    std::vector<Note*> chordNotes = note_->chord()->notes();
                                    for (Note* note_in_chord : chordNotes) {
                                        ottava_map[note_in_chord] = _ottavaType;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            mu::engraving::Segment* ns = s->prev(mu::engraving::SegmentType::ChordRest);
            while (ns && !ns->visible()) {
                ns = ns->prev(mu::engraving::SegmentType::ChordRest);
            }
            s = ns;
        }
    }

    SpannerMap& smap = score->spannerMap();
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        if (measure_spanner_map.find(measure->no()) == measure_spanner_map.end()) {
            measure_spanner_map[measure->no()] = {};
        }
        EngravingItemList measure_children = measure->childrenItems(true);
        
        int min_ticks = 0;
        int max_ticks = 0;
        
        for (size_t m_k = 0; m_k < measure_children.size(); m_k++) {
            EngravingItem* measure_item = measure_children.at(m_k);
            if (min_ticks == 0) {
                min_ticks = measure_item->tick().ticks();
            }
            if (max_ticks == 0) {
                max_ticks = measure_item->tick().ticks();
            }
            if (measure_item->tick().ticks() < min_ticks) {
                min_ticks = measure_item->tick().ticks();
            }
            if (measure_item->tick().ticks() > max_ticks) {
                max_ticks = measure_item->tick().ticks();
            }

            if (measure_item->type() == mu::engraving::ElementType::NOTE) {
                Note* note_item = toNote(measure_item);
                Tie* _tieBack = note_item->tieBack();
                Tie* _tieFor = note_item->tieFor();
                if (_tieBack) {
                    if (_tieBack->startNote() && _tieBack->endNote()) {
                        measure_spanner_map[measure->no()].insert((EngravingItem*)_tieBack);
                        if (spanner_ticks_map.find((EngravingItem*)_tieBack) == spanner_ticks_map.end()) {
                            spanner_ticks_map[(EngravingItem*)_tieBack] = {};
                        }

                        Note* _startNote = _tieBack->startNote();
                        Note* _endNote = _tieBack->endNote();
                        
                        spanner_ticks_map[(EngravingItem*)_tieBack][0] = _startNote->tick().ticks();
                        spanner_ticks_map[(EngravingItem*)_tieBack][1] = _endNote->tick().ticks();
                    }
                }
                if (_tieFor) {
                    if (_tieFor->startNote() && _tieFor->endNote()) {
                        measure_spanner_map[measure->no()].insert((EngravingItem*)_tieFor);
                        if (spanner_ticks_map.find((EngravingItem*)_tieFor) == spanner_ticks_map.end()) {
                            spanner_ticks_map[(EngravingItem*)_tieFor] = {};
                        }

                        Note* _startNote = _tieFor->startNote();
                        Note* _endNote = _tieFor->endNote();

                        spanner_ticks_map[(EngravingItem*)_tieFor][0] = _startNote->tick().ticks();
                        spanner_ticks_map[(EngravingItem*)_tieFor][1] = _endNote->tick().ticks();
                    }
                }

                Chord* note_chord = note_item->chord();
                if (note_chord) {
                    const std::set<Spanner*> sSpanners = note_chord->startingSpanners();
                    const std::set<Spanner*> eSpanners = note_chord->endingSpanners();

                    for (Spanner* spanner : sSpanners) {
                        int spannerStartTicks = spanner->tick().ticks();
                        int spannerEndTicks = spanner->tick2().ticks();
                        if (spanner->isSlur() || spanner->isTrill() || spanner->isGlissando() || spanner->isHairpin()) {
                            measure_spanner_map[measure->no()].insert((EngravingItem*)spanner);
                            if (spanner_ticks_map.find((EngravingItem*)spanner) == spanner_ticks_map.end()) {
                                spanner_ticks_map[(EngravingItem*)spanner] = {};
                            }
                            spanner_ticks_map[(EngravingItem*)spanner][0] = spannerStartTicks;
                            spanner_ticks_map[(EngravingItem*)spanner][1] = spannerEndTicks;
                        }
                    }

                    for (Spanner* spanner : eSpanners) {
                        int spannerStartTicks = spanner->tick().ticks();
                        int spannerEndTicks = spanner->tick2().ticks();
                        if (spanner->isSlur() || spanner->isTrill() || spanner->isGlissando() || spanner->isHairpin()) {
                            measure_spanner_map[measure->no()].insert((EngravingItem*)spanner);
                            if (spanner_ticks_map.find((EngravingItem*)spanner) == spanner_ticks_map.end()) {
                                spanner_ticks_map[(EngravingItem*)spanner] = {};
                            }
                            spanner_ticks_map[(EngravingItem*)spanner][0] = spannerStartTicks;
                            spanner_ticks_map[(EngravingItem*)spanner][1] = spannerEndTicks;
                        }
                    }
                }
            }
        }

        auto spanners = smap.findOverlapping(min_ticks, max_ticks);
        for (auto interval : spanners) {
            Spanner* spanner = interval.value;
            int spannerStartTicks = spanner->tick().ticks();
            int spannerEndTicks = spanner->tick2().ticks();
            EngravingItem* startElem = spanner->startElement();
            EngravingItem* endElem = spanner->endElement();
            if (spanner->isOttava()) {
                if (startElem->staff()->idx() == endElem->staff()->idx()) {
                    for (size_t _k = 0; _k < measure_children.size(); _k++) {
                        EngravingItem* _item = measure_children.at(_k);
                        if (_item->type() == mu::engraving::ElementType::NOTE) {
                            if (_item->staff()->idx() == startElem->staff()->idx() && ottava_map[toNote(_item)] == 0) {
                                if (_item->tick().ticks() >= spannerStartTicks && _item->tick().ticks() <= spannerEndTicks) {
                                    ottava_map[toNote(_item)] = (int)toOttava(spanner)->ottavaType() + 100;
                                }
                            }
                        }
                    }
                }
            }

            if (spanner->isSlur() || spanner->isTrill() || spanner->isGlissando() || spanner->isHairpin()) {
                measure_spanner_map[measure->no()].insert((EngravingItem*)spanner);
                if (spanner_ticks_map.find((EngravingItem*)spanner) == spanner_ticks_map.end()) {
                    spanner_ticks_map[(EngravingItem*)spanner] = {};
                }
                spanner_ticks_map[(EngravingItem*)spanner][0] = spannerStartTicks;
                spanner_ticks_map[(EngravingItem*)spanner][1] = spannerEndTicks;
            }
        }
        spanners = smap.findContained(min_ticks, max_ticks);
        for (auto interval : spanners) {
            Spanner* spanner = interval.value;
            int spannerStartTicks = spanner->tick().ticks();
            int spannerEndTicks = spanner->tick2().ticks();
            EngravingItem* startElem = spanner->startElement();
            EngravingItem* endElem = spanner->endElement();
            if (spanner->isOttava()) {
                if (startElem->staff()->idx() == endElem->staff()->idx()) {
                    for (size_t _k = 0; _k < measure_children.size(); _k++) {
                        EngravingItem* _item = measure_children.at(_k);
                        if (_item->type() == mu::engraving::ElementType::NOTE) {
                            if (_item->staff()->idx() == startElem->staff()->idx() && ottava_map[toNote(_item)] == 0) {
                                if (_item->tick().ticks() >= spannerStartTicks && _item->tick().ticks() <= spannerEndTicks) {
                                    ottava_map[toNote(_item)] = (int)toOttava(spanner)->ottavaType() + 100;
                                }
                            }
                        }
                    }
                }
            }

            if (spanner->isSlur() || spanner->isTrill() || spanner->isGlissando() || spanner->isHairpin()) {
                measure_spanner_map[measure->no()].insert((EngravingItem*)spanner);
                if (spanner_ticks_map.find((EngravingItem*)spanner) == spanner_ticks_map.end()) {
                    spanner_ticks_map[(EngravingItem*)spanner] = {};
                }
                spanner_ticks_map[(EngravingItem*)spanner][0] = spannerStartTicks;
                spanner_ticks_map[(EngravingItem*)spanner][1] = spannerEndTicks;
            }
        }
    }

    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        std::set<Note*> _measure_trill_notes;
        std::map<Note*, int> _measure_trill_notes_trill_type_map;
        std::map<Note*, EngravingItem*> _measure_trill_notes_trill_map;
        std::map<Note*, int> _measure_tremolo_type_map;
        std::map<Note*, bool> _measure_tremolo_half_map;
        EngravingItemList measure_children = measure->childrenItems(true);
        bool hasTremoloBar = false;
        for (size_t m_k = 0; m_k < measure_children.size(); m_k++) {
            EngravingItem* measure_item = measure_children.at(m_k);
            if (measure_item && measure_item->isOrnament()) {
                Ornament* orn = toOrnament(measure_item); // subtype 2214(trill). 2215(turn) 2216(Inverted turn)  2217(turn with slash)
                Trill* _trill = toTrill(measure_item);
                if (_trill) {
                    Note* note = orn->noteAbove();
                    
                    if (spanner_ticks_map.find((EngravingItem*)_trill) != spanner_ticks_map.end()) {
                        _measure_trill_notes_trill_map[note] = (EngravingItem*)_trill;
                    }
                    
                    if (note) {
                        _measure_trill_notes.insert(note);
                        _measure_trill_notes_trill_type_map[note] = orn->subtype();

                        for (size_t m_k1 = 0; m_k1 < measure_children.size(); m_k1++) {
                            EngravingItem* measure_item1 = measure_children.at(m_k1);
                            if (measure_item1 && measure_item1->isTie()) {
                                Tie* tie = toTie(measure_item1);
                                if (tie->startNote() == note) {
                                    score_trill_tie_map[note] = true;
                                }
                            }
                        }
                    }
                }
            }

            TremoloBar* _tremoloBar = toTremoloBar(measure_item);
            if (_tremoloBar) {
                hasTremoloBar = true;
            }
        }

        if (hasTremoloBar) {
            for (size_t m_k = 0; m_k < measure_children.size(); m_k++) {
                EngravingItem* measure_item = measure_children.at(m_k);
                if (measure_item->type() == mu::engraving::ElementType::NOTE) {
                    Note* _tremoloNote = toNote(measure_item);
                    TremoloType _tremoloType = _tremoloNote->chord()->tremoloType();
                    _measure_tremolo_type_map[_tremoloNote] = (int)_tremoloType;

                    TremoloTwoChord* _tremoloTwoChord = _tremoloNote->chord()->tremoloTwoChord();
                    TremoloSingleChord* _tremoloSingleChord = _tremoloNote->chord()->tremoloSingleChord();
                    if (_tremoloTwoChord) {
                        Chord* chord1 = _tremoloTwoChord->chord1();
                        Chord* chord2 = _tremoloTwoChord->chord2();

                        Note* leftFirstNote = chord1->notes()[0];
                        Note* leftLastNote = chord1->notes()[chord1->notes().size() - 1];
                        Note* leftNote = leftFirstNote;
                        if (leftFirstNote->canvasPos().y() < leftLastNote->canvasPos().y()) {
                            leftNote = leftLastNote;
                        }
                        _measure_trill_notes.insert(leftNote);
                        
                        Note* rightFirstNote = chord2->notes()[0];
                        Note* rightLastNote = chord2->notes()[chord2->notes().size() - 1];
                        Note* rightNote = rightFirstNote;
                        if (rightFirstNote->canvasPos().y() < rightLastNote->canvasPos().y()) {
                            rightNote = rightLastNote;
                        }
                        _measure_trill_notes.insert(rightNote);
                        _measure_tremolo_half_map[leftNote] = true;
                        _measure_tremolo_half_map[rightNote] = true;
                    }
                    if (_tremoloSingleChord) {
                        Chord* _chord = _tremoloNote->chord();
                        Note* _firstNote = _chord->notes()[0];
                        Note* _lastNote = _chord->notes()[_chord->notes().size() - 1];
                        Note* _note = _firstNote;
                        if (_firstNote->canvasPos().y() < _lastNote->canvasPos().y()) {
                            _note = _lastNote;
                        }
                        _measure_trill_notes.insert(_note); 
                    }
                }
            }
        }

        for (mu::engraving::Segment* s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
            EngravingItemList _list = s->childrenItems(true);
            std::map<int, EngravingItem*> staff_fermata_map;
            for (size_t i = 0; i < _list.size(); i++) {
                EngravingItem* _item = _list.at(i);
                if (_item->type() == mu::engraving::ElementType::FERMATA) {
                    staff_fermata_map[_item->staff()->idx()] = _item;
                }
            }

            std::vector<EngravingItem*> engravingItemList = s->elist();
            bool _arpeggio_seg_checked = false;
            bool _glissando_seg_checked = false;
            
            bool _trill_note_checked = false;
            bool _trill_note1_checked = false;
            for (size_t i = 0; i < engravingItemList.size(); i++) {
                EngravingItem* engravingItem = engravingItemList[i];
                if (engravingItem == nullptr) {
                    continue;
                } 
                int staff_idx = engravingItem->staff()->idx();
                if (staff_fermata_map.find(staff_idx) != staff_fermata_map.end()) {
                    chordrest_fermata_map[engravingItem] = staff_fermata_map[staff_idx];
                }
                ChordRest *chordRest = toChordRest(engravingItem);
                int duration_ticks = chordRest->durationTypeTicks().ticks();
                
                EngravingItemList itemList = engravingItem->childrenItems(true);
                
                for (size_t j = 0; j < itemList.size(); j++) {
                    EngravingItem* item = itemList.at(j);
                    if (item == nullptr) {
                        continue;
                    }
                    if (item->type() == mu::engraving::ElementType::NOTE) {
                        Note* _pre_note = toNote(item);
                        for (Note* mnote : _measure_trill_notes) {
                            if (mnote == _pre_note) {
                                if (!_trill_note_checked) {
                                    _trill_note_checked = true;
                                    int logic_tremoloType = 0;
                                    int _duration_ticks = duration_ticks;
                                    if (_measure_tremolo_type_map.find(mnote) != _measure_tremolo_type_map.end()) {
                                        if (_measure_tremolo_half_map[mnote]) {
                                            _duration_ticks /= 2;
                                        }
                                        int _tremoloType = _measure_tremolo_type_map[mnote];
                                        TremoloType tremoloType = (TremoloType)_tremoloType;
                                        if (tremoloType == TremoloType::R8 || tremoloType == TremoloType::C8) {
                                            logic_tremoloType = 20;
                                        } else if (tremoloType == TremoloType::R16 || tremoloType == TremoloType::C16) {
                                            logic_tremoloType = 40;
                                        } else if (tremoloType == TremoloType::R32 || tremoloType == TremoloType::C32) {
                                            logic_tremoloType = 80;
                                        } else if (tremoloType == TremoloType::R64 || tremoloType == TremoloType::C64) {
                                            logic_tremoloType = 160;
                                        } else if (tremoloType == TremoloType::BUZZ_ROLL) {
                                            logic_tremoloType = 50;
                                        }
                                    }

                                    int trill_type = 0;
                                    if (_measure_trill_notes_trill_type_map.find(mnote) != _measure_trill_notes_trill_type_map.end()) {
                                        trill_type = _measure_trill_notes_trill_type_map[mnote];
                                    }
                                    
                                    score_trill_map[engravingItem] = mnote;
                                    score_trill_type_map[engravingItem] = trill_type;
                                    score_trill_st_map[engravingItem] = mnote->tick().ticks();
                                    score_trill_dt_map[engravingItem] = _duration_ticks;
                                    score_trill_tt_map[engravingItem] = logic_tremoloType;
                                    score_trill_ot_map[engravingItem] = ottava_map[mnote];

                                    score_trill_tdt_map[engravingItem] = 0;
                                    if (_measure_trill_notes_trill_map.find(mnote) != _measure_trill_notes_trill_map.end()) {
                                        EngravingItem* __trill = _measure_trill_notes_trill_map[mnote];
                                        score_trill_tdt_map[engravingItem] = spanner_ticks_map[__trill][1] - spanner_ticks_map[__trill][0];
                                    }
                                    
                                } else if (_trill_note_checked && !_trill_note1_checked) {
                                    _trill_note1_checked = true;
                                    int logic_tremoloType = 0;
                                    int _duration_ticks = duration_ticks;
                                    if (_measure_tremolo_type_map.find(mnote) != _measure_tremolo_type_map.end()) {
                                        if (_measure_tremolo_half_map[mnote]) {
                                            _duration_ticks /= 2;
                                        }
                                        int _tremoloType = _measure_tremolo_type_map[mnote];
                                        TremoloType tremoloType = (TremoloType)_tremoloType;
                                        if (tremoloType == TremoloType::R8 || tremoloType == TremoloType::C8) {
                                            logic_tremoloType = 20;
                                        } else if (tremoloType == TremoloType::R16 || tremoloType == TremoloType::C16) {
                                            logic_tremoloType = 40;
                                        } else if (tremoloType == TremoloType::R32 || tremoloType == TremoloType::C32) {
                                            logic_tremoloType = 80;
                                        } else if (tremoloType == TremoloType::R64 || tremoloType == TremoloType::C64) {
                                            logic_tremoloType = 160;
                                        } else if (tremoloType == TremoloType::BUZZ_ROLL) {
                                            logic_tremoloType = 50;
                                        }
                                    }

                                    int trill_type = 0;
                                    if (_measure_trill_notes_trill_type_map.find(mnote) != _measure_trill_notes_trill_type_map.end()) {
                                        trill_type = _measure_trill_notes_trill_type_map[mnote];
                                    }

                                    score_trill_map1[engravingItem] = mnote;
                                    score_trill_type_map1[engravingItem] = trill_type;
                                    score_trill_st_map1[engravingItem] = mnote->tick().ticks();
                                    score_trill_dt_map1[engravingItem] = _duration_ticks;
                                    score_trill_tt_map1[engravingItem] = logic_tremoloType;
                                    score_trill_ot_map1[engravingItem] = ottava_map[mnote];

                                    score_trill_tdt_map1[engravingItem] = 0;
                                    if (_measure_trill_notes_trill_map.find(mnote) != _measure_trill_notes_trill_map.end()) {
                                        EngravingItem* __trill = _measure_trill_notes_trill_map[mnote];
                                        score_trill_tdt_map1[engravingItem] = spanner_ticks_map[__trill][1] - spanner_ticks_map[__trill][0];
                                    }
                                }
                            }
                        }
                        
                    } else if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
                        std::map<int, bool> seg_arpeggio_staffindex;
                        if (!_arpeggio_seg_checked) {
                            EngravingItem* arpeggio = item->parentItem();
                            // check Fermata
                            bool isFermataTag = false;
                            double fermata_stretch = 1.0;
                            bool isFermataAtLastSegment = false;
                            EngravingItem* arpeggioParent = arpeggio->parentItem();
                            EngravingItemList ___itemList = arpeggioParent->childrenItems(false);
                            for (size_t _k = 0; _k < ___itemList.size(); _k++) {
                                EngravingItem* ___item = ___itemList.at(_k);
                                if (___item == nullptr) {
                                    continue;
                                }
                                if (___item->type() == mu::engraving::ElementType::FERMATA) {
                                    isFermataTag = true;
                                    mu::engraving::Fermata *fermata = toFermata(___item);
                                    fermata_stretch = fermata->timeStretch();
                                    if (___item->tick().ticks() >= measure->last(mu::engraving::SegmentType::ChordRest)->tick().ticks()) {
                                        isFermataAtLastSegment = true;
                                    }
                                    chordrest_fermata_map[engravingItem] = ___item;
                                }
                            }

                            EngravingItemList _itemList = s->childrenItems(true);
                            for (size_t k = 0; k < _itemList.size(); k++) {
                                EngravingItem* _item = _itemList.at(k);
                                    if (_item == nullptr) {
                                        continue;
                                    }
                                    if (_item->type() == mu::engraving::ElementType::ARPEGGIO) {
                                        Staff* arpeggioStaff = _item->staff();
                                        int arpeggioStaffIndex = arpeggioStaff->idx();
                                        seg_arpeggio_staffindex[arpeggioStaffIndex] = true;
                                    } else if (_item->type() == mu::engraving::ElementType::NOTE) {
                                        if (_item->canvasBoundingRect().y() >= item->canvasBoundingRect().y() && _item->canvasBoundingRect().y() <= item->canvasBoundingRect().y() + item->canvasBoundingRect().height()) {
                                            Staff* arpeggioStaff = _item->staff();
                                            int arpeggioStaffIndex = arpeggioStaff->idx();
                                            seg_arpeggio_staffindex[arpeggioStaffIndex] = true;
                                        }
                                    }
                            }
                            
                            bool _arpeggio_duration_check = false;
                            bool arpeggio_whole = false;
                            int arpeggio_duration_ticks = 0;
                            if (arpeggio->type() == mu::engraving::ElementType::CHORD) {
                                mu::engraving::Chord *arpeggioChord = toChord(arpeggio);
                                arpeggio_duration_ticks = arpeggioChord->durationTypeTicks().ticks();
                                if (arpeggioChord->durationType().type() <= mu::engraving::DurationType::V_HALF) {
                                    if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_HALF) {
                                        arpeggio_duration_ticks /= 3;
                                    } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_WHOLE) {
                                        arpeggio_whole = true;
                                        arpeggio_duration_ticks /= 4;
                                    }
                                    if (isFermataTag) {
                                        if (fermata_stretch > 1) {
                                            arpeggio_duration_ticks /= fermata_stretch;
                                        } else {
                                            arpeggio_duration_ticks /= 8;
                                        }
                                    } else {
                                        arpeggio_duration_ticks /= 2;
                                    }
                                } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_QUARTER) {
                                    if (isFermataTag) {
                                        if (fermata_stretch > 1) {
                                            arpeggio_duration_ticks /= fermata_stretch;
                                        } else {
                                            arpeggio_duration_ticks /= 10;
                                        }
                                    } else {
                                        arpeggio_duration_ticks /= 4;
                                    }
                                } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_EIGHTH) {
                                    arpeggio_duration_ticks /= 2;
                                    
                                    // if (isFermataTag && isFermataAtLastSegment) {
                                    //    arpeggio_duration_ticks /= 10;
                                    // }
                                    if (isFermataTag) {
                                        if (fermata_stretch > 1) {
                                            arpeggio_duration_ticks /= fermata_stretch;
                                        } else if (isFermataAtLastSegment) {
                                            arpeggio_duration_ticks /= 10;
                                        }
                                    }
                                }
                                _arpeggio_duration_check = true;
                            }
                            

                            for (size_t k = 0; k < _itemList.size(); k++) {
                                EngravingItem* _item = _itemList.at(k);
                                if (_item == nullptr) {
                                    continue;
                                }
                                if (_item->type() == mu::engraving::ElementType::NOTE) {
                                    Staff* arpeggioStaff = _item->staff();
                                    int arpeggioStaffIndex = arpeggioStaff->idx();
                                    if (seg_arpeggio_staffindex[arpeggioStaffIndex]) {
                                        EngravingItem* _itemParent = _item->parentItem();
                                        if (_itemParent->type() == mu::engraving::ElementType::CHORD) {
                                            mu::engraving::Chord *_itemParentChord = toChord(_itemParent);
                                            Note* _note = toNote(_item);

                                            if (score_arpeggio_map.find(engravingItem) == score_arpeggio_map.end()) {
                                                score_arpeggio_map[engravingItem] = {};
                                            }

                                            if (!_arpeggio_duration_check) {
                                                mu::engraving::Chord *arpeggioChord = _note->chord();
                                                arpeggio_duration_ticks = arpeggioChord->durationTypeTicks().ticks();
                                                if (arpeggioChord->durationType().type() <= mu::engraving::DurationType::V_HALF) {
                                                    if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_HALF) {
                                                        arpeggio_duration_ticks /= 3;
                                                    } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_WHOLE) {
                                                        arpeggio_whole = true;
                                                        arpeggio_duration_ticks /= 4;
                                                    }
                                                    if (isFermataTag) {
                                                        if (fermata_stretch > 1) {
                                                            arpeggio_duration_ticks /= fermata_stretch;
                                                        } else {
                                                            arpeggio_duration_ticks /= 8;
                                                        }
                                                    } else {
                                                        arpeggio_duration_ticks /= 2;
                                                    }
                                                } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_QUARTER) {
                                                    if (isFermataTag) {
                                                        if (fermata_stretch > 1) {
                                                            arpeggio_duration_ticks /= fermata_stretch;
                                                        } else {
                                                            arpeggio_duration_ticks /= 10;
                                                        }
                                                    } else {
                                                        arpeggio_duration_ticks /= 4;
                                                    }
                                                } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_EIGHTH) {
                                                    arpeggio_duration_ticks /= 2;
                                                    // if (isFermataTag && isFermataAtLastSegment) {
                                                    //     arpeggio_duration_ticks /= 10;
                                                    // }
                                                    if (isFermataTag) {
                                                        if (fermata_stretch > 1) {
                                                            arpeggio_duration_ticks /= fermata_stretch;
                                                        } else if (isFermataAtLastSegment) {
                                                            arpeggio_duration_ticks /= 10;
                                                        }
                                                    }
                                                }
                                                for (Note* note_ : _note->chord()->notes()) {
                                                    if (std::find(score_arpeggio_map[engravingItem].begin(), score_arpeggio_map[engravingItem].end(), note_) == score_arpeggio_map[engravingItem].end()) {
                                                        score_arpeggio_map[engravingItem].push_back(note_);
                                                        score_arpeggio_st_map[engravingItem] = _itemParentChord->tick().ticks();
                                                        score_arpeggio_ot_map[engravingItem] = ottava_map[note_];

                                                        int ___arpeggio_duration_ticks = arpeggio_duration_ticks;

                                                        if (arpeggio_whole && score_arpeggio_map[engravingItem].size() >= 8) {
                                                            if (score_arpeggio_map[engravingItem].size() == 8) {
                                                                
                                                            } else if (score_arpeggio_map[engravingItem].size() >= 12) {
                                                                ___arpeggio_duration_ticks = 2.4 * arpeggio_duration_ticks;
                                                            } else if (score_arpeggio_map[engravingItem].size() >= 16) {
                                                                ___arpeggio_duration_ticks = 4 * arpeggio_duration_ticks;
                                                            }
                                                        } 
                                                        score_arpeggio_dt_map[engravingItem] = ___arpeggio_duration_ticks;
                                                    }
                                                }
                                                _arpeggio_duration_check = true;
                                            } else {
                                                if (std::find(score_arpeggio_map[engravingItem].begin(), score_arpeggio_map[engravingItem].end(), _note) == score_arpeggio_map[engravingItem].end()) {
                                                    score_arpeggio_map[engravingItem].push_back(_note);
                                                    score_arpeggio_st_map[engravingItem] = _itemParentChord->tick().ticks();
                                                    score_arpeggio_ot_map[engravingItem] = ottava_map[_note];

                                                    int ___arpeggio_duration_ticks = arpeggio_duration_ticks;

                                                    if (arpeggio_whole && score_arpeggio_map[engravingItem].size() >= 8) {
                                                        if (score_arpeggio_map[engravingItem].size() == 8) {
                                                            
                                                        } else if (score_arpeggio_map[engravingItem].size() >= 12) {
                                                            ___arpeggio_duration_ticks = 2.4 * arpeggio_duration_ticks;
                                                        } else if (score_arpeggio_map[engravingItem].size() >= 16) {
                                                            ___arpeggio_duration_ticks = 4 * arpeggio_duration_ticks;
                                                        }
                                                    } 
                                                    score_arpeggio_dt_map[engravingItem] = ___arpeggio_duration_ticks;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            _arpeggio_seg_checked = true;
                        }
                    } else if (item->type() == mu::engraving::ElementType::GLISSANDO) {
                        if (!_glissando_seg_checked) {
                            EngravingItem* glissandoNote = item->parentItem();
                            if (glissandoNote->type() != mu::engraving::ElementType::NOTE && glissandoNote->parentItem()->type() == mu::engraving::ElementType::NOTE) {
                                glissandoNote = glissandoNote->parentItem();
                            }
                            if (glissandoNote->type() == mu::engraving::ElementType::NOTE) {
                                Note* _note= toNote(glissandoNote);
                                score_glissando_startnote_map[engravingItem] = _note;
                                score_glissando_st_map[engravingItem] = glissandoNote->tick().ticks();
                                score_glissando_dt_map[engravingItem] = duration_ticks;
                                score_glissando_ot_map[engravingItem] = ottava_map[_note];

                                EngravingItemList itemList__ = measure->childrenItems(true);
                                if (score_glissando_endnotes_map.find(engravingItem) == score_glissando_endnotes_map.end()) {
                                    score_glissando_endnotes_map[engravingItem] = {};
                                }
                                for (size_t __j = 0; __j < itemList__.size(); __j++) {
                                    EngravingItem* __item__ = itemList__.at(__j);
                                    if (__item__ == nullptr) {
                                        continue;
                                    }
                                    if (__item__->type() == mu::engraving::ElementType::NOTE) {
                                        if (__item__->tick().ticks() >= glissandoNote->tick().ticks() + duration_ticks / 10) {
                                            Note* _note = toNote(__item__);
                                            if (std::find(score_glissando_endnotes_map[engravingItem].begin(), score_glissando_endnotes_map[engravingItem].end(), _note) == score_glissando_endnotes_map[engravingItem].end()) {
                                                score_glissando_endnotes_map[engravingItem].push_back(_note);
                                            }
                                        }
                                    } 
                                }
                            } 
                            _glissando_seg_checked = true;
                        }
                    } 
                }
            }
            
            mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
            while (ns && !ns->visible()) {
                ns = ns->next(mu::engraving::SegmentType::ChordRest);
            }
            s = ns;
        }
    }

    if (!pianoKeyboardPlaybackEnable) {
        return;
    }

    std::map<int, std::vector<std::map<int, ClefType>>> _clef_staff_map;
    std::map<int, std::map<int, std::map<int, ClefType>>> _clef_staff_map_extend;

    std::map<int, std::map<int, ClefType>> _score_staff_clef_map;
    std::map<int, std::vector<int>> _score_clef_index_map;

    std::map<int, std::unordered_set<ClefType>> score_clef_map;
    std::map<int, std::set<mu::engraving::Key>> score_keysig_map;

    int staff_count = 0;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ClefType); segment;) {
            std::vector<EngravingItem*> clefItemList = segment->elist();
            for (size_t i = 0; i < clefItemList.size(); i++) {
                EngravingItem* clefItem = clefItemList[i];
                if (clefItem == nullptr) {
                    continue;
                }
                Clef* clef = toClef(clefItem);
                ClefType clefType = clef->clefType();

                if (clefType == mu::engraving::ClefType::G || clefType == mu::engraving::ClefType::F 
                    || clefType == mu::engraving::ClefType::G8_VA || clefType == mu::engraving::ClefType::G15_MA 
                    || clefType == mu::engraving::ClefType::G8_VB
                    || clefType == mu::engraving::ClefType::F_8VA || clefType == mu::engraving::ClefType::F8_VB) {

                    Staff* clefStaff = clef->staff();
                    int clefStaffIndex = clefStaff->idx();
                    if (clefStaffIndex + 1 > staff_count) {
                        staff_count = clefStaffIndex + 1;
                    }

                    if (_clef_staff_map.find(clefStaffIndex) == _clef_staff_map.end()) {
                        _clef_staff_map[clefStaffIndex] = {};
                    }

                    if (_score_clef_index_map.find(clefStaffIndex) == _score_clef_index_map.end()) {
                        _score_clef_index_map[clefStaffIndex] = {};
                    }
                    if (_score_clef_index_map[clefStaffIndex].size() == 0 
                    || _score_clef_index_map[clefStaffIndex][_score_clef_index_map[clefStaffIndex].size() - 1] != measure->no()) {
                        _score_clef_index_map[clefStaffIndex].push_back(measure->no());
                        _clef_staff_map[clefStaffIndex].push_back({});
                    }
                    _clef_staff_map[clefStaffIndex][_clef_staff_map[clefStaffIndex].size() - 1].insert({ clef->tick().ticks(), clefType });
                }   
            }
            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ClefType);
            segment = next_segment;
        }
    }

    std::map<int, int> __index_map;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        std::map<int, int> traverse_measure_index_map;
        for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ChordRest); segment;) {
            std::vector<EngravingItem*> itemList = segment->elist();
            for (size_t i = 0; i < itemList.size(); i++) {
                EngravingItem* item = itemList[i];
                if (item == nullptr) {
                    continue;
                }
                Staff* staff = item->staff();
                int staffIndex = staff->idx();

                if (__index_map.find(staffIndex) == __index_map.end()) {
                    __index_map[staffIndex] = 0;
                }

                int __index = __index_map[staffIndex];

                struct Lower {
                    bool operator()(const int& a, const int& b) const {
                        return a < b; 
                    }
                };

                if (_score_clef_index_map.find(staffIndex) != _score_clef_index_map.end()) {
                    if (_score_clef_index_map[staffIndex].size() <= __index) {
                        if (traverse_measure_index_map.find(staffIndex) != traverse_measure_index_map.end()) {
                            __index = traverse_measure_index_map[staffIndex];
                        }
                    }

                    if (_score_clef_index_map[staffIndex].size() > __index) {
                        if (_clef_staff_map_extend.find(staffIndex) != _clef_staff_map_extend.end()) {
                            std::map<int, ClefType> __extend_ts_clef_map = _clef_staff_map_extend[staffIndex][measure->no()];
                            std::map<int, ClefType, Lower> _extend_ts_clef_map;
                            _extend_ts_clef_map.insert(__extend_ts_clef_map.begin(), __extend_ts_clef_map.end());

                            ClefType ___clefType = ClefType::INVALID;
                            for (const auto& [_ticks, _clefType] : _extend_ts_clef_map) {
                                if (segment->tick().ticks() >= _ticks) {
                                    ___clefType = _clefType;
                                }
                            }
                            if (___clefType != ClefType::INVALID) {
                                if (_score_staff_clef_map.find(staffIndex) == _score_staff_clef_map.end()) {
                                    _score_staff_clef_map[staffIndex] = {};
                                }
                                _score_staff_clef_map[staffIndex].insert({ segment->tick().ticks(), ___clefType });
                            }
                        }
                        if (measure->no() == _score_clef_index_map[staffIndex][__index]) {
                            int _no = _score_clef_index_map[staffIndex][__index];
                            
                            std::map<int, ClefType> __ts_clef_map = _clef_staff_map[staffIndex][__index];
                            std::map<int, ClefType, Lower> _ts_clef_map;
                            _ts_clef_map.insert(__ts_clef_map.begin(), __ts_clef_map.end());

                            ClefType ___clefType = ClefType::INVALID;
                            for (const auto& [_ticks, _clefType] : _ts_clef_map) {
                                if (segment->tick().ticks() >= _ticks) {
                                    ___clefType = _clefType;
                                } else if (segment == measure->last(mu::engraving::SegmentType::ChordRest)) {
                                    if (std::find(_score_clef_index_map[staffIndex].begin(), _score_clef_index_map[staffIndex].end(), _no + 1) == _score_clef_index_map[staffIndex].end()) {
                                        if (_clef_staff_map_extend.find(staffIndex) == _clef_staff_map_extend.end()) {
                                            _clef_staff_map_extend[staffIndex] = {};
                                        }
                                        if (_clef_staff_map_extend[staffIndex].find(_no + 1) == _clef_staff_map_extend[staffIndex].end()) {
                                            _clef_staff_map_extend[staffIndex][_no + 1] = {};
                                        }
                                        _clef_staff_map_extend[staffIndex][_no + 1].insert({ _ticks, _clefType });
                                    } else {
                                        _clef_staff_map[staffIndex][__index + 1].insert({ _ticks, _clefType });
                                    }
                                }
                            }
                            if (___clefType != ClefType::INVALID) {
                                if (_score_staff_clef_map.find(staffIndex) == _score_staff_clef_map.end()) {
                                    _score_staff_clef_map[staffIndex] = {};
                                }
                                _score_staff_clef_map[staffIndex].insert({ segment->tick().ticks(), ___clefType });
                            } else if (__index >= 1) {
                                int index__ = __index - 1;
                                std::map<int, ClefType> __ts_clef_map = _clef_staff_map[staffIndex][index__];
                                std::map<int, ClefType, Lower> _ts_clef_map;
                                _ts_clef_map.insert(__ts_clef_map.begin(), __ts_clef_map.end());

                                ClefType ___clefType___ = ClefType::INVALID;
                                for (const auto& [_ticks, _clefType] : _ts_clef_map) {
                                    if (segment->tick().ticks() >= _ticks) {
                                        ___clefType___ = _clefType;
                                    }
                                }
                                if (___clefType___ != ClefType::INVALID) {
                                    if (_score_staff_clef_map.find(staffIndex) == _score_staff_clef_map.end()) {
                                        _score_staff_clef_map[staffIndex] = {};
                                    }
                                    _score_staff_clef_map[staffIndex].insert({ segment->tick().ticks(), ___clefType___ });
                                }
                            }
                        }
                        if (measure->no() < _score_clef_index_map[staffIndex][__index] && __index >= 1) {
                            if (_score_staff_clef_map.find(staffIndex) == _score_staff_clef_map.end() 
                            || _score_staff_clef_map[staffIndex].find(segment->tick().ticks()) == _score_staff_clef_map[staffIndex].end()) {
                                int index__ = __index - 1;
                                std::map<int, ClefType> __ts_clef_map = _clef_staff_map[staffIndex][index__];
                                std::map<int, ClefType, Lower> _ts_clef_map;
                                _ts_clef_map.insert(__ts_clef_map.begin(), __ts_clef_map.end());

                                ClefType ___clefType = ClefType::INVALID;
                                for (const auto& [_ticks, _clefType] : _ts_clef_map) {
                                    if (segment->tick().ticks() >= _ticks) {
                                        ___clefType = _clefType;
                                    }
                                }
                                if (___clefType != ClefType::INVALID) {
                                    if (_score_staff_clef_map.find(staffIndex) == _score_staff_clef_map.end()) {
                                        _score_staff_clef_map[staffIndex] = {};
                                    }
                                    _score_staff_clef_map[staffIndex].insert({ segment->tick().ticks(), ___clefType });
                                }
                            }
                        }
                        if (measure->no() > _score_clef_index_map[staffIndex][__index]) {
                            traverse_measure_index_map[staffIndex] = __index;
                            __index_map[staffIndex] = __index + 1;
                        }
                    }
                }
            }
            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
            segment = next_segment;
        }
    }

    std::map<int, ClefType> stashed_staff_clef;
    std::map<int, ClefType> seg_staff_clef;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ChordRest); segment;) {
            seg_staff_clef.clear();
            std::vector<EngravingItem*> itemList = segment->elist();
            std::unordered_set<ClefType> seg_clefTypes;
            int _ticks = segment->tick().ticks();
            for (size_t i = 0; i < itemList.size(); i++) {
                EngravingItem* item = itemList[i];
                if (item == nullptr) {
                    continue;
                }
                int staffIndex = item->staff()->idx();
                if (_score_staff_clef_map.find(staffIndex) != _score_staff_clef_map.end()) {
                    if (_score_staff_clef_map[staffIndex].find(_ticks) != _score_staff_clef_map[staffIndex].end()) {
                        ClefType _clefType = _score_staff_clef_map[staffIndex][_ticks];
                        seg_clefTypes.insert(_clefType);
                        seg_staff_clef[staffIndex] = _clefType;
                        stashed_staff_clef[staffIndex] = _clefType;
                    }
                }
            }
            for (int i = 0; i < staff_count; i++) {
                if (seg_staff_clef.find(i) == seg_staff_clef.end() && stashed_staff_clef.find(i) != stashed_staff_clef.end()) {
                    seg_clefTypes.insert(stashed_staff_clef[i]);
                }
            }
            
            if (seg_clefTypes.size() > 0) {
                if (seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VA) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G);
                } 
                if (seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VA) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::G15_MA) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G8_VA);
                } 
                if (seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::F8_VB) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                    seg_clefTypes.erase(mu::engraving::ClefType::G);
                } 
                if (seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G8_VB);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend()  
                    && seg_clefTypes.find(mu::engraving::ClefType::F8_VB) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                } 
                if (seg_clefTypes.find(mu::engraving::ClefType::G8_VA) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G15_MA) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G15_MA);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::G8_VA) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::F_8VA) != seg_clefTypes.cend()
                    && seg_clefTypes.find(mu::engraving::ClefType::F8_VB) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::F_8VA) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::F8_VB) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::G) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::F_8VA) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::G);
                }
                if (seg_clefTypes.find(mu::engraving::ClefType::F) != seg_clefTypes.cend() 
                    && seg_clefTypes.find(mu::engraving::ClefType::G8_VB) != seg_clefTypes.cend()) {
                    seg_clefTypes.erase(mu::engraving::ClefType::F);
                }
                for (const auto& _clefType : seg_clefTypes) {
                    if (score_clef_map.find(_ticks) == score_clef_map.end()) {
                        score_clef_map[_ticks] = {};
                    }
                    score_clef_map[_ticks].insert(_clefType);
                }
            } 
        
            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
            segment = next_segment;
        }
    }

    std::set<mu::engraving::Key> last_keySigKeys;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        std::vector<std::set<mu::engraving::Key>> seg_keySigKeys;
        std::vector<int> seg_tag_ticks;
        for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::KeySigType); segment;) {
            std::vector<EngravingItem*> keySigItemList = segment->elist();
            seg_keySigKeys.push_back({});
            for (size_t i = 0; i < keySigItemList.size(); i++) {
                EngravingItem* keySigItem = keySigItemList[i];
                if (keySigItem == nullptr) {
                    continue;
                }
                
                if (segment->tick().ticks() == measure->tick().ticks()) {
                    mu::engraving::KeySig *keySig = toKeySig(keySigItem);
                    mu::engraving::Key key = keySig->key();
                    seg_keySigKeys[seg_keySigKeys.size() - 1].insert(key);
                } 
            }
            seg_tag_ticks.push_back(segment->tick().ticks());

            if (seg_keySigKeys[seg_keySigKeys.size() - 1].size() > 0) {
                break;
            }

            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
            segment = next_segment;
        }

        if (seg_keySigKeys.empty()) {
            mu::engraving::System* system = measure->system();
            if (system && measure == system->firstMeasure()) {
                seg_keySigKeys.push_back({});
                seg_keySigKeys[0].insert(mu::engraving::Key::C);
                last_keySigKeys.clear();
                last_keySigKeys.insert(mu::engraving::Key::C);
                seg_tag_ticks.push_back(0);
            }
        } 

        if (!seg_keySigKeys.empty() && last_keySigKeys.empty()) {
            for (const auto& _key : seg_keySigKeys[0]) {
                last_keySigKeys.insert(_key);
            }
        }

        if (seg_tag_ticks.size() == 0) {
            for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ChordRest); segment;) {
                int _ticks = segment->tick().ticks();
                for (const auto& _key : last_keySigKeys) {
                    score_keysig_map[_ticks].insert(_key);
                }
                mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
                segment = next_segment;
            }
        } else {
            int _index = 0;

            for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ChordRest); segment;) {
                int _ticks = segment->tick().ticks();
                if (_ticks < seg_tag_ticks[_index]) {
                    if (score_keysig_map.find(_ticks) == score_keysig_map.end()) {
                        score_keysig_map[_ticks] = {};
                    }
                    for (const auto& _key : last_keySigKeys) {
                        score_keysig_map[_ticks].insert(_key);
                    }
                } else {
                    if (seg_tag_ticks.size() > _index + 1) {
                        if (_ticks < seg_tag_ticks[_index + 1]) {
                            if (score_keysig_map.find(_ticks) == score_keysig_map.end()) {
                                score_keysig_map[_ticks] = {};
                            }
                            if (seg_keySigKeys[_index].size() > 0) {
                                last_keySigKeys.clear();
                            }
                            for (const auto& _key : seg_keySigKeys[_index]) {
                                score_keysig_map[_ticks].insert(_key);
                                last_keySigKeys.insert(_key);
                            }
                        } else {
                            _index += 1;
                            while (seg_tag_ticks.size() > _index + 1 && _ticks >= seg_tag_ticks[_index + 1]) {
                                _index += 1;
                            }

                            if (score_keysig_map.find(_ticks) == score_keysig_map.end()) {
                                score_keysig_map[_ticks] = {};
                            }
                            if (seg_keySigKeys[_index].size() > 0) {
                                last_keySigKeys.clear();
                            }
                            for (const auto& _key : seg_keySigKeys[_index]) {
                                score_keysig_map[_ticks].insert(_key);
                                last_keySigKeys.insert(_key);
                            }
                        }
                    } else {
                        if (score_keysig_map.find(_ticks) == score_keysig_map.end()) {
                            score_keysig_map[_ticks] = {};
                        }
                        if (seg_keySigKeys[_index].size() > 0) {
                            last_keySigKeys.clear();
                        }
                        for (const auto& _key : seg_keySigKeys[_index]) {
                            score_keysig_map[_ticks].insert(_key);
                            last_keySigKeys.insert(_key);
                        }
                    }
                }

                mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
                segment = next_segment;
            }
        }
    }
    
    for (const auto& [_ticks, _clefTypes] : score_clef_map) {
        if (score_keysig_map.find(_ticks) == score_keysig_map.end()) {
            continue;
        }
        std::set<mu::engraving::Key> _keySigKeys = score_keysig_map[_ticks];

        if (clefKeySigsKeysMap.find(_ticks) == clefKeySigsKeysMap.end()) {
            clefKeySigsKeysMap[_ticks] = {};
        }
        for (auto clefType : _clefTypes) {
            for (auto key : _keySigKeys) {
                if (clefType == mu::engraving::ClefType::G) {
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 15);
                    }
                }
                if (clefType == mu::engraving::ClefType::F) {
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 8);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 22);
                    }
                }

                if (clefType == mu::engraving::ClefType::G8_VA) { // G#8va
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 120);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 15 + 120);
                    }
                }

                if (clefType == mu::engraving::ClefType::G15_MA) { // G#15va
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 120 * 2);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 15 + 120 * 2);
                    }
                }

                if (clefType == mu::engraving::ClefType::G8_VB) { // Gb8va
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 120 * 3);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 15 + 120 * 3);
                    }
                }

                if (clefType == mu::engraving::ClefType::F_8VA) { // F#8va
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 8 + 120 * 4);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 22 + 120 * 4);
                    }
                }

                if (clefType == mu::engraving::ClefType::F8_VB) { // Fb8va
                    if ((int)key >= 0) {
                        clefKeySigsKeysMap[_ticks].insert((int)key + 8 + 120 * 5);
                    } else {
                        clefKeySigsKeysMap[_ticks].insert(-1 * (int)key + 22 + 120 * 5);
                    }
                }
            }
        }
    }

    mu::engraving::System* __system;
    bool multimeasureRestsFlag = false;
    muse::RectF multimeasureRestsStartPreMeasureRect;
    mu::engraving::System* multimeasureRestsStartSystem;
    muse::RectF _preMeasureRect = RectF(0, 0, 0, 0);
    mu::engraving::System* _preSystem;
    int multimeasureStartNo = -1;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Measure* _measure = measure; _measure; _measure = _measure->nextMeasure()) {
            EngravingItemList _itemList = _measure->childrenItems(true);
            int msTicks = _measure->tick().ticks();
            int meTicks = _measure->tick().ticks() + _measure->ticks().ticks();
            mu::engraving::System* system = _measure->system();
            if (system) {
                __system = system;
            }
            bool isNotesExist = false;
            for (size_t j = 0; j < _itemList.size(); j++) {
                EngravingItem* _item = _itemList.at(j);
                if (_item == nullptr) {
                    continue;
                }
                if (_item->type() == mu::engraving::ElementType::NOTE) {
                    isNotesExist = true;
                    break;
                }
            }
            bool isMultimeasure = false;
            for (size_t j = 0; j < _itemList.size(); j++) {
                EngravingItem* _item = _itemList.at(j);
                if (_item == nullptr) {
                    continue;
                }
                if (!isNotesExist && (_item->type() == mu::engraving::ElementType::MMREST || _item->type() == mu::engraving::ElementType::REST)) {
                    Rest* rest = toRest(_item);
                    int sTicks = rest->tick().ticks();
                    int eTicks = rest->endTick().ticks();
                    
                    if (sTicks == msTicks && eTicks == meTicks) {
                        isMultimeasure = true;
                        if (!multimeasureRestsFlag) {
                            multimeasureRestsFlag = true;
                            multimeasureRestsStartPreMeasureRect = _preMeasureRect;
                            multimeasureRestsStartSystem = _preSystem;
                            multimeasureStartNo = _measure->no();
                        }
                        
                        double y = 0;
                        if (__system) {
                            y = __system->staffYpage(0) + __system->page()->pos().y();
                        }
                        double _spatium = score->style().spatium();

                        double cursorW  = 8;
                        double h  = 6 * _spatium;
                        double y2 = 0;
                        for (size_t i = 0; i < score->nstaves(); ++i) {
                            mu::engraving::SysStaff* ss;
                            if (__system) {
                                ss = __system->staff(i);
                            } 
                            if (ss) {
                                if (!ss->show() || !score->staff(i)->show()) {
                                    continue;
                                }
                                y2 = ss->bbox().bottom();
                            }
                        }

                        h += y2;
                        y -= 3 * _spatium;
                        mnRestSTicksMap.insert({ _measure->no(), sTicks });
                        mnRestETicksMap.insert({ _measure->no(), eTicks });
                        mnRestRectFMap.insert({ _measure->no(), RectF(0, y, 0, h) });
                        mnCursorWidthMap.insert({ _measure->no(), cursorW });
                        mnSpatiumMap.insert({ _measure->no(), _spatium });
                    }
                }
            }
            _preMeasureRect = _measure->canvasBoundingRect();
            _preSystem = _measure->system();
            if (!isMultimeasure) {
                if (multimeasureRestsFlag) {
                    multimeasureRestsFlag = false;
                    int multimeasureEndNo = _measure->no() - 1;
                    int multimeasuresCount = multimeasureEndNo - multimeasureStartNo + 1;
                    
                    int _x = multimeasureRestsStartPreMeasureRect.x() + multimeasureRestsStartPreMeasureRect.width();
                    int _dwidth = _measure->canvasBoundingRect().x() - _x; 
                    if (_dwidth < 0 && multimeasureRestsStartSystem) {
                        _dwidth = multimeasureRestsStartSystem->canvasBoundingRect().x() + multimeasureRestsStartSystem->canvasBoundingRect().width() - _x;
                    }
                    for (int _no = multimeasureStartNo; _no <= multimeasureEndNo; _no++) {
                        int __x = _x + (_no - multimeasureStartNo) * _dwidth / multimeasuresCount;
                        int __y = mnRestRectFMap[_no].y();
                        int __width = _dwidth / multimeasuresCount;
                        int __height = mnRestRectFMap[_no].height();
                        mnRestRectFMap[_no] = RectF(__x, __y, __width, __height);
                    }
                    measure = _measure;
                    break;
                }
            } else {
                if (_measure == score->lastMeasure()) {
                    int multimeasureEndNo = _measure->no();
                    int multimeasuresCount = multimeasureEndNo - multimeasureStartNo + 1;
                    
                    int _x = multimeasureRestsStartPreMeasureRect.x() + multimeasureRestsStartPreMeasureRect.width();
                    int _dwidth = __system->canvasBoundingRect().x() + __system->canvasBoundingRect().width() - _x;
                    for (int _no = multimeasureStartNo; _no <= multimeasureEndNo; _no++) {
                        int __x = _x + (_no - multimeasureStartNo) * _dwidth / multimeasuresCount;
                        int __y = mnRestRectFMap[_no].y();
                        int __width = _dwidth / multimeasuresCount;
                        int __height = mnRestRectFMap[_no].height();
                        mnRestRectFMap[_no] = RectF(__x, __y, __width, __height);
                    }
                    measure = _measure;
                    break;
                }
            }
        }
    }
}

void PlaybackCursor::processCursorSpannerRenderStatus(Measure* measure, Fraction tick, bool recover, bool isPlaying) {
    if (m_cursorSpannerRenderStatusProcessFuture.valid()) {
        m_cursorSpannerRenderStatusProcessFuture.wait();
    }
    m_cursorSpannerRenderStatusProcessFuture = std::async(std::launch::async, [this, measure, tick, recover, isPlaying]() {
        processCursorSpannerRenderStatusAsync(measure, tick, recover, isPlaying);
    });
}

void PlaybackCursor::processCursorSpannerRenderStatusAsync(Measure* measure, Fraction tick, bool recover, bool isPlaying) {
    for (EngravingItem* _item : measure_spanner_map[measure->no()]) {
        int max_rollback_measures = 4;
        if (recover) {
            max_rollback_measures = 8;
        } 
        if (spanner_ticks_map.find(_item) != spanner_ticks_map.end()) {
            if (recover) {
                _item->setColor(muse::draw::Color::BLACK);
            } else {
                if (tick.ticks() >= spanner_ticks_map[_item][0] && tick.ticks() < spanner_ticks_map[_item][1]) {
                    if (isPlaying) {
                        _item->setColor(muse::draw::Color::RED);
                    }
                } else {
                    _item->setColor(muse::draw::Color::BLACK);
                }
            }
        }
        Measure* prevMeasure = measure->prevMeasure();
        max_rollback_measures -= 1;
        while (max_rollback_measures > 0 && prevMeasure) {
            for (EngravingItem* _item : measure_spanner_map[prevMeasure->no()]) {
                if (spanner_ticks_map.find(_item) != spanner_ticks_map.end()) {
                    if (recover) {
                        _item->setColor(muse::draw::Color::BLACK);
                    } else {
                        if (tick.ticks() >= spanner_ticks_map[_item][0] && tick.ticks() < spanner_ticks_map[_item][1]) {
                            if (isPlaying) {
                                _item->setColor(muse::draw::Color::RED);
                            }
                        } else {
                            _item->setColor(muse::draw::Color::BLACK);
                        }
                    }
                }
            }
            prevMeasure = prevMeasure->prevMeasure();
            max_rollback_measures -= 1;
        }
    }
}

void PlaybackCursor::processCursorNoteRenderStatus(Measure* measure, int curr_ticks) {
    if (m_cursorNoteRenderStatusProcessFuture.valid()) {
        m_cursorNoteRenderStatusProcessFuture.wait();
    }
    m_cursorNoteRenderStatusProcessFuture = std::async(std::launch::async, [this, measure, curr_ticks]() {
        processCursorNoteRenderStatusAsync(measure, curr_ticks);
    });
}

void PlaybackCursor::processCursorNoteRenderStatusAsync(Measure* measure, int curr_ticks) {
    for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ChordRest); segment;) {
        std::vector<EngravingItem*> engravingItemListOfPrevMeasure = segment->elist();
        size_t prev_len = engravingItemListOfPrevMeasure.size();
        for (size_t i = 0; i < prev_len; i++) {
            EngravingItem* engravingItem = engravingItemListOfPrevMeasure[i];
            if (engravingItem == nullptr) {
                continue;
            }
            if (chordrest_fermata_map.find(engravingItem) != chordrest_fermata_map.end()) {
                chordrest_fermata_map[engravingItem]->setColor(muse::draw::Color::BLACK);
            }
            engravingItem->setColor(muse::draw::Color::BLACK);

            EngravingItemList itemList = engravingItem->childrenItems(true);
            for (size_t j = 0; j < itemList.size(); j++) {
                EngravingItem* item = itemList.at(j);
                if (item == nullptr) {
                    continue;
                }
                
                if (item->type() == mu::engraving::ElementType::NOTE) {
                    Note *_pre_note = toNote(item);
                    if (_pre_note->isGrace()) {
                        _pre_note->setColor(muse::draw::Color::BLACK);
                    } 
                    for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                        _pre_note->dot(k)->setColor(muse::draw::Color::BLACK);
                    }
                    if (_pre_note->accidental()) {
                        _pre_note->accidental()->setColor(muse::draw::Color::BLACK);
                    }
                    if (_pre_note->chord()) {
                        if (_pre_note->chord()->articulations().size() > 0) {
                            std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                            for (auto& a : mArticulations) {
                                a->setColor(muse::draw::Color::BLACK);
                            }
                        }

                        Stem* _stem = _pre_note->chord()->stem();
                        if (_stem) {
                            _stem->setColor(muse::draw::Color::BLACK);
                        }
                        Hook* _hook = _pre_note->chord()->hook();
                        if (_hook) {
                            _hook->setColor(muse::draw::Color::BLACK);
                        }
                        Beam* _beam = _pre_note->chord()->beam();
                        if (_beam) {
                            if (curr_ticks < _beam->tick().ticks() || curr_ticks >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                _beam->setColor(muse::draw::Color::BLACK);
                            }
                        }
                    }
                }
                if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
                    item->setColor(muse::draw::Color::BLACK);
                }
            }
        }

        mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
        while (next_segment && !next_segment->visible()) {
            next_segment = next_segment->next(mu::engraving::SegmentType::ChordRest);
        }
        segment = next_segment;
    }
}

void PlaybackCursor::processCursorNoteRenderRecover(EngravingItem* engravingItem, int curr_ticks) {
    if (m_cursorNoteRenderRecoverFuture.valid()) {
        m_cursorNoteRenderRecoverFuture.wait();
    }
    m_cursorNoteRenderRecoverFuture = std::async(std::launch::async, [this, engravingItem, curr_ticks]() {
        processCursorNoteRenderRecoverAsync(engravingItem, curr_ticks);
    });
}

void PlaybackCursor::processCursorNoteRenderRecoverAsync(EngravingItem* engravingItem, int curr_ticks) {
    ChordRest *chordRest = toChordRest(engravingItem);
    int duration_ticks = chordRest->durationTypeTicks().ticks();
    if (chordrest_fermata_map.find(engravingItem) != chordrest_fermata_map.end()) {
        mu::engraving::Fermata *fermata = toFermata(chordrest_fermata_map[engravingItem]);
        double stretch = fermata->timeStretch();
        if (chordRest->tick().ticks() + duration_ticks * stretch > curr_ticks)
        if (curr_ticks < chordRest->tick().ticks() || curr_ticks >= chordRest->tick().ticks() + duration_ticks * stretch) {
            chordrest_fermata_map[engravingItem]->setColor(muse::draw::Color::BLACK);
        }
    }
    engravingItem->setColor(muse::draw::Color::BLACK);
    EngravingItemList itemList = engravingItem->childrenItems(true);
    size_t items_len = itemList.size();
    for (size_t j = 0; j < items_len; j++) {
        EngravingItem* item = itemList.at(j);
        if (item == nullptr) {
            continue;
        }
        
        if (item->type() == mu::engraving::ElementType::NOTE) {
            Note *_pre_note = toNote(item);
            // check grace
            bool is_grace = _pre_note->isGrace();
            if (is_grace) {
                _pre_note->setColor(muse::draw::Color::BLACK);
            }
            if (_pre_note->qmlDotsCount() > 0) {
                for (NoteDot* dot : _pre_note->dots()) {
                    dot->setColor(muse::draw::Color::BLACK);
                }
            }
            if (_pre_note->accidental()) {
                _pre_note->accidental()->setColor(muse::draw::Color::BLACK);
            }

            if (_pre_note->chord()) {
                if (_pre_note->chord()->articulations().size() > 0) {
                    std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                    for (auto& a : mArticulations) {
                        a->setColor(muse::draw::Color::BLACK);
                    }
                }

                Stem* _stem = _pre_note->chord()->stem();
                if (_stem) {
                    _stem->setColor(muse::draw::Color::BLACK);
                }
                Hook* _hook = _pre_note->chord()->hook();
                if (_hook) {
                    _hook->setColor(muse::draw::Color::BLACK);
                }
                Beam* _beam = _pre_note->chord()->beam();
                if (_beam) {
                    if (curr_ticks < _beam->tick().ticks() || curr_ticks >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                        _beam->setColor(muse::draw::Color::BLACK);
                    }
                }
            }
        }
        if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
            item->setColor(muse::draw::Color::BLACK);
        }
    }
}

muse::RectF PlaybackCursor::resolveCursorRectByTick1(muse::midi::tick_t _tick, bool isPlaying) {
    Fraction tick = Fraction::fromTicks(_tick);
    if (!m_notation) {
        return RectF();
    }

    mu::engraving::Score* score = m_notation->elements()->msScore();
    if (!preProcessScore) {
        processOttava(score, isPlaying);
    }

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        for (const auto& pair : mnRestSTicksMap) {
            if (tick.ticks() >= pair.second && tick.ticks() < mnRestETicksMap[pair.first]) {
                muse::RectF mRect = mnRestRectFMap[pair.first];
                double dt = mnRestETicksMap[pair.first] - pair.second;
                double x = mRect.x() + mRect.width() * (tick.ticks() - pair.second) / dt;
                return RectF(x, mRect.y(), mnCursorWidthMap[pair.first], mRect.height());
            }
        }
        return RectF();
    }

    mu::engraving::System* system = measure->system();
    if (!system) {
        for (const auto& pair : mnRestSTicksMap) {
            if (tick.ticks() >= pair.second && tick.ticks() < mnRestETicksMap[pair.first]) {
                muse::RectF mRect = mnRestRectFMap[pair.first];
                double dt = mnRestETicksMap[pair.first] - pair.second;
                double x = mRect.x() + mRect.width() * (tick.ticks() - pair.second) / dt;
                return RectF(x, mRect.y(), mnCursorWidthMap[pair.first], mRect.height());
            }
        }
        return RectF();
    }

    processCursorSpannerRenderStatus(measure, tick, false, isPlaying);

    qreal x = 0.0;
    mu::engraving::Segment* s = nullptr;
    mu::engraving::Segment* s1 = nullptr;
    m_notation->interaction()->clearPlaybackNotes();
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        s1 = s;
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0.0;
        Fraction t2;

        // alex:: check note ticks and duration, compare with current ticks
        if (isPlaying) {
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
                    processCursorNoteRenderRecover(engravingItem, tick.ticks());
                } else {
                    if (pianoKeyboardPlaybackEnable) {
                        if (s->tick().ticks() != curr_seg_ticks) {
                            curr_seg_ticks = s->tick().ticks();
                            if (clefKeySigsKeysMap.find(curr_seg_ticks) != clefKeySigsKeysMap.end()) {
                                if (clefKeySigsKeysMap[curr_seg_ticks].size() > 0) {
                                    m_notation->interaction()->addClefKeySigsKeysSet(clefKeySigsKeysMap[curr_seg_ticks]);
                                    m_notation->interaction()->notifyClefKeySigsKeysChange();    
                                }
                            }
                        }
                    }
                    if (chordrest_fermata_map.find(engravingItem) != chordrest_fermata_map.end()) {
                        chordrest_fermata_map[engravingItem]->setColor(muse::draw::Color::RED);
                    }
                    engravingItem->setColor(muse::draw::Color::RED);

                    if (pianoKeyboardPlaybackEnable) {
                        if (score_trill_map[engravingItem]) {
                            m_notation->interaction()->addTrillNote(score_trill_map[engravingItem], score_trill_type_map[engravingItem], score_trill_st_map[engravingItem], 
                                score_trill_dt_map[engravingItem], score_trill_tdt_map[engravingItem], score_trill_tt_map[engravingItem], score_trill_ot_map[engravingItem], 
                                score_trill_tie_map[score_trill_map[engravingItem]]);
                            m_notation->interaction()->trillNoteUpdate();
                        }
                        if (score_trill_map1[engravingItem]) {
                            m_notation->interaction()->addTrillNote1(score_trill_map1[engravingItem], score_trill_type_map1[engravingItem], score_trill_st_map1[engravingItem], 
                                score_trill_dt_map1[engravingItem], score_trill_tdt_map1[engravingItem], score_trill_tt_map1[engravingItem], score_trill_ot_map1[engravingItem], 
                                score_trill_tie_map1[score_trill_map1[engravingItem]]);
                            m_notation->interaction()->trillNoteUpdate1();
                        }

                        if (score_arpeggio_map.find(engravingItem) != score_arpeggio_map.end()) {
                            m_notation->interaction()->addArpeggioNotes(score_arpeggio_map[engravingItem], score_arpeggio_st_map[engravingItem], score_arpeggio_dt_map[engravingItem], score_arpeggio_ot_map[engravingItem]);
                            m_notation->interaction()->arpeggioNotesUpdate(false);
                        }

                        if (score_glissando_endnotes_map.find(engravingItem) != score_glissando_endnotes_map.end()) {
                            m_notation->interaction()->addGlissandoNote(score_glissando_startnote_map[engravingItem], 
                                        score_glissando_st_map[engravingItem], score_glissando_dt_map[engravingItem], 
                                        score_glissando_ot_map[engravingItem]);
                            std::vector<Note*> _endNotes = score_glissando_endnotes_map[engravingItem];
                            for (Note* _note : _endNotes) {
                                m_notation->interaction()->addGlissandoEndNote(_note, ottava_map[_note]);
                            }
                            m_notation->interaction()->glissandoEndNotesUpdate();
                        }
                    }

                    EngravingItemList itemList = engravingItem->childrenItems(true);
                    size_t items_len = itemList.size();
                    for (size_t j = 0; j < items_len; j++) {
                        EngravingItem* item = itemList.at(j);
                        if (item == nullptr) {
                            continue;
                        }
                        if (item->type() == mu::engraving::ElementType::NOTE) {
                            Note* _pre_note = toNote(item);
                            bool note_hit_ts = false;
                            int ticks_dis = tick.ticks() - _pre_note->tick().ticks();
                            if (_pre_note->chord()) {
                                int note_dt = _pre_note->chord()->durationTypeTicks().ticks();
                                if (_pre_note->chord()->durationType().type() <= mu::engraving::DurationType::V_WHOLE) {
                                    note_dt /= 4;
                                }
                                if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_HALF) {
                                    note_dt /= 2;
                                }
                                if (_pre_note->chord()->durationType().type() >= mu::engraving::DurationType::V_EIGHTH 
                                && _pre_note->chord()->durationType().type() <= mu::engraving::DurationType::V_1024TH) {
                                    note_dt *= 4;
                                }
                                if (ticks_dis >= 0 && ticks_dis <= note_dt / 8) {
                                    note_hit_ts = true;
                                }
                            }
                            
                            int _pre_note_ottavaType = ottava_map[_pre_note];  
                            // check grace
                            bool is_grace = _pre_note->isGrace();
                            if (!is_grace) {
                                std::vector<Chord*>& _graceChords = _pre_note->chord()->graceNotes();

                                size_t gracechords_size = _graceChords.size();
                                if (gracechords_size > 0) {
                                    bool grace_before = true;
                                    if (_graceChords[0]->canvasPos().x() > _pre_note->canvasPos().x()) {
                                        grace_before = false;
                                    }
                                    int grace_duration_ticks = _pre_note->chord()->durationTypeTicks().ticks();
                                    if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_WHOLE) {
                                        grace_duration_ticks /= 16;
                                    } else if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_HALF) {
                                        grace_duration_ticks /= 8;
                                    } else if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_QUARTER) {
                                        grace_duration_ticks /= 4;
                                    } else if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_EIGHTH) {
                                        grace_duration_ticks /= 2;
                                    } else if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_16TH) {
                                        grace_duration_ticks /= 2;
                                    } else if (_pre_note->chord()->durationType().type() == mu::engraving::DurationType::V_32ND) {
                                        grace_duration_ticks /= 2;
                                    } else if (_pre_note->chord()->durationType().type() >= mu::engraving::DurationType::V_64TH) {
                                        grace_duration_ticks /= 2;
                                    } 
                                    
                                    std::vector<Chord*> graceChords;
                                    for (auto& _g : _graceChords) {
                                        graceChords.push_back(_g);
                                    }
                                    std::sort(graceChords.begin(), graceChords.end(), compare_by_chord_x);
                                    int single_grace_duration_ticks = grace_duration_ticks / gracechords_size;
                                    if (grace_before) {
                                        if (ticks_dis < grace_duration_ticks) {
                                            for (size_t grace_i = 0; grace_i < gracechords_size; ++grace_i) {
                                                if (ticks_dis >= single_grace_duration_ticks * grace_i && ticks_dis <= single_grace_duration_ticks * (grace_i + 1)) {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::RED);
                                                    for (Note* choreNote : graceChords[grace_i]->notes()) {
                                                        if (choreNote->accidental()) {
                                                            choreNote->accidental()->setColor(muse::draw::Color::RED);
                                                        }
                                                    }
                                                    Stem* _stem = graceChords[grace_i]->stem();
                                                    if (_stem) {
                                                        _stem->setColor(muse::draw::Color::RED);
                                                    }
                                                    Hook* _hook = graceChords[grace_i]->hook();
                                                    if (_hook) {
                                                        _hook->setColor(muse::draw::Color::RED);
                                                    }
                                                    Beam* _beam = graceChords[grace_i]->beam();
                                                    if (_beam) {
                                                        _beam->setColor(muse::draw::Color::RED);
                                                    }
                                                    if (pianoKeyboardPlaybackEnable) {
                                                        std::vector<Note*> _notesList = graceChords[grace_i]->notes();
                                                        for (Note* __note__ : _notesList) {
                                                            int __note__ottavaType = ottava_map[__note__];
                                                            m_notation->interaction()->addPlaybackNote(__note__, __note__ottavaType, false);
                                                        }
                                                    }
                                                } else {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::BLACK);
                                                    for (Note* choreNote : graceChords[grace_i]->notes()) {
                                                        if (choreNote->accidental()) {
                                                            choreNote->accidental()->setColor(muse::draw::Color::BLACK);
                                                        }
                                                    }
                                                    Stem* _stem = graceChords[grace_i]->stem();
                                                    if (_stem) {
                                                        _stem->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Hook* _hook = graceChords[grace_i]->hook();
                                                    if (_hook) {
                                                        _hook->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Beam* _beam = graceChords[grace_i]->beam();
                                                    if (_beam) {
                                                        if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                            _beam->setColor(muse::draw::Color::BLACK);
                                                        }
                                                    }
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::BLACK);
                                            for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                                                _pre_note->dot(k)->setColor(muse::draw::Color::BLACK);
                                            }
                                            if (_pre_note->accidental()) {
                                                _pre_note->accidental()->setColor(muse::draw::Color::BLACK);
                                            }
                                            if (_pre_note->chord()) {
                                                if (_pre_note->chord()->articulations().size() > 0) {
                                                    std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                                    for (auto& a : mArticulations) {
                                                        a->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                                Stem* _stem = _pre_note->chord()->stem();
                                                if (_stem) {
                                                    _stem->setColor(muse::draw::Color::BLACK);
                                                }
                                                Hook* _hook = _pre_note->chord()->hook();
                                                if (_hook) {
                                                    _hook->setColor(muse::draw::Color::BLACK);
                                                }
                                                Beam* _beam = _pre_note->chord()->beam();
                                                if (_beam) {
                                                    if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                        _beam->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                            }
                                        } else {
                                            for (Chord *_chord : _graceChords) {
                                                _chord->setColor(muse::draw::Color::BLACK);
                                                for (Note* choreNote : _chord->notes()) {
                                                    if (choreNote->accidental()) {
                                                        choreNote->accidental()->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                                Stem* _stem = _chord->stem();
                                                if (_stem) {
                                                    _stem->setColor(muse::draw::Color::BLACK);
                                                }
                                                Hook* _hook = _chord->hook();
                                                if (_hook) {
                                                    _hook->setColor(muse::draw::Color::BLACK);
                                                }
                                                Beam* _beam = _chord->beam();
                                                if (_beam) {
                                                    if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                        _beam->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::RED);
                                            for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                                                _pre_note->dot(k)->setColor(muse::draw::Color::RED);
                                            }
                                            if (_pre_note->accidental()) {
                                                _pre_note->accidental()->setColor(muse::draw::Color::RED);
                                            }
                                            if (_pre_note->chord()) {
                                                if (_pre_note->chord()->articulations().size() > 0) {
                                                    std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                                    for (auto& a : mArticulations) {
                                                        a->setColor(muse::draw::Color::RED);
                                                    }
                                                }

                                                Stem* _stem = _pre_note->chord()->stem();
                                                if (_stem) {
                                                    _stem->setColor(muse::draw::Color::RED);
                                                }
                                                Hook* _hook = _pre_note->chord()->hook();
                                                if (_hook) {
                                                    _hook->setColor(muse::draw::Color::RED);
                                                }
                                                Beam* _beam = _pre_note->chord()->beam();
                                                if (_beam) {
                                                    _beam->setColor(muse::draw::Color::RED);
                                                }
                                            }
                                            if (pianoKeyboardPlaybackEnable) {
                                                m_notation->interaction()->addPlaybackNote(_pre_note, _pre_note_ottavaType, note_hit_ts);
                                            }
                                        }
                                    } else {
                                        int _pre_note_duration_ticks = _pre_note->chord()->durationTypeTicks().ticks();
                                        if (ticks_dis + grace_duration_ticks > _pre_note_duration_ticks) {
                                            for (size_t grace_i = 0; grace_i < gracechords_size; ++grace_i) {
                                                if (ticks_dis >= _pre_note_duration_ticks - single_grace_duration_ticks * (gracechords_size - grace_i) && ticks_dis <= _pre_note_duration_ticks - single_grace_duration_ticks * (gracechords_size - grace_i - 1)) {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::RED);
                                                    for (Note* choreNote : graceChords[grace_i]->notes()) {
                                                        if (choreNote->accidental()) {
                                                            choreNote->accidental()->setColor(muse::draw::Color::RED);
                                                        }
                                                    }
                                                    Stem* _stem = graceChords[grace_i]->stem();
                                                    if (_stem) {
                                                        _stem->setColor(muse::draw::Color::RED);
                                                    }
                                                    Hook* _hook = graceChords[grace_i]->hook();
                                                    if (_hook) {
                                                        _hook->setColor(muse::draw::Color::RED);
                                                    }
                                                    Beam* _beam = graceChords[grace_i]->beam();
                                                    if (_beam) {
                                                        _beam->setColor(muse::draw::Color::RED);
                                                    }
                                                    if (pianoKeyboardPlaybackEnable) {
                                                        for (Note* _note_item : graceChords[grace_i]->notes()) {
                                                            int _note_item_ottavaType = ottava_map[_note_item];
                                                            m_notation->interaction()->addPlaybackNote(_note_item, _note_item_ottavaType, false);
                                                        }
                                                    }
                                                } else {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::BLACK);
                                                    for (Note* choreNote : graceChords[grace_i]->notes()) {
                                                        if (choreNote->accidental()) {
                                                            choreNote->accidental()->setColor(muse::draw::Color::BLACK);
                                                        }
                                                    }
                                                    Stem* _stem = graceChords[grace_i]->stem();
                                                    if (_stem) {
                                                        _stem->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Hook* _hook = graceChords[grace_i]->hook();
                                                    if (_hook) {
                                                        _hook->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Beam* _beam = graceChords[grace_i]->beam();
                                                    if (_beam) {
                                                        if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                            _beam->setColor(muse::draw::Color::BLACK);
                                                        }
                                                    }
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::BLACK);
                                            for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                                                _pre_note->dot(k)->setColor(muse::draw::Color::BLACK);
                                            }
                                            if (_pre_note->accidental()) {
                                                _pre_note->accidental()->setColor(muse::draw::Color::BLACK);
                                            }
                                            if (_pre_note->chord()) {
                                                if (_pre_note->chord()->articulations().size() > 0) {
                                                    std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                                    for (auto& a : mArticulations) {
                                                        a->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Stem* _stem = _pre_note->chord()->stem();
                                                    if (_stem) {
                                                        _stem->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Hook* _hook = _pre_note->chord()->hook();
                                                    if (_hook) {
                                                        _hook->setColor(muse::draw::Color::BLACK);
                                                    }
                                                    Beam* _beam = _pre_note->chord()->beam();
                                                    if (_beam) {
                                                        if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                            _beam->setColor(muse::draw::Color::BLACK);
                                                        }
                                                    }
                                                }
                                            }
                                        } else {
                                            for (Chord *_chord : _graceChords) {
                                                _chord->setColor(muse::draw::Color::BLACK);
                                                for (Note* choreNote : _chord->notes()) {
                                                    if (choreNote->accidental()) {
                                                        choreNote->accidental()->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                                Stem* _stem = _chord->stem();
                                                if (_stem) {
                                                    _stem->setColor(muse::draw::Color::BLACK);
                                                }
                                                Hook* _hook = _chord->hook();
                                                if (_hook) {
                                                    _hook->setColor(muse::draw::Color::BLACK);
                                                }
                                                Beam* _beam = _chord->beam();
                                                if (_beam) {
                                                    if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                                        _beam->setColor(muse::draw::Color::BLACK);
                                                    }
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::RED);
                                            for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                                                _pre_note->dot(k)->setColor(muse::draw::Color::RED);
                                            }
                                            if (_pre_note->accidental()) {
                                                _pre_note->accidental()->setColor(muse::draw::Color::RED);
                                            }
                                            if (_pre_note->chord()) {
                                                if (_pre_note->chord()->articulations().size() > 0) {
                                                    std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                                    for (auto& a : mArticulations) {
                                                        a->setColor(muse::draw::Color::RED);
                                                    }
                                                }

                                                Stem* _stem = _pre_note->chord()->stem();
                                                if (_stem) {
                                                    _stem->setColor(muse::draw::Color::RED);
                                                }
                                                Hook* _hook = _pre_note->chord()->hook();
                                                if (_hook) {
                                                    _hook->setColor(muse::draw::Color::RED);
                                                }
                                                Beam* _beam = _pre_note->chord()->beam();
                                                if (_beam) {
                                                    _beam->setColor(muse::draw::Color::RED);
                                                }
                                            }
                                            if (pianoKeyboardPlaybackEnable) {
                                                m_notation->interaction()->addPlaybackNote(_pre_note, _pre_note_ottavaType, note_hit_ts);
                                            }
                                        }
                                    }
                                } else {
                                    for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                                        _pre_note->dot(k)->setColor(muse::draw::Color::RED);
                                    }
                                    if (_pre_note->accidental()) {
                                        _pre_note->accidental()->setColor(muse::draw::Color::RED);
                                    }
                                    
                                    if (_pre_note->chord()) {
                                        if (_pre_note->chord()->articulations().size() > 0) {
                                            std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                            for (auto& a : mArticulations) {
                                                a->setColor(muse::draw::Color::RED);
                                            }
                                        }
                                        Stem* _stem = _pre_note->chord()->stem();
                                        if (_stem) {
                                            _stem->setColor(muse::draw::Color::RED);
                                        }

                                        Hook* _hook = _pre_note->chord()->hook();
                                        if (_hook) {
                                            _hook->setColor(muse::draw::Color::RED);
                                        }

                                        Beam* _beam = _pre_note->chord()->beam();
                                        if (_beam) {
                                            _beam->setColor(muse::draw::Color::RED);
                                        }
                                    }
                                    if (pianoKeyboardPlaybackEnable) {
                                        m_notation->interaction()->addPlaybackNote(_pre_note, ottava_map[_pre_note], note_hit_ts);
                                    }
                                }
                            }
                        } 

                        if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
                            item->setColor(muse::draw::Color::RED);
                        }
                    }

                }
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

    // muse::RectF measureRect = measure->pageBoundingRect();
    int measureNo = measure->no();

    if (hit_measure_no() != measureNo || hit_measure() != measure) {
        Measure* prevMeasure = measure->prevMeasure();
        if (prevMeasure) {
            processCursorNoteRenderStatus(prevMeasure, tick.ticks());
            if (hit_measure() != nullptr && prevMeasure != hit_measure()) {
                processCursorNoteRenderStatus(hit_measure(), tick.ticks());
                processCursorSpannerRenderStatus(hit_measure(), tick, false, isPlaying);
            }
        }
        setHitMeasureNo(measureNo);
        setHitMeasure(measure);
    }
    
    if (pianoKeyboardPlaybackEnable) {
        m_notation->interaction()->notifyPianoKeyboardNotesChanged();
    }

    if (tick.ticks() == 0 && !isPlaying) {
        for (mu::engraving::Segment* segment = score->lastMeasure()->first(mu::engraving::SegmentType::ChordRest); segment;) {
            std::vector<EngravingItem*> engravingItemListOfHitMeasure = segment->elist();
            size_t hit_len = engravingItemListOfHitMeasure.size();
            for (size_t i = 0; i < hit_len; i++) {
                EngravingItem* engravingItem = engravingItemListOfHitMeasure[i];
                if (engravingItem == nullptr) {
                    continue;
                }
                if (chordrest_fermata_map.find(engravingItem) != chordrest_fermata_map.end()) {
                    chordrest_fermata_map[engravingItem]->setColor(muse::draw::Color::BLACK);
                }
                engravingItem->setColor(muse::draw::Color::BLACK);

                EngravingItemList itemList = engravingItem->childrenItems(true);
                for (size_t j = 0; j < itemList.size(); j++) {
                    EngravingItem* item = itemList.at(j);
                    if (item == nullptr) {
                        continue;
                    }
                    
                    if (item->type() == mu::engraving::ElementType::NOTE) {
                        Note *_pre_note = toNote(item);
                        if (_pre_note->isGrace()) {
                            _pre_note->setColor(muse::draw::Color::BLACK);
                        }
                        for (int k = 0; k < _pre_note->qmlDotsCount(); k++) {
                            _pre_note->dot(k)->setColor(muse::draw::Color::BLACK);
                        }
                        if (_pre_note->accidental()) {
                            _pre_note->accidental()->setColor(muse::draw::Color::BLACK);
                        }
                        if (_pre_note->chord()) {
                            if (_pre_note->chord()->articulations().size() > 0) {
                                std::vector<Articulation*> mArticulations = _pre_note->chord()->articulations();
                                for (auto& a : mArticulations) {
                                    a->setColor(muse::draw::Color::BLACK);
                                }
                            }

                            Stem* _stem = _pre_note->chord()->stem();
                            if (_stem) {
                                _stem->setColor(muse::draw::Color::BLACK);
                            }
                            Hook* _hook = _pre_note->chord()->hook();
                            if (_hook) {
                                _hook->setColor(muse::draw::Color::BLACK);
                            }
                            Beam* _beam = _pre_note->chord()->beam();
                            if (_beam) {
                                if (tick.ticks() < _beam->tick().ticks() || tick.ticks() >= _beam->tick().ticks() + _beam->ticks().ticks()) {
                                    _beam->setColor(muse::draw::Color::BLACK);
                                }
                            }
                        }
                    }
                    if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
                        item->setColor(muse::draw::Color::BLACK);
                    }
                }
            }

            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ChordRest);
            while (next_segment && !next_segment->visible()) {
                next_segment = next_segment->next(mu::engraving::SegmentType::ChordRest);
            }
            segment = next_segment;
        }

        processCursorSpannerRenderStatus(hit_measure(), tick, true, isPlaying);
    }
    if (pianoKeyboardPlaybackEnable) {
        m_notation->interaction()->arpeggioTick(tick.ticks());
        m_notation->interaction()->trillTick(tick.ticks());
        m_notation->interaction()->trillTick1(tick.ticks());
        m_notation->interaction()->glissandoTick(tick.ticks());
        m_notation->interaction()->lastMeasure(measure == score->lastMeasure());
    }

    // if (measureNo < 2) {
    //     emit lingeringCursorUpdate(0.0, measureRect.y(), measureRect.width(), measureRect.height());
    // } else {
    //     emit lingeringCursorUpdate(measureRect.x(), measureRect.y(), measureRect.width(), measureRect.height());
    // }
    emit lingeringCursorUpdate1();

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

    m_adjust_nm_rect = false;

    Measure* next_measure = measure->nextMeasure();
    if (next_measure && next_measure->no() != m_nm_no) {
        mu::engraving::System* nm_system = next_measure->system();
        if (nm_system != system && next_measure->canvasPos().y() > measure->canvasPos().y()) {
            double nm_y = nm_system->staffYpage(0) + nm_system->page()->pos().y();

            double nm_y2 = 0.0;
            double nm_h  = 6 * _spatium;
            
            for (size_t i = 0; i < score->nstaves(); ++i) {
                mu::engraving::SysStaff* nm_ss = nm_system->staff(i);
                if (!nm_ss->show() || !score->staff(i)->show()) {
                    continue;
                }
                nm_y2 = nm_ss->bbox().bottom();
            }

            mu::engraving::Segment* nm_s = next_measure->first();
            double nm_x = nm_s->canvasPos().x();
            nm_h += nm_y2;
            nm_x -= _spatium;
            nm_y -= 3 * _spatium;

            m_nm_rect = RectF(nm_x, nm_y, w, nm_h);
            m_adjust_nm_rect = true;
            m_nm_no = next_measure->no();
        }
    }

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

const bool PlaybackCursor::adjust_nm_rect() const
{
    return m_adjust_nm_rect;
}

const muse::RectF& PlaybackCursor::nm_rect() const
{
    return m_nm_rect;
}

QColor PlaybackCursor::color() const
{
    return configuration()->playbackCursorColor();
}

// alex::
int PlaybackCursor::hit_measure_no() 
{ 
    return m_hit_measure_no; 
}
Measure* PlaybackCursor::hit_measure() 
{
    return m_hit_measure;
}
void PlaybackCursor::setHitMeasureNo(int m_no) 
{ 
    m_hit_measure_no = m_no; 
}
void PlaybackCursor::setHitMeasure(Measure* m) 
{
    m_hit_measure = m;
}
