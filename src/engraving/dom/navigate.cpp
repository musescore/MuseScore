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

#include "navigate.h"

#include "box.h"
#include "chord.h"
#include "fret.h"
#include "guitarbend.h"
#include "engravingitem.h"
#include "lyrics.h"
#include "hammeronpulloff.h"
#include "measure.h"
#include "measurerepeat.h"
#include "marker.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "staff.h"
#include "soundflag.h"
#include "tapping.h"

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

    PointF p1 = child1.first->pos() + child1.first->ldata()->bbox().center();
    PointF p2 = child2.first->pos() + child2.first->ldata()->bbox().center();

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

static EngravingItem* nextElementForSpannerSegment(const SpannerSegment* spannerSegment)
{
    Spanner* spanner = spannerSegment->spanner();
    EngravingItem* elSt = spanner->startElement();
    IF_ASSERT_FAILED(elSt->isNote()) {
        return nullptr;
    }

    Note* note = toNote(elSt);
    return note->nextElement();
}

static EngravingItem* prevElementForSpannerSegment(const SpannerSegment* spannerSegment)
{
    Spanner* spanner = spannerSegment->spanner();
    EngravingItem* elSt = spanner->startElement();
    IF_ASSERT_FAILED(elSt->isNote()) {
        return nullptr;
    }

    Note* note = toNote(elSt);
    return note->prevElement();
}

//---------------------------------------------------------
//   nextChordRest
//    return next Chord or Rest
//---------------------------------------------------------

