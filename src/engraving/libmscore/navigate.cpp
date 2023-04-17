/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "navigate.h"

#include "chord.h"
#include "engravingitem.h"
#include "lyrics.h"
#include "measure.h"
#include "measurerepeat.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "staff.h"

using namespace mu;

namespace mu::engraving {
using ElementPair = std::pair<EngravingItem*, int>; // element and its index as a child

//---------------------------------------------------------
//   isChild1beforeChild2
//    compares if two children are order correctly
//    from top-left to bottom-right
//---------------------------------------------------------

static bool isChild1beforeChild2(const ElementPair& child1, const ElementPair& child2)
{
    assert(child1.first->parent() == child2.first->parent());

    PointF p1 = child1.first->pos() + child1.first->bbox().center();
    PointF p2 = child2.first->pos() + child2.first->bbox().center();

    // If one child is *clearly* above the other then visit the higher one first
    double verticalSeparation = p2.y() - p1.y();
    if (verticalSeparation > 10) {
        return true;
    } else if (verticalSeparation < -10) {
        return false;
    }

    // Children are roughly aligned vertically. Visit the one on the left first
    double horizontalSeparation = p2.x() - p1.x();
    if (horizontalSeparation > 0.0) {
        return true;
    } else if (horizontalSeparation < 0.0) {
        return false;
    }

    // Children are aligned horizontally so check exact vertical position.
    if (verticalSeparation > 0.0) {
        return true;
    } else if (verticalSeparation < 0.0) {
        return false;
    }

    // Elements are at exact same position so fall back to using child index to determine order.
    if (child1.second < child2.second) {
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   toChildPairsSet
//    return a set of children ordered
//    on the basis of position
//---------------------------------------------------------

static std::set<ElementPair, decltype(& isChild1beforeChild2)> toChildPairsSet(const EngravingItem* element)
{
    std::set<ElementPair, decltype(& isChild1beforeChild2)> children(&isChild1beforeChild2);
    int index = 0;

    for (EngravingObject* object : element->children()) {
        EngravingItem* engravingItem = toEngravingItem(object);

        if (!engravingItem) {
            continue;
        }

        children.insert(std::pair<EngravingItem*, int>(engravingItem, index));
        index++;
    }

    return children;
}

//---------------------------------------------------------
//   nextChordRest
//    return next Chord or Rest
//---------------------------------------------------------

ChordRest* nextChordRest(const ChordRest* cr, bool skipGrace, bool skipMeasureRepeatRests)
{
    if (!cr) {
        return nullptr;
    }

    if (cr->isGrace()) {
        const Chord* c  = toChord(cr);
        Chord* pc = toChord(cr->explicitParent());

        if (skipGrace) {
            cr = toChordRest(cr->explicitParent());
        } else if (cr->isGraceBefore()) {
            const GraceNotesGroup& group = pc->graceNotesBefore();
            auto i = std::find(group.begin(), group.end(), c);
            if (i == group.end()) {
                return nullptr;           // unable to find self?
            }
            ++i;
            if (i != group.end()) {
                return *i;
            }
            // if this was last grace note before, return parent
            return pc;
        } else {
            const GraceNotesGroup& group = pc->graceNotesAfter();
            auto i = std::find(group.begin(), group.end(), c);
            if (i == group.end()) {
                return nullptr;           // unable to find self?
            }
            ++i;
            if (i != group.end()) {
                return *i;
            }
            // if this was last grace note after, fall through to find next main note
            cr = pc;
        }
    } else { // cr is not a grace note
        if (cr->isChord() && !skipGrace) {
            const Chord* c = toChord(cr);
            if (!c->graceNotes().empty()) {
                const GraceNotesGroup& group = c->graceNotesAfter();
                if (!group.empty()) {
                    return group.front();
                }
            }
        }
    }

    track_idx_t track = cr->track();
    SegmentType st = SegmentType::ChordRest;

    for (Segment* seg = cr->segment()->next1MM(st); seg; seg = seg->next1MM(st)) {
        ChordRest* e = toChordRest(seg->element(track));
        if (e) {
            if (skipMeasureRepeatRests && e->isRest() && e->measure()->isMeasureRepeatGroup(track2staff(track))) {
                continue; // these rests are not shown, skip them
            }
            if (e->isChord() && !skipGrace) {
                Chord* c = toChord(e);
                if (!c->graceNotes().empty()) {
                    const GraceNotesGroup& group = c->graceNotesBefore();
                    if (!group.empty()) {
                        return group.front();
                    }
                }
            }
            return e;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   prevChordRest
//    return previous Chord or Rest
//    if grace is true, include grace notes
//---------------------------------------------------------

ChordRest* prevChordRest(const ChordRest* cr, bool skipGrace, bool skipMeasureRepeatRests)
{
    if (!cr) {
        return nullptr;
    }

    if (cr->isGrace()) {
        const Chord* c  = toChord(cr);
        Chord* pc = toChord(cr->explicitParent());

        if (skipGrace) {
            cr = toChordRest(cr->explicitParent());
        } else if (cr->isGraceBefore()) {
            const GraceNotesGroup& group = pc->graceNotesBefore();
            auto i = std::find(group.begin(), group.end(), c);
            if (i == group.end()) {
                return nullptr;           // unable to find self?
            }
            if (i != group.begin()) {
                return *--i;
            }
            // if this was first grace note before, fall through to find previous main note
            cr = pc;
        } else {
            const GraceNotesGroup& group = pc->graceNotesAfter();
            auto i = std::find(group.begin(), group.end(), c);
            if (i == group.end()) {
                return nullptr;           // unable to find self?
            }
            if (i != group.begin()) {
                return *--i;
            }
            // if this was first grace note after, return parent
            return pc;
        }
    } else {
        //
        // cr is not a grace note
        if (cr->isChord() && !skipGrace) {
            const Chord* c = toChord(cr);
            const GraceNotesGroup& group = c->graceNotesBefore();
            if (!group.empty()) {
                return group.back();
            }
        }
    }

    track_idx_t track = cr->track();
    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = cr->segment()->prev1MM(st); seg; seg = seg->prev1MM(st)) {
        ChordRest* e = toChordRest(seg->element(track));
        if (e) {
            if (skipMeasureRepeatRests && e->isRest() && e->measure()->isMeasureRepeatGroup(track2staff(track))) {
                continue; // these rests are not shown, skip them
            }
            if (e->isChord() && !skipGrace) {
                const GraceNotesGroup& group = toChord(e)->graceNotesAfter();
                if (!group.empty()) {
                    return group.back();
                }
            }
            return e;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   upAlt
//    element: Note, Rest, MMRest, or MeasureRepeat
//    return: Note, Rest, MMRest, or MeasureRepeat
//
//    return next higher pitched note in chord
//    move to previous track if at top of chord
//---------------------------------------------------------

EngravingItem* Score::upAlt(EngravingItem* element)
{
    EngravingItem* re = 0;
    if (element->isRestFamily()) {
        re = prevTrack(toRest(element));
    } else if (element->isNote()) {
        Note* note = toNote(element);
        Chord* chord = note->chord();
        const std::vector<Note*>& notes = chord->notes();
        auto i = std::find(notes.begin(), notes.end(), note);
        ++i;
        if (i != notes.end()) {
            re = *i;
        } else {
            re = prevTrack(chord);
            if (re->track() == chord->track()) {
                re = element;
            }
        }
    }
    if (re == 0) {
        return 0;
    }
    if (re->isChord()) {
        re = toChord(re)->notes().front();
    }
    return re;
}

//---------------------------------------------------------
//   upAltCtrl
//    select top note in chord
//---------------------------------------------------------

Note* Score::upAltCtrl(Note* note) const
{
    return note->chord()->upNote();
}

//---------------------------------------------------------
//   downAlt
//    return next lower pitched note in chord
//    move to next track if at bottom of chord
//---------------------------------------------------------

EngravingItem* Score::downAlt(EngravingItem* element)
{
    EngravingItem* re = 0;
    if (element->isRestFamily()) {
        re = nextTrack(toRest(element));
    } else if (element->isNote()) {
        Note* note   = toNote(element);
        Chord* chord = note->chord();
        const std::vector<Note*>& notes = chord->notes();
        auto i = std::find(notes.begin(), notes.end(), note);
        if (i != notes.begin()) {
            --i;
            re = *i;
        } else {
            re = nextTrack(chord);
            if (re->track() == chord->track()) {
                re = element;
            }
        }
    }
    if (re == 0) {
        return 0;
    }
    if (re->isChord()) {
        re = toChord(re)->notes().back();
    }
    return re;
}

//---------------------------------------------------------
//   downAltCtrl
//    niedrigste Note in Chord selektieren
//---------------------------------------------------------

Note* Score::downAltCtrl(Note* note) const
{
    return note->chord()->downNote();
}

//---------------------------------------------------------
//   firstElement
//---------------------------------------------------------

EngravingItem* Score::firstElement(bool frame)
{
    if (frame) {
        MeasureBase* mb = measures()->first();
        if (mb && mb->isBox()) {
            return mb;
        }
    }
    Segment* s = firstSegmentMM(SegmentType::All);
    return s ? s->element(0) : nullptr;
}

//---------------------------------------------------------
//   lastElement
//---------------------------------------------------------

EngravingItem* Score::lastElement(bool frame)
{
    if (frame) {
        MeasureBase* mb = measures()->last();
        if (mb && mb->isBox()) {
            return mb;
        }
    }
    EngravingItem* re = 0;
    Segment* seg = lastSegmentMM();
    if (!seg) {
        return nullptr;
    }
    while (true) {
        for (size_t i = (staves().size() - 1) * VOICES; i < staves().size() * VOICES; i++) {
            if (seg->element(i)) {
                re = seg->element(i);
            }
        }
        if (re) {
            if (re->isChord()) {
                return toChord(re)->notes().front();
            }
            return re;
        }
        seg = seg->prev1MM(SegmentType::All);
    }
}

//---------------------------------------------------------
//   upStaff
//---------------------------------------------------------

ChordRest* Score::upStaff(ChordRest* cr)
{
    Segment* segment = cr->segment();

    if (cr->staffIdx() == 0) {
        return cr;
    }

    for (int track = (static_cast<int>(cr->staffIdx()) - 1) * VOICES; track >= 0; --track) {
        EngravingItem* el = segment->element(track);
        if (!el) {
            continue;
        }
        if (el->isNote()) {
            el = toNote(el)->chord();
        }
        if (el->isChordRest()) {
            return toChordRest(el);
        }
    }
    return 0;
}

//---------------------------------------------------------
//   downStaff
//---------------------------------------------------------

ChordRest* Score::downStaff(ChordRest* cr)
{
    Segment* segment = cr->segment();
    track_idx_t tracks = nstaves() * VOICES;

    if (cr->staffIdx() == nstaves() - 1) {
        return cr;
    }

    for (track_idx_t track = (cr->staffIdx() + 1) * VOICES; track < tracks; --track) {
        EngravingItem* el = segment->element(track);
        if (!el) {
            continue;
        }
        if (el->isNote()) {
            el = toNote(el)->chord();
        }
        if (el->isChordRest()) {
            return toChordRest(el);
        }
    }
    return 0;
}

//---------------------------------------------------------
//   nextTrack
//    returns note at or just before current (cr) position
//    in next track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::nextTrack(ChordRest* cr, bool skipMeasureRepeatRests)
{
    if (!cr) {
        return 0;
    }

    ChordRest* el = nullptr;
    Measure* measure = cr->measure();
    track_idx_t track = cr->track();
    size_t tracks = nstaves() * VOICES;

    while (!el) {
        // find next non-empty track
        while (++track < tracks) {
            if (measure->hasVoice(track)) {
                break;
            }
        }
        // no more tracks, return original element
        if (track == tracks) {
            return cr;
        }
        // treat MeasureRepeat as if it starts in the first measure of its group even if internally it's farther along
        if (cr->isMeasureRepeat()) {
            measure = measure->firstOfMeasureRepeatGroup(cr->staffIdx());
            return toChordRest(measure->first(SegmentType::ChordRest)->element(track));
        }
        // find element at same or previous segment within this track
        for (Segment* segment = cr->segment(); segment; segment = segment->prev(SegmentType::ChordRest)) {
            el = toChordRest(segment->element(track));
            if (el) {
                break;
            }
        }
    }
    if (skipMeasureRepeatRests && el->isRest() && measure->isMeasureRepeatGroup(track2staff(track))) {
        el = measure->measureRepeatElement(track2staff(track));
    }
    return el;
}

//---------------------------------------------------------
//   prevTrack
//    returns ChordRest at or just before current (cr) position
//    in previous track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::prevTrack(ChordRest* cr, bool skipMeasureRepeatRests)
{
    if (!cr) {
        return 0;
    }

    ChordRest* el = nullptr;
    Measure* measure = cr->measure();
    int track = static_cast<int>(cr->track());

    while (!el) {
        // find next non-empty track
        while (--track >= 0) {
            if (measure->hasVoice(track)) {
                break;
            }
        }
        // no more tracks, return original element
        if (track < 0) {
            return cr;
        }
        // treat MeasureRepeat as if it starts in the first measure of its group even if internally it's farther along
        if (cr->isMeasureRepeat()) {
            measure = measure->firstOfMeasureRepeatGroup(cr->staffIdx());
            return toChordRest(measure->first(SegmentType::ChordRest)->element(track));
        }
        // find element at same or previous segment within this track
        for (Segment* segment = cr->segment(); segment; segment = segment->prev(SegmentType::ChordRest)) {
            el = toChordRest(segment->element(track));
            if (el) {
                break;
            }
        }
    }
    if (skipMeasureRepeatRests && el->isRest() && measure->isMeasureRepeatGroup(track2staff(track))) {
        el = measure->measureRepeatElement(track2staff(track));
    }
    return el;
}

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* element, bool selectBehavior, bool mmRest)
{
    if (!element) {
        return 0;
    }

    Measure* measure = nullptr;
    if (mmRest) {
        measure = element->measure()->nextMeasureMM();
    } else {
        measure = element->measure()->nextMeasure();
    }

    if (!measure) {
        return 0;
    }

    Fraction endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
    bool last   = false;

    if (selection().isRange()) {
        if (element->tick() != endTick && selection().tickEnd() <= endTick) {
            measure = element->measure();
            last = true;
        } else if (element->tick() == endTick && selection().isEndActive()) {
            last = true;
        }
    } else if (element->tick() != endTick && selectBehavior) {
        measure = element->measure();
        last = true;
    }
    if (!measure) {
        measure = element->measure();
        last = true;
    }
    staff_idx_t staff = element->staffIdx();

    Segment* startSeg = last ? measure->last() : measure->first();
    for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
        track_idx_t etrack = (staff + 1) * VOICES;
        for (track_idx_t track = staff * VOICES; track < etrack; ++track) {
            EngravingItem* pel = seg->element(track);

            if (pel && pel->isChordRest()) {
                return toChordRest(pel);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element, bool mmRest)
{
    if (!element) {
        return 0;
    }

    Measure* measure =  0;
    if (mmRest) {
        measure = element->measure()->prevMeasureMM();
    } else {
        measure = element->measure()->prevMeasure();
    }

    Fraction startTick = element->measure()->first()->nextChordRest(element->track())->tick();
    bool last = false;

    if (selection().isRange() && selection().isEndActive() && selection().startSegment()->tick() <= startTick) {
        last = true;
    } else if (element->tick() != startTick) {
        measure = element->measure();
    }
    if (!measure) {
        measure = element->measure();
        last = false;
    }

    staff_idx_t staff = element->staffIdx();

    Segment* startSeg = last ? measure->last() : measure->first();
    for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
        track_idx_t etrack = (staff + 1) * VOICES;
        for (track_idx_t track = staff * VOICES; track < etrack; ++track) {
            EngravingItem* pel = seg->element(track);

            if (pel && pel->isChordRest()) {
                return toChordRest(pel);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* Score::nextElement()
{
    EngravingItem* e = getSelectedElement();
    if (!e) {
        return nullptr;
    }
    staff_idx_t staffId = e->staffIdx();
    while (e) {
        switch (e->type()) {
        case ElementType::NOTE:
        case ElementType::REST:
        case ElementType::MMREST:
        case ElementType::CHORD: {
            EngravingItem* next = e->nextElement();
            if (next) {
                return next;
            } else {
                break;
            }
        }
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            EngravingItem* next = s->nextElement(staffId);
            if (next) {
                return next;
            } else {
                break;
            }
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            EngravingItem* next = m->nextElementStaff(staffId);
            if (next) {
                return next;
            } else {
                break;
            }
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE: {
            for (; e && e->type() != ElementType::SEGMENT; e = e->parentItem()) {
            }
            Segment* s = toSegment(e);
            EngravingItem* next = s->nextElement(staffId);
            if (next) {
                return next;
            } else {
                return score()->firstElement();
            }
        }
        case ElementType::VOLTA_SEGMENT:
        case ElementType::SLUR_SEGMENT:
        case ElementType::TEXTLINE_SEGMENT:
        case ElementType::HAIRPIN_SEGMENT:
        case ElementType::OTTAVA_SEGMENT:
        case ElementType::TRILL_SEGMENT:
        case ElementType::VIBRATO_SEGMENT:
        case ElementType::LET_RING_SEGMENT:
        case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
        case ElementType::PALM_MUTE_SEGMENT:
        case ElementType::WHAMMY_BAR_SEGMENT:
        case ElementType::RASGUEADO_SEGMENT:
        case ElementType::HARMONIC_MARK_SEGMENT:
        case ElementType::PICK_SCRAPE_SEGMENT:
        case ElementType::PEDAL_SEGMENT: {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            Spanner* nextSp = sp->nextSpanner(sp, staffId);
            if (nextSp) {
                return nextSp->spannerSegments().front();
            }

            Segment* seg = tick2segment(sp->tick());
            if (seg) {
                Segment* nextSegment = seg->next1();
                while (nextSegment) {
                    EngravingItem* nextEl = nextSegment->firstElementOfSegment(nextSegment, staffId);
                    if (nextEl) {
                        return nextEl;
                    }
                    nextSegment = nextSegment->next1MM();
                }
            }
            break;
        }
        case ElementType::GLISSANDO_SEGMENT:
        case ElementType::TIE_SEGMENT: {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            EngravingItem* elSt = sp->startElement();
            Note* n = toNote(elSt);
            EngravingItem* next =  n->nextElement();
            if (next) {
                return next;
            } else {
                break;
            }
        }
        case ElementType::VBOX:
        case ElementType::HBOX:
        case ElementType::TBOX: {
            auto boxChildren = toChildPairsSet(e);

            EngravingItem* selectedElement = getSelectedElement();

            if ((selectedElement->type() == ElementType::VBOX
                 || selectedElement->type() == ElementType::HBOX
                 || selectedElement->type() == ElementType::TBOX) && !boxChildren.empty()) {
                return boxChildren.begin()->first;
            }

            for (auto child = boxChildren.begin(); child != boxChildren.end(); child++) {
                if (selectedElement != child->first) {
                    continue;
                }

                auto targetElement = std::next(child);

                if (targetElement != boxChildren.end()) {
                    return targetElement->first;
                }
            }

            MeasureBase* mb = toMeasureBase(e)->nextMM();
            if (!mb) {
                break;
            } else if (mb->isMeasure()) {
                ChordRest* cr = selection().currentCR();
                staff_idx_t si = cr ? cr->staffIdx() : 0;
                return toMeasure(mb)->nextElementStaff(si);
            } else {
                return mb;
            }
        }
        case ElementType::LAYOUT_BREAK: {
            staffId = 0;             // otherwise it will equal -1, which breaks the navigation
        }
        default:
            break;
        }
        e = e->parentItem();
    }
    return score()->lastElement();
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* Score::prevElement()
{
    EngravingItem* e = getSelectedElement();
    if (!e) {
        return nullptr;
    }
    staff_idx_t staffId = e->staffIdx();
    while (e) {
        switch (e->type()) {
        case ElementType::NOTE:
        case ElementType::REST:
        case ElementType::MMREST:
        case ElementType::CHORD: {
            EngravingItem* prev = e->prevElement();
            if (prev) {
                return prev;
            } else {
                break;
            }
        }
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            EngravingItem* prev = s->prevElement(staffId);
            if (prev) {
                return prev;
            } else {
                break;
            }
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            return m->prevElementStaff(staffId);
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE: {
            for (; e && e->type() != ElementType::SEGMENT; e = e->parentItem()) {
            }
            EngravingItem* previousElement = toSegment(e)->prevElement(staffId);

            if (previousElement->type() != ElementType::VBOX
                && previousElement->type() != ElementType::HBOX
                && previousElement->type() != ElementType::TBOX) {
                return previousElement;
            }

            auto boxChildren = toChildPairsSet(previousElement);

            if (boxChildren.size() > 0) {
                return boxChildren.rbegin()->first;
            }

            return previousElement;
        }
        case ElementType::VOLTA_SEGMENT:
        case ElementType::SLUR_SEGMENT:
        case ElementType::TEXTLINE_SEGMENT:
        case ElementType::HAIRPIN_SEGMENT:
        case ElementType::OTTAVA_SEGMENT:
        case ElementType::TRILL_SEGMENT:
        case ElementType::VIBRATO_SEGMENT:
        case ElementType::PEDAL_SEGMENT: {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            EngravingItem* stEl = sp->startElement();
            Spanner* prevSp = sp->prevSpanner(sp, staffId);
            if (prevSp) {
                return prevSp->spannerSegments().front();
            } else {
                Segment* startSeg = sp->startSegment();
                if (!startSeg->annotations().empty()) {
                    EngravingItem* last = startSeg->lastAnnotation(startSeg, staffId);
                    if (last) {
                        return last;
                    }
                }
                EngravingItem* el = startSeg->lastElementOfSegment(startSeg, staffId);
                if (stEl->type() == ElementType::CHORD || stEl->type() == ElementType::REST
                    || stEl->type() == ElementType::MEASURE_REPEAT || stEl->type() == ElementType::MMREST
                    || stEl->type() == ElementType::NOTE) {
                    ChordRest* cr = startSeg->cr(stEl->track());
                    if (cr) {
                        EngravingItem* elCr = cr->lastElementBeforeSegment();
                        if (elCr) {
                            return elCr;
                        }
                    }
                }
                if (el->isChord()) {
                    return toChord(el)->lastElementBeforeSegment();
                } else if (el->isNote()) {
                    Chord* c = toNote(el)->chord();
                    return c->lastElementBeforeSegment();
                } else {
                    return el;
                }
            }
        }
        case ElementType::GLISSANDO_SEGMENT:
        case ElementType::TIE_SEGMENT: {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            EngravingItem* elSt = sp->startElement();
            assert(elSt->type() == ElementType::NOTE);
            Note* n = toNote(elSt);
            EngravingItem* prev =  n->prevElement();
            if (prev) {
                return prev;
            } else {
                break;
            }
        }
        case ElementType::VBOX:
        case ElementType::HBOX:
        case ElementType::TBOX: {
            auto boxChildren = toChildPairsSet(e);

            EngravingItem* selectedElement = getSelectedElement();

            for (auto child = boxChildren.rbegin(); child != boxChildren.rend(); child++) {
                if (selectedElement != child->first) {
                    continue;
                }

                auto targetElement = std::next(child);

                if (targetElement != boxChildren.rend()) {
                    return targetElement->first;
                }
            }

            MeasureBase* mb = toMeasureBase(e)->prevMM();
            if (!mb) {
                break;
            } else if (mb->isMeasure()) {
                ChordRest* cr = selection().currentCR();
                staff_idx_t si = cr ? cr->staffIdx() : 0;
                Segment* s = toMeasure(mb)->last();
                if (s) {
                    return s->lastElement(si);
                }
            } else {
                return mb;
            }
        }
        break;
        case ElementType::LAYOUT_BREAK: {
            staffId = 0;             // otherwise it will equal -1, which breaks the navigation
        }
        default:
            break;
        }
        e = e->parentItem();
    }
    return score()->firstElement();
}

//---------------------------------------------------------
//   prevLyrics
//    - find the lyric (if any) before this one (not including lines)
//    - currently used to determine the first lyric of a melisma
//---------------------------------------------------------

Lyrics* prevLyrics(const Lyrics* lyrics)
{
    track_idx_t currTrack = lyrics->track();
    Segment* seg = lyrics->segment();
    if (!seg) {
        return nullptr;
    }
    Segment* prevSegment = seg;
    while ((prevSegment = prevSegment->prev1(mu::engraving::SegmentType::ChordRest))) {
        EngravingItem* el = prevSegment->element(currTrack);
        Lyrics* prevLyrics = el && el->isChord() ? toChordRest(el)->lyrics(lyrics->no(), lyrics->placement()) : nullptr;
        if (prevLyrics) {
            return prevLyrics;
        }
    }
    return nullptr;
}
}
