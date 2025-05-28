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

void PlaybackCursor::move(muse::midi::tick_t tick, bool isPlaying)
{
    // LOGALEX();
    // m_rect = resolveCursorRectByTick(tick);
    m_rect = resolveCursorRectByTick(tick, isPlaying);
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
muse::RectF PlaybackCursor::resolveCursorRectByTick(muse::midi::tick_t _tick, bool isPlaying) {
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

    if (measure != m_hit_measure) {
        curr_measure_trill_notes.clear();
        tremolo_type_map.clear();
        tremolo_half_map.clear();
        EngravingItemList measure_children = measure->childrenItems(true);
        bool hasTremoloBar = false;
        for (size_t m_k = 0; m_k < measure_children.size(); m_k++) {
            EngravingItem* measure_item = measure_children.at(m_k);
            if (measure_item && measure_item->isOrnament()) {
                Ornament* orn = toOrnament(measure_item); // subtype 2214(trill)
                Trill* _trill = toTrill(measure_item);
                if (_trill) {
                    Note* note = orn->noteAbove();
                    if (note) {
                        curr_measure_trill_notes.insert(note);
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
                    tremolo_type_map[_tremoloNote] = (int)_tremoloType;

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
                        curr_measure_trill_notes.insert(leftNote);
                        
                        Note* rightFirstNote = chord2->notes()[0];
                        Note* rightLastNote = chord2->notes()[chord2->notes().size() - 1];
                        Note* rightNote = rightFirstNote;
                        if (rightFirstNote->canvasPos().y() < rightLastNote->canvasPos().y()) {
                            rightNote = rightLastNote;
                        }
                        curr_measure_trill_notes.insert(rightNote);
                        tremolo_half_map[leftNote] = true;
                        tremolo_half_map[rightNote] = true;
                    }
                    if (_tremoloSingleChord) {
                        curr_measure_trill_notes.insert(_tremoloNote->chord()->notes()[_tremoloNote->chord()->notes().size() - 1]); 
                    }
                }
            }
        }
    }

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
            bool curr_trill_note_checked = false;
            bool curr_trill_note1_checked = false;
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
                        }
                    }
                } else {
                    engravingItem->setColor(muse::draw::Color::RED);
                    EngravingItemList itemList = engravingItem->childrenItems(true);
                    size_t items_len = itemList.size();
                    for (size_t j = 0; j < items_len; j++) {
                        EngravingItem* item = itemList.at(j);
                        if (item == nullptr) {
                            continue;
                        }
                        if (item->type() == mu::engraving::ElementType::NOTE) {
                            // m_notation->interaction()->addPlaybackNote(toNote(item));
                            Note *_pre_note = toNote(item);
                            for (Note* mnote : curr_measure_trill_notes) {
                                if (mnote == _pre_note) {
                                    if (!curr_trill_note_checked) {
                                        if (curr_trill_note != _pre_note) {
                                            curr_trill_note_checked = true;
                                            curr_trill_note = _pre_note;
                                            int logic_tremoloType = 0;
                                            int _duration_ticks = duration_ticks;
                                            if (tremolo_type_map.find(curr_trill_note) != tremolo_type_map.end()) {
                                                if (tremolo_half_map[curr_trill_note]) {
                                                    _duration_ticks /= 2;
                                                }
                                                int _tremoloType = tremolo_type_map[curr_trill_note];
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
                                            m_notation->interaction()->addTrillNote(curr_trill_note, curr_trill_note->tick().ticks(), _duration_ticks, logic_tremoloType);
                                            m_notation->interaction()->trillNoteUpdate();
                                        }
                                    } else if (curr_trill_note_checked && !curr_trill_note1_checked) {
                                        if (curr_trill_note1 != _pre_note) {
                                            curr_trill_note1_checked = true;
                                            curr_trill_note1 = _pre_note;
                                            int logic_tremoloType = 0;
                                            int _duration_ticks = duration_ticks;
                                            if (tremolo_type_map.find(curr_trill_note1) != tremolo_type_map.end()) {
                                                if (tremolo_half_map[curr_trill_note1]) {
                                                    _duration_ticks /= 2;
                                                }
                                                int _tremoloType = tremolo_type_map[curr_trill_note1];
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
                                            m_notation->interaction()->addTrillNote1(curr_trill_note1, curr_trill_note1->tick().ticks(), _duration_ticks, logic_tremoloType);
                                            m_notation->interaction()->trillNoteUpdate1();
                                        }
                                    }
                                }
                            }
                        
                            // check grace
                            bool is_grace = _pre_note->isGrace();
                            if (is_grace) {
                                
                            } else {
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
                                    int ticks_dis = tick.ticks() - _pre_note->tick().ticks();
                                    int single_grace_duration_ticks = grace_duration_ticks / gracechords_size;
                                    if (grace_before) {
                                        if (ticks_dis < grace_duration_ticks) {
                                            for (size_t grace_i = 0; grace_i < gracechords_size; ++grace_i) {
                                                if (ticks_dis >= single_grace_duration_ticks * grace_i && ticks_dis <= single_grace_duration_ticks * (grace_i + 1)) {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::RED);
                                                    EngravingItemList pre_notesList = graceChords[grace_i]->childrenItems(false);
                                                    size_t pre_notes_len = pre_notesList.size();
                                                    for (size_t pre_j = 0; pre_j < pre_notes_len; pre_j++) {
                                                        EngravingItem* pre_note_item = pre_notesList.at(pre_j);
                                                        if (pre_note_item->type() == mu::engraving::ElementType::NOTE) {
                                                            m_notation->interaction()->addPlaybackNote(toNote(pre_note_item));
                                                        }
                                                    }
                                                } else {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::BLACK);
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::BLACK);
                                        } else {
                                            for (Chord *_chord : _graceChords) {
                                                _chord->setColor(muse::draw::Color::BLACK);
                                            }
                                            _pre_note->setColor(muse::draw::Color::RED);
                                            m_notation->interaction()->addPlaybackNote(_pre_note);
                                        }
                                    } else {
                                        int _pre_note_duration_ticks = _pre_note->chord()->durationTypeTicks().ticks();
                                        if (ticks_dis + grace_duration_ticks > _pre_note_duration_ticks) {
                                            for (size_t grace_i = 0; grace_i < gracechords_size; ++grace_i) {
                                                if (ticks_dis >= _pre_note_duration_ticks - single_grace_duration_ticks * (gracechords_size - grace_i) && ticks_dis <= _pre_note_duration_ticks - single_grace_duration_ticks * (gracechords_size - grace_i - 1)) {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::RED);
                                                    EngravingItemList pre_notesList = graceChords[grace_i]->childrenItems(false);
                                                    size_t pre_notes_len = pre_notesList.size();
                                                    for (size_t pre_j = 0; pre_j < pre_notes_len; pre_j++) {
                                                        EngravingItem* pre_note_item = pre_notesList.at(pre_j);
                                                        if (pre_note_item->type() == mu::engraving::ElementType::NOTE) {
                                                            m_notation->interaction()->addPlaybackNote(toNote(pre_note_item));
                                                        }
                                                    }
                                                } else {
                                                    graceChords[grace_i]->setColor(muse::draw::Color::BLACK);
                                                }
                                            }
                                            _pre_note->setColor(muse::draw::Color::BLACK);
                                        } else {
                                            for (Chord *_chord : _graceChords) {
                                                _chord->setColor(muse::draw::Color::BLACK);
                                            }
                                            _pre_note->setColor(muse::draw::Color::RED);
                                            m_notation->interaction()->addPlaybackNote(_pre_note);
                                        }
                                    }
                                    
                                } else {
                                    m_notation->interaction()->addPlaybackNote(toNote(item));
                                }
                            }
                            
                        } else if (item->type() == mu::engraving::ElementType::GLISSANDO) {
                            EngravingItem* glissandoNote = item->parentItem();
                            if (glissandoNote->type() != mu::engraving::ElementType::NOTE && glissandoNote->parentItem()->type() == mu::engraving::ElementType::NOTE) {
                                glissandoNote = glissandoNote->parentItem();
                            }
                            if (glissandoNote->type() == mu::engraving::ElementType::NOTE) {
                                if (tick.ticks() < m_notation->interaction()->glissandoNoteTicks() 
                                || tick.ticks() > m_notation->interaction()->glissandoNoteTicks() + m_notation->interaction()->glissandoNoteDurationticks()) {
                                    m_notation->interaction()->addGlissandoNote(toNote(glissandoNote), glissandoNote->tick().ticks(), duration_ticks);

                                    if (m_notation->interaction()->glissandoEndNotes().size() == 0) {
                                        EngravingItemList itemList__ = measure->childrenItems(true);
                                        for (size_t __j = 0; __j < itemList__.size(); __j++) {
                                            EngravingItem* __item__ = itemList__.at(__j);
                                            if (__item__ == nullptr) {
                                                continue;
                                            }
                                            if (__item__->type() == mu::engraving::ElementType::NOTE) {
                                                if (__item__->tick().ticks() >= glissandoNote->tick().ticks() + duration_ticks / 10) {
                                                    m_notation->interaction()->addGlissandoEndNote(toNote(__item__));
                                                }
                                            } 
                                        }
                                    }
                                    
                                    m_notation->interaction()->glissandoEndNotesUpdate();
                                }
                            } 
                        } else if (item->type() == mu::engraving::ElementType::ARPEGGIO) {
                            Arpeggio *__arpeggio = toArpeggio(item);
                            ArpeggioType __arpeggioType = __arpeggio->arpeggioType();
                            EngravingItem* arpeggio = item->parentItem();
                            // check Fermata
                            bool isFermataTag = false;
                            EngravingItem* arpeggioParent = arpeggio->parentItem();
                            EngravingItemList ___itemList = arpeggioParent->childrenItems(false);
                            for (size_t _k = 0; _k < ___itemList.size(); _k++) {
                                EngravingItem* ___item = ___itemList.at(_k);
                                if (___item == nullptr) {
                                    continue;
                                }
                                if (___item->type() == mu::engraving::ElementType::FERMATA) {
                                    isFermataTag = true;
                                    break;
                                }
                            }
                            if (tick.ticks() < m_notation->interaction()->arpeggioNoteTicks() || tick.ticks() > m_notation->interaction()->arpeggioNoteTicks() + m_notation->interaction()->arpeggioNoteDurationticks()) {
                                if (arpeggio->type() == mu::engraving::ElementType::CHORD) {
                                    if (!m_notation->interaction()->arpeggioPointEqual(item->canvasPos())) {
                                        m_notation->interaction()->arpeggioPointClear();
                                        mu::engraving::Chord *arpeggioChord = toChord(arpeggio);
                                        
                                        bool arpeggio_whole = false;
                                        int arpeggio_duration_ticks = arpeggioChord->durationTypeTicks().ticks();
                                        if (arpeggioChord->durationType().type() <= mu::engraving::DurationType::V_HALF) {
                                            if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_HALF) {
                                                arpeggio_duration_ticks /= 3;
                                            } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_WHOLE) {
                                                arpeggio_whole = true;
                                                arpeggio_duration_ticks /= 4;
                                            }
                                            if (isFermataTag) {
                                                arpeggio_duration_ticks /= 8;
                                            } else {
                                                arpeggio_duration_ticks /= 2;
                                            }
                                        } else if (arpeggioChord->durationType().type() == mu::engraving::DurationType::V_QUARTER) {
                                            arpeggio_duration_ticks /= 2;
                                        }

                                        EngravingItemList _itemList = s->childrenItems(true);
                                        for (size_t k = 0; k < _itemList.size(); k++) {
                                            EngravingItem* _item = _itemList.at(k);
                                            if (_item == nullptr) {
                                                continue;
                                            }
                                            if (_item->type() == mu::engraving::ElementType::NOTE) {
                                                EngravingItem* _itemParent = _item->parentItem();
                                                if (_itemParent->type() == mu::engraving::ElementType::CHORD) {
                                                    if (_itemParent->tick().ticks() == t1.ticks()) {
                                                        mu::engraving::Chord *_itemParentChord = toChord(_itemParent);
                                                        int _item_duration_ticks = _itemParentChord->durationTypeTicks().ticks();
                                                        if (tick.ticks() >= _itemParentChord->tick().ticks() && tick.ticks() <= _itemParentChord->tick().ticks() + _item_duration_ticks) {
                                                            if (!m_notation->interaction()->arpeggioNoteTicksExist(item->canvasPos())) {
                                                                m_notation->interaction()->addArpeggioPoint(item->canvasPos());
                                                                m_notation->interaction()->addArpeggioNote(toNote(_item), _itemParentChord->tick().ticks(), arpeggio_duration_ticks);
                                                            } else {
                                                                m_notation->interaction()->addArpeggioNote(toNote(_item));
                                                                if (arpeggio_whole && m_notation->interaction()->arpeggioNotes().size() >= 8) {
                                                                    if (m_notation->interaction()->arpeggioNotes().size() == 8) {
                                                                        m_notation->interaction()->updateArpeggioDuration(1.2 * arpeggio_duration_ticks);
                                                                    } else if (m_notation->interaction()->arpeggioNotes().size() >= 12) {
                                                                        m_notation->interaction()->updateArpeggioDuration(2.4 * arpeggio_duration_ticks);
                                                                    } else if (m_notation->interaction()->arpeggioNotes().size() >= 16) {
                                                                        m_notation->interaction()->updateArpeggioDuration(4 * arpeggio_duration_ticks);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        // if (__arpeggioType == ArpeggioType::DOWN || __arpeggioType == ArpeggioType::DOWN_STRAIGHT) {
                                        //     m_notation->interaction()->arpeggioNotesUpdate(true);
                                        // } else {
                                        //     m_notation->interaction()->arpeggioNotesUpdate(false);
                                        // }
                                        m_notation->interaction()->arpeggioNotesUpdate(false);
                                    }
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
            if (hit_measure() != nullptr && prevMeasure != hit_measure()) {
                for (mu::engraving::Segment* segment = hit_measure()->first(mu::engraving::SegmentType::ChordRest); segment;) {
                    std::vector<EngravingItem*> engravingItemListOfHitMeasure = segment->elist();
                    size_t hit_len = engravingItemListOfHitMeasure.size();
                    for (size_t i = 0; i < hit_len; i++) {
                        EngravingItem* engravingItem = engravingItemListOfHitMeasure[i];
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
        }
        
        curr_clefTypes.clear();
        for (mu::engraving::Segment* segment = measure->first(mu::engraving::SegmentType::ClefType); segment;) {
            std::vector<EngravingItem*> clefItemList = segment->elist();
            size_t len = clefItemList.size();
            for (size_t i = 0; i < len; i++) {
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
                    clefTypes.insert(clefType);
                    mu::engraving::Segment* lastChordRestSegment = measure->last(mu::engraving::SegmentType::ChordRest);
                    if (lastChordRestSegment->canvasPos().x() > clef->canvasPos().x()) {
                        curr_clefTypes.insert(clefType);
                    } else {
                        stash_clefType[measureNo] = clefType;
                    } 
                }
            }

            mu::engraving::Segment* next_segment = segment->next(mu::engraving::SegmentType::ClefType);
            segment = next_segment;
        }
        
        if (measureNo > 0) {
            if (stash_clefType.find(measureNo - 1) != stash_clefType.end()) {
                curr_clefTypes.insert(stash_clefType[measureNo - 1]);
            }
        }

        std::set<mu::engraving::Key> _keySigKeys;
        for (mu::engraving::Segment* segment = measure->last(mu::engraving::SegmentType::KeySigType); segment;) {
            std::vector<EngravingItem*> keySigItemList = segment->elist();
            size_t len = keySigItemList.size();
            for (size_t i = 0; i < len; i++) {
                EngravingItem* keySigItem = keySigItemList[i];
                if (keySigItem == nullptr) {
                    continue;
                }
                
                if (keySigItem->canvasPos().x() < x) {
                    mu::engraving::KeySig *keySig = toKeySig(keySigItem);
                    mu::engraving::Key key = keySig->key();
                    _keySigKeys.insert(key);
                }
            }
            if (!_keySigKeys.empty()) {
                break;
            }
            mu::engraving::Segment* pre_segment = segment->prev(mu::engraving::SegmentType::KeySig);
            segment = pre_segment;
        }

        keySigKeys.clear();
        if (!_keySigKeys.empty()) {
            for (auto key : _keySigKeys) {
                keySigKeys.insert(key);
            }
        } else {
            if (measure == system->firstMeasure()) {
                keySigKeys.insert(mu::engraving::Key::C);
            } else {
                for (Measure* _prevMeasure = measure->prevMeasure(); _prevMeasure;) {
                    if (_prevMeasure) {
                        for (mu::engraving::Segment* segment = _prevMeasure->last(mu::engraving::SegmentType::KeySigType); segment;) {
                            std::vector<EngravingItem*> keySigItemList = segment->elist();
                            size_t len = keySigItemList.size();
                            for (size_t i = 0; i < len; i++) {
                                EngravingItem* keySigItem = keySigItemList[i];
                                if (keySigItem == nullptr) {
                                    continue;
                                }
                                mu::engraving::KeySig *keySig = toKeySig(keySigItem);
                                mu::engraving::Key key = keySig->key();
                                _keySigKeys.insert(key);
                            }
                            if (!_keySigKeys.empty()) {
                                break;
                            }
                            mu::engraving::Segment* prev_segment = segment->prev(mu::engraving::SegmentType::KeySig);
                            segment = prev_segment;
                        }
                        if (!_keySigKeys.empty()) {
                            break;
                        }
                        if (_prevMeasure == system->firstMeasure()) {
                            break;
                        }
                        _prevMeasure = _prevMeasure->prevMeasure();
                    }
                }
                if (!_keySigKeys.empty()) {
                    for (auto key : _keySigKeys) {
                        keySigKeys.insert(key);
                    }
                } else {
                    keySigKeys.insert(mu::engraving::Key::C);
                }
            }
                        
        }
        
        if (curr_clefTypes.size() > 0) {
            if (curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VA) != curr_clefTypes.cend()
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VB) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G);
            } 
            if (curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VA) != curr_clefTypes.cend()
                && curr_clefTypes.find(mu::engraving::ClefType::G15_MA) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G8_VA);
            } 
            if (curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::F8_VB) != curr_clefTypes.cend()
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VB) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
                curr_clefTypes.erase(mu::engraving::ClefType::G);
            } 
            if (curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend()  
                && curr_clefTypes.find(mu::engraving::ClefType::F8_VB) != curr_clefTypes.cend()
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VB) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
                curr_clefTypes.erase(mu::engraving::ClefType::G8_VB);
            } 
            if (curr_clefTypes.find(mu::engraving::ClefType::G8_VA) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G15_MA) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G15_MA);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::G8_VB) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::G8_VA) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::F_8VA) != curr_clefTypes.cend()
                && curr_clefTypes.find(mu::engraving::ClefType::F8_VB) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::F_8VA) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::F8_VB) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::G) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::F_8VA) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::G);
            }
            if (curr_clefTypes.find(mu::engraving::ClefType::F) != curr_clefTypes.cend() 
                && curr_clefTypes.find(mu::engraving::ClefType::G8_VB) != curr_clefTypes.cend()) {
                curr_clefTypes.erase(mu::engraving::ClefType::F);
            }
            for (auto clefType : curr_clefTypes) {
                for (auto key : keySigKeys) {
                    if (clefType == mu::engraving::ClefType::G) {
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 15);
                        }
                    }
                    if (clefType == mu::engraving::ClefType::F) {
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 8);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 22);
                        }
                    }

                    if (clefType == mu::engraving::ClefType::G8_VA) { // G#8va
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 120);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 15 + 120);
                        }
                    }

                    if (clefType == mu::engraving::ClefType::G15_MA) { // G#15va
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 120 * 2);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 15 + 120 * 2);
                        }
                    }

                    if (clefType == mu::engraving::ClefType::G8_VB) { // Gb8va
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 120 * 3);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 15 + 120 * 3);
                        }
                    }

                    if (clefType == mu::engraving::ClefType::F_8VA) { // F#8va
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 8 + 120 * 4);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 22 + 120 * 4);
                        }
                    }

                    if (clefType == mu::engraving::ClefType::F8_VB) { // Fb8va
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 8 + 120 * 5);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 22 + 120 * 5);
                        }
                    }
                }
            }
        } else {
            for (auto clefType : clefTypes) {
                for (auto key : keySigKeys) {
                    if (clefType == mu::engraving::ClefType::G) {
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 15);
                        }
                    }
                    if (clefType == mu::engraving::ClefType::F) {
                        if ((int)key >= 0) {
                            m_notation->interaction()->addClefKeySigsKeys((int)key + 8);
                        } else {
                            m_notation->interaction()->addClefKeySigsKeys(-1 * (int)key + 22);
                        }
                    }
                }
            }
        }
        m_notation->interaction()->notifyClefKeySigsKeysChange();

        setHitMeasureNo(measureNo);
        setHitMeasure(measure);
    }
    
    m_notation->interaction()->notifyPianoKeyboardNotesChanged();

    if (tick.ticks() == 0 && !isPlaying) {
        for (mu::engraving::Segment* segment = score->lastMeasure()->first(mu::engraving::SegmentType::ChordRest); segment;) {
            std::vector<EngravingItem*> engravingItemListOfHitMeasure = segment->elist();
            size_t hit_len = engravingItemListOfHitMeasure.size();
            for (size_t i = 0; i < hit_len; i++) {
                EngravingItem* engravingItem = engravingItemListOfHitMeasure[i];
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
    m_notation->interaction()->glissandoTick(tick.ticks());
    m_notation->interaction()->arpeggioTick(tick.ticks());
    if (m_notation->interaction()->trillTick(tick.ticks())) {
        curr_trill_note = nullptr;
    }
    if (m_notation->interaction()->trillTick1(tick.ticks())) {
        curr_trill_note1 = nullptr;
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