ChordRest* nextChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options)
{
    if (!cr) {
        return nullptr;
    }

    if (cr->isGrace()) {
        const Chord* c  = toChord(cr);
        Chord* pc = toChord(cr->explicitParent());

        if (options.skipGrace) {
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
        if (cr->isChord() && !options.skipGrace) {
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
    Segment* curSeg = cr->segment();
    for (Segment* seg = curSeg->next1MM(st); seg; seg = seg->next1MM(st)) {
        if (options.disableOverRepeats && !segmentsAreAdjacentInRepeatStructure(curSeg, seg)) {
            return nullptr;
        }
        ChordRest* e = toChordRest(seg->element(track));
        if (e) {
            if (options.skipMeasureRepeatRests && e->isRest() && e->measure()->isMeasureRepeatGroup(track2staff(track))) {
                continue; // these rests are not shown, skip them
            }
            if (e->isChord() && !options.skipGrace) {
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

ChordRest* prevChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options)
{
    if (!cr) {
        return nullptr;
    }

    if (cr->isGrace()) {
        const Chord* c  = toChord(cr);
        Chord* pc = toChord(cr->explicitParent());

        if (options.skipGrace) {
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
        if (cr->isChord() && !options.skipGrace) {
            const Chord* c = toChord(cr);
            const GraceNotesGroup& group = c->graceNotesBefore();
            if (!group.empty()) {
                return group.back();
            }
        }
    }

    track_idx_t track = cr->track();
    SegmentType st = SegmentType::ChordRest;
    Segment* curSeg = cr->segment();
    for (Segment* seg = cr->segment()->prev1MM(st); seg; seg = seg->prev1MM(st)) {
        if (options.disableOverRepeats && !segmentsAreAdjacentInRepeatStructure(curSeg, seg)) {
            return nullptr;
        }

        ChordRest* e = toChordRest(seg->element(track));
        if (e) {
            if (options.skipMeasureRepeatRests && e->isRest() && e->measure()->isMeasureRepeatGroup(track2staff(track))) {
                continue; // these rests are not shown, skip them
            }
            if (e->isChord() && !options.skipGrace) {
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
            auto boxChildren = toChildPairsSet(mb);
            if (!boxChildren.empty()) {
                return boxChildren.rbegin()->first;
            }
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
        return nullptr;
    }

    Measure* measure = nullptr;
    if (mmRest) {
        measure = element->measure()->nextMeasureMM();
    } else {
        measure = element->measure()->nextMeasure();
    }

    Fraction endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
    bool last = false;

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
    return nullptr;
}

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element, bool mmRest)
{
    if (!element) {
        return nullptr;
    }

    Measure* measure = nullptr;
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
    return nullptr;
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
        case ElementType::CHORD:
        case ElementType::TUPLET: {
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
        case ElementType::TAPPING:
        {
            TappingHalfSlur* halfSlurAbove = toTapping(e)->halfSlurAbove();
            EngravingItem* selEl = getSelectedElement();
            if (halfSlurAbove && !(selEl && selEl->isTappingHalfSlurSegment())) {
                return halfSlurAbove->frontSegment();
            }
            break;
        }
        case ElementType::HAMMER_ON_PULL_OFF_TEXT:
            return toHammerOnPullOffText(e)->endChord()->upNote();
        case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
        {
            HammerOnPullOffSegment* hopoSeg = toHammerOnPullOffSegment(e);
            if (!hopoSeg->hopoText().empty()) {
                return hopoSeg->hopoText().front();
            } // else fallthrough:
        }
        case ElementType::TAPPING_HALF_SLUR_SEGMENT:
        {
            TappingHalfSlur* halfSlur = toTappingHalfSlurSegment(e)->tappingHalfSlur();
            Tapping* tapping = halfSlur->tapping();
            if (halfSlur->isHalfSlurAbove() && tapping->halfSlurBelow()) {
                return tapping->halfSlurBelow()->frontSegment();
            } // else fallthrough:
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
                    if (nextSegment->isTimeTickType()) {
                        if (EngravingItem* annotation = nextSegment->firstAnnotation(staffId)) {
                            return annotation;
                        }
                        if (Spanner* spanner = nextSegment->firstSpanner(staffId)) {
                            return spanner->spannerSegments().front();
                        }
                    } else if (EngravingItem* nextEl = nextSegment->firstElementOfSegment(staffId)) {
                        return nextEl;
                    }
                    nextSegment = nextSegment->next1MM();
                }
            }
            break;
        }
        case ElementType::GUITAR_BEND_SEGMENT:
        case ElementType::GUITAR_BEND_HOLD_SEGMENT: {
            GuitarBend* bend
                = e->isGuitarBendSegment() ? toGuitarBendSegment(e)->guitarBend() : toGuitarBendHoldSegment(e)->guitarBendHold()->
                  guitarBend();
            if (bend->type() != GuitarBendType::SLIGHT_BEND) {
                return bend->endNote();
            } else {
                EngravingItem* next = nextElementForSpannerSegment(toSpannerSegment(e));
                if (next) {
                    return next;
                }
            }

            break;
        }
        case ElementType::GLISSANDO_SEGMENT:
        case ElementType::NOTELINE_SEGMENT:
        case ElementType::LAISSEZ_VIB_SEGMENT:
        case ElementType::PARTIAL_TIE_SEGMENT:
        case ElementType::TIE_SEGMENT: {
            EngravingItem* next = nextElementForSpannerSegment(toSpannerSegment(e));
            if (next) {
                return next;
            } else {
                break;
            }
        }
        case ElementType::VBOX:
        case ElementType::HBOX:
        case ElementType::TBOX:
        case ElementType::FBOX: {
            auto boxChildren = toChildPairsSet(e);

            EngravingItem* selectedElement = getSelectedElement();

            if ((selectedElement->type() == ElementType::VBOX
                 || selectedElement->type() == ElementType::HBOX
                 || selectedElement->type() == ElementType::TBOX) && !boxChildren.empty()) {
                return boxChildren.begin()->first;
            }

            if (selectedElement->type() == ElementType::FBOX) {
                for (EngravingItem* child : toFBox(selectedElement)->el()) {
                    if (child->isFretDiagram() && child->visible()) {
                        return toFretDiagram(child)->harmony();
                    }
                }
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
            break;
        }
        case ElementType::SYSTEM_LOCK_INDICATOR:
        {
            staffId = 0;
            e = toSystemLockIndicator(e)->systemLock()->endMB();
            continue;
        }
        case ElementType::SOUND_FLAG:
            if (EngravingItem* parent = toSoundFlag(e)->parentItem()) {
                return parent;
            }
            break;
        case ElementType::HARMONY: {
            if (EngravingItem* parent = toHarmony(e)->parentItem()) {
                if (parent->isFretDiagram()) {
                    return parent;
                }
            }
            break;
        }
        case ElementType::FRET_DIAGRAM: {
            FretDiagram* fretDiagram = toFretDiagram(e);
            if (fretDiagram->isInFretBox()) {
                const ElementList& diagrams = toFBox(fretDiagram->explicitParent())->el();

                size_t index = muse::indexOf(diagrams, fretDiagram);
                if (index != muse::nidx) {
                    while (++index < diagrams.size()) {
                        FretDiagram* fretDiagramI = toFretDiagram(diagrams[index]);
                        if (fretDiagramI->visible()) {
                            return fretDiagramI->harmony();
                        }
                    }
                }
            }
            break;
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
        case ElementType::CHORD:
        case ElementType::TUPLET: {
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
                && previousElement->type() != ElementType::TBOX
                && previousElement->type() == ElementType::FBOX) {
                return previousElement;
            }

            auto boxChildren = toChildPairsSet(previousElement);

            if (boxChildren.size() > 0) {
                return boxChildren.rbegin()->first;
            }

            return previousElement;
        }
        case ElementType::HAMMER_ON_PULL_OFF_TEXT:
        {
            HammerOnPullOffText* hopoText = toHammerOnPullOffText(e);
            HammerOnPullOffSegment* hopoSegment = toHammerOnPullOffSegment(hopoText->parent());
            DO_ASSERT(hopoSegment);
            if (hopoSegment->hopoText().size() > 0 && hopoSegment->hopoText().front() == hopoText) {
                return hopoSegment;
            } else {
                return hopoText->startChord()->downNote();
            }
        }
        case ElementType::TAPPING_HALF_SLUR_SEGMENT:
        {
            TappingHalfSlur* halfSlur = toTappingHalfSlurSegment(e)->tappingHalfSlur();
            Tapping* tapping = halfSlur->tapping();
            if (!halfSlur->isHalfSlurAbove()) {
                IF_ASSERT_FAILED(tapping->halfSlurAbove()) {
                    return tapping;
                }
                return tapping->halfSlurAbove()->frontSegment();
            } else {
                return tapping;
            }
        }
        case ElementType::VOLTA_SEGMENT:
        case ElementType::SLUR_SEGMENT:
        case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
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
                if (EngravingItem* annotation = startSeg->lastAnnotation(staffId)) {
                    return annotation;
                }
                if (startSeg->isTimeTickType()) {
                    startSeg = startSeg->prev1MMenabled();
                    for (; startSeg && startSeg->isTimeTickType(); startSeg = startSeg->prev1MMenabled()) {
                        if (Spanner* spanner = startSeg->lastSpanner(staffId)) {
                            return spanner->spannerSegments().front();
                        }
                        if (EngravingItem* annotation = startSeg->lastAnnotation(staffId)) {
                            return annotation;
                        }
                    }
                    if (!startSeg) {
                        break;
                    }
                    // Also check first non-timeTick segment encountered.
                    if (Spanner* spanner = startSeg->lastSpanner(staffId)) {
                        return spanner->spannerSegments().front();
                    }
                    if (EngravingItem* annotation = startSeg->lastAnnotation(staffId)) {
                        return annotation;
                    }
                }
                EngravingItem* el = startSeg->lastElementOfSegment(staffId);
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
        case ElementType::GUITAR_BEND_SEGMENT:
        case ElementType::GUITAR_BEND_HOLD_SEGMENT: {
            GuitarBend* bend = e->isGuitarBendSegment() ? toGuitarBendSegment(e)->guitarBend()
                               : toGuitarBendHoldSegment(e)->guitarBendHold()->guitarBend();
            return bend->startNote();
        }
        case ElementType::GLISSANDO_SEGMENT:
        case ElementType::NOTELINE_SEGMENT:
        case ElementType::LAISSEZ_VIB_SEGMENT:
        case ElementType::PARTIAL_TIE_SEGMENT:
        case ElementType::TIE_SEGMENT: {
            EngravingItem* prev = prevElementForSpannerSegment(toSpannerSegment(e));
            if (prev) {
                return prev;
            }

            break;
        }
        case ElementType::VBOX:
        case ElementType::HBOX:
        case ElementType::TBOX:
        case ElementType::FBOX: {
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
                    return s->lastElementForNavigation(si);
                }
            } else {
                return mb;
            }
        }
        break;
        case ElementType::LAYOUT_BREAK: {
            staffId = 0;             // otherwise it will equal -1, which breaks the navigation
            break;
        }
        case ElementType::SYSTEM_LOCK_INDICATOR:
        {
            staffId = 0;
            e = toSystemLockIndicator(e)->systemLock()->endMB();
            continue;
        }
        case ElementType::HARMONY: {
            Harmony* harmony = toHarmony(e);
            if (harmony->isInFretBox()) {
                FretDiagram* fretDiagram = toFretDiagram(harmony->explicitParent());
                FBox* fretBox = toFBox(fretDiagram->explicitParent());
                const ElementList& diagrams = fretBox->el();

                size_t index = muse::indexOf(diagrams, fretDiagram);
                if (index != muse::nidx) {
                    while (--index > 0) {
                        FretDiagram* fretDiagramI = toFretDiagram(diagrams[index]);
                        if (fretDiagramI->visible()) {
                            return fretDiagramI;
                        }
                    }
                }

                return fretBox;
            } else if (harmony->explicitParent()->isFretDiagram()) {
                // jump over fret diagram
                e = harmony->getParentSeg();
                continue;
            }
            break;
        }
        case ElementType::FRET_DIAGRAM: {
            FretDiagram* fretDiagram = toFretDiagram(e);
            if (EngravingItem* harmony = fretDiagram->harmony()) {
                return harmony;
            }
            break;
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

Lyrics* lastLyricsInMeasure(const Segment* seg, const staff_idx_t staffIdx, const int no, const PlacementV& placement)
{
    while (seg) {
        const track_idx_t strack = staffIdx * VOICES;
        const track_idx_t etrack = strack + VOICES;
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* el = seg->element(track);
            Lyrics* prevLyrics = el && el->isChord() ? toChordRest(el)->lyrics(no, placement) : nullptr;
            if (prevLyrics) {
                return prevLyrics;
            }
        }
        seg = seg->prev1(mu::engraving::SegmentType::ChordRest);
    }
    return nullptr;
}

Lyrics* prevLyrics(const Lyrics* lyrics)
{
    Segment* seg = lyrics->explicitParent() ? lyrics->segment() : nullptr;
    if (!seg) {
        return nullptr;
    }
    while ((seg = seg->prev1(mu::engraving::SegmentType::ChordRest))) {
        const track_idx_t strack = lyrics->staffIdx() * VOICES;
        const track_idx_t etrack = strack + VOICES;
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* el = seg->element(track);
            Lyrics* prevLyrics = el && el->isChord() ? toChordRest(el)->lyrics(lyrics->no(), lyrics->placement()) : nullptr;
            if (prevLyrics) {
                return prevLyrics;
            }
        }
    }
    return nullptr;
}

Lyrics* nextLyrics(const Lyrics* lyrics)
{
    Segment* seg = lyrics->explicitParent() ? lyrics->segment() : nullptr;
    if (!seg) {
        return nullptr;
    }
    Segment* nextSegment = seg;
    while ((nextSegment = nextSegment->next1(mu::engraving::SegmentType::ChordRest))) {
        const track_idx_t strack = lyrics->staffIdx() * VOICES;
        const track_idx_t etrack = strack + VOICES;
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* el = nextSegment->element(track);
            Lyrics* nextLyrics = el && el->isChord() ? toChordRest(el)->lyrics(lyrics->no(), lyrics->placement()) : nullptr;
            if (nextLyrics) {
                return nextLyrics;
            }
        }
    }
    return nullptr;
}
}
