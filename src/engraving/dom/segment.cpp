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

#include "segment.h"

#include <climits>

#include "translation.h"

#include "types/typesconv.h"

#include "rendering/score/tlayout.h"

#include "accidental.h"
#include "barline.h"
#include "beam.h"
#include "chord.h"
#include "chordrest.h"
#include "clef.h"
#include "engravingitem.h"
#include "harppedaldiagram.h"
#include "hook.h"
#include "instrchange.h"
#include "keysig.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "ornament.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "staff.h"
#include "staffstate.h"
#include "system.h"
#include "timesig.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"

#include "navigate.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Segment::subTypeName() const
{
    return subTypeName(m_segmentType);
}

const char* Segment::subTypeName(SegmentType t)
{
    switch (t) {
    case SegmentType::Invalid:               return "Invalid";
    case SegmentType::BeginBarLine:          return "BeginBarLine";
    case SegmentType::HeaderClef:            return "HeaderClef";
    case SegmentType::Clef:                  return "Clef";
    case SegmentType::KeySig:                return "Key Signature";
    case SegmentType::Ambitus:               return "Ambitus";
    case SegmentType::TimeSig:               return "Time Signature";
    case SegmentType::StartRepeatBarLine:    return "Begin Repeat";
    case SegmentType::ClefStartRepeatAnnounce:    return "Clef Repeat Start Courtesy";
    case SegmentType::KeySigStartRepeatAnnounce:  return "Key Sig Repeat Start Courtesy";
    case SegmentType::TimeSigStartRepeatAnnounce: return "Time Sig Repeat Start Courtesy";
    case SegmentType::BarLine:               return "BarLine";
    case SegmentType::Breath:                return "Breath";
    case SegmentType::ChordRest:             return "ChordRest";
    case SegmentType::ClefRepeatAnnounce:    return "Clef Repeat Courtesy";
    case SegmentType::KeySigRepeatAnnounce:  return "Key Sig Repeat Courtesy";
    case SegmentType::TimeSigRepeatAnnounce: return "Time Sig Repeat Courtesy";
    case SegmentType::EndBarLine:            return "EndBarLine";
    case SegmentType::KeySigAnnounce:        return "Key Sig Courtesy";
    case SegmentType::TimeSigAnnounce:       return "Time Sig Courtesy";
    case SegmentType::TimeTick:              return "Time tick";
    default:
        return "??";
    }
}

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Segment::setElement(track_idx_t track, EngravingItem* el)
{
    if (el) {
        el->setParent(this);
        m_elist[track] = el;
        setEmpty(false);
    } else {
        m_elist[track] = 0;
        checkEmpty();
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::removeElement(track_idx_t track)
{
    EngravingItem* el = element(track);
    if (el && el->isChordRest()) {
        ChordRest* cr = (ChordRest*)el;
        Beam* beam = cr->beam();
        if (beam) {
            beam->remove(cr);
        }
        Tuplet* tuplet = cr->tuplet();
        if (tuplet) {
            tuplet->remove(cr);
        }
    }
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(Measure* m)
    : EngravingItem(ElementType::SEGMENT, m->score(), ElementFlag::EMPTY | ElementFlag::ENABLED | ElementFlag::NOT_SELECTABLE)
{
    setParent(m);
    init();
}

Segment::Segment(Measure* m, SegmentType st, const Fraction& t)
    : EngravingItem(ElementType::SEGMENT, m->score(), ElementFlag::EMPTY | ElementFlag::ENABLED | ElementFlag::NOT_SELECTABLE)
{
    setParent(m);
//      assert(t >= Fraction(0,1));
//      assert(t <= m->ticks());
    m_segmentType = st;
    m_tick = t;
    init();
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(const Segment& s)
    : EngravingItem(s)
{
    m_segmentType        = s.m_segmentType;
    m_tick               = s.m_tick;
    m_extraLeadingSpace  = s.m_extraLeadingSpace;

    for (EngravingItem* e : s.m_annotations) {
        add(e->clone());
    }

    m_elist.reserve(s.m_elist.size());
    for (EngravingItem* e : s.m_elist) {
        EngravingItem* ne = 0;
        if (e) {
            ne = e->clone();
            ne->setParent(this);
        }
        m_elist.push_back(ne);
    }
    m_shapes  = s.m_shapes;
}

void Segment::setParent(Measure* parent)
{
    EngravingItem::setParent(parent);
}

//---------------------------------------------------------
//   setSegmentType
//---------------------------------------------------------

void Segment::setSegmentType(SegmentType t)
{
    assert(m_segmentType != SegmentType::Clef || t != SegmentType::ChordRest);
    m_segmentType = t;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Segment::setScore(Score* score)
{
    EngravingItem::setScore(score);
    for (EngravingItem* e : m_elist) {
        if (e) {
            e->setScore(score);
        }
    }
    for (EngravingItem* e : m_annotations) {
        e->setScore(score);
    }
}

Segment::~Segment()
{
    for (EngravingItem* e : m_elist) {
        if (!e) {
            continue;
        }

        Staff* staff = e->staff();

        if (e->isTimeSig() && staff) {
            staff->removeTimeSig(toTimeSig(e));
        }

        delete e;
    }

    muse::DeleteAll(m_annotations);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Segment::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Segment::init()
{
    size_t staves = score()->nstaves();
    size_t tracks = staves * VOICES;
    m_elist.assign(tracks, 0);
    m_preAppendedItems.assign(tracks, 0);
    m_shapes.assign(staves, Shape());
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Segment::tick() const
{
    return m_tick + measure()->tick();
}

//---------------------------------------------------------
//   next1
///   return next \a Segment, don’t stop searching at end
///   of \a Measure
//---------------------------------------------------------

Segment* Segment::next1() const
{
    if (next()) {
        return next();
    }
    Measure* m = measure()->nextMeasure();
    return m ? m->first() : 0;
}

//---------------------------------------------------------
//   next1enabled
//---------------------------------------------------------

Segment* Segment::next1enabled() const
{
    Segment* s = next1();
    while (s && !s->enabled()) {
        s = s->next1();
    }
    return s;
}

//---------------------------------------------------------
//   next1MM
//---------------------------------------------------------

Segment* Segment::next1MM() const
{
    if (next()) {
        return next();
    }
    Measure* m = measure()->nextMeasureMM();
    return m ? m->first() : 0;
}

Segment* Segment::next1(SegmentType types) const
{
    for (Segment* s = next1(); s; s = s->next1()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

Segment* Segment::next1ChordRestOrTimeTick() const
{
    Segment* nextSeg = next1(CHORD_REST_OR_TIME_TICK_TYPE);
    while (nextSeg && nextSeg->tick() == tick()) {
        nextSeg = nextSeg->next1(CHORD_REST_OR_TIME_TICK_TYPE);
    }
    if (!nextSeg) {
        return nullptr;
    }

    Segment* nextNextSeg = nextSeg->next1(CHORD_REST_OR_TIME_TICK_TYPE);
    if (!nextNextSeg) {
        return nextSeg;
    }

    if (nextSeg->tick() == nextNextSeg->tick()) {
        return nextSeg->isChordRestType() ? nextSeg : nextNextSeg;
    }

    return nextSeg;
}

Segment* Segment::next1WithElemsOnStaff(staff_idx_t staffIdx, SegmentType segType) const
{
    Segment* next = next1(segType);

    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES - 1;
    while (next && !next->hasElements(startTrack, endTrack)) {
        next = next->next1(segType);
    }

    return next;
}

Segment* Segment::next1MM(SegmentType types) const
{
    for (Segment* s = next1MM(); s; s = s->next1MM()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

Segment* Segment::next1MMenabled() const
{
    Segment* s = next1MM();
    while (s && !s->enabled()) {
        s = s->next1MM();
    }
    return s;
}

//---------------------------------------------------------
//   next
//    got to next segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::next(SegmentType types) const
{
    for (Segment* s = next(); s; s = s->next()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   nextInStaff
///   Returns next \c Segment in the staff with given index
//---------------------------------------------------------

Segment* Segment::nextInStaff(staff_idx_t staffIdx, SegmentType type) const
{
    Segment* s = next(type);
    const track_idx_t minTrack = staffIdx * VOICES;
    const track_idx_t maxTrack = (staffIdx + 1) * VOICES - 1;
    while (s && !s->hasElements(minTrack, maxTrack)) {
        s = s->next(type);
    }
    return s;
}

//---------------------------------------------------------
//   prev
//    got to previous segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::prev(SegmentType types) const
{
    for (Segment* s = prev(); s; s = s->prev()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   prev1
///   return previous \a Segment, don’t stop searching at
///   \a Measure begin
//---------------------------------------------------------

Segment* Segment::prev1() const
{
    if (prev()) {
        return prev();
    }
    Measure* m = measure()->prevMeasure();
    return m ? m->last() : 0;
}

Segment* Segment::prev1ChordRestOrTimeTick() const
{
    Segment* prevSeg = prev1(CHORD_REST_OR_TIME_TICK_TYPE);
    while (prevSeg && prevSeg->tick() == tick()) {
        prevSeg = prevSeg->prev1(CHORD_REST_OR_TIME_TICK_TYPE);
    }
    if (!prevSeg) {
        return nullptr;
    }

    Segment* prevPrevSeg = prevSeg->prev1(CHORD_REST_OR_TIME_TICK_TYPE);
    if (!prevPrevSeg) {
        return prevSeg;
    }

    if (prevSeg->tick() == prevPrevSeg->tick()) {
        return prevSeg->isChordRestType() ? prevSeg : prevPrevSeg;
    }

    return prevSeg;
}

Segment* Segment::prev1WithElemsOnStaff(staff_idx_t staffIdx, SegmentType segType) const
{
    Segment* prev = prev1(segType);

    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES - 1;
    while (prev && !prev->hasElements(startTrack, endTrack)) {
        prev = prev->prev1(segType);
    }

    return prev;
}

Segment* Segment::prev1enabled() const
{
    Segment* s = prev1();
    while (s && !s->enabled()) {
        s = s->prev1();
    }
    return s;
}

Segment* Segment::prev1MM() const
{
    if (prev()) {
        return prev();
    }
    Measure* m = measure()->prevMeasureMM();
    return m ? m->last() : 0;
}

Segment* Segment::prev1MMenabled() const
{
    Segment* s = prev1MM();
    while (s && !s->enabled()) {
        s = s->prev1MM();
    }
    return s;
}

Segment* Segment::prev1(SegmentType types) const
{
    for (Segment* s = prev1(); s; s = s->prev1()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

Segment* Segment::prev1MM(SegmentType types) const
{
    for (Segment* s = prev1MM(); s; s = s->prev1MM()) {
        if (s->segmentType() & types) {
            return s;
        }
    }
    return 0;
}

Segment* Segment::nextActive() const
{
    Segment* ns = next();
    while (ns && !ns->isActive()) {
        ns = ns->next();
    }
    return ns;
}

Segment* Segment::nextEnabled() const
{
    Segment* ns = next();
    while (ns && !ns->enabled()) {
        ns = ns->next();
    }
    return ns;
}

Segment* Segment::prevActive() const
{
    Segment* ps = prev();
    while (ps && !ps->isActive()) {
        ps = ps->prev();
    }
    return ps;
}

Segment* Segment::prevEnabled() const
{
    Segment* ps = prev();
    while (ps && !ps->enabled()) {
        ps = ps->prev();
    }
    return ps;
}

//---------------------------------------------------------
//   nextCR
//    get next ChordRest Segment
//---------------------------------------------------------

Segment* Segment::nextCR(track_idx_t track, bool sameStaff) const
{
    track_idx_t strack = track;
    track_idx_t etrack;
    if (sameStaff) {
        strack &= ~(VOICES - 1);
        etrack = strack + VOICES;
    } else {
        etrack = strack + 1;
    }
    for (Segment* seg = next1(); seg; seg = seg->next1()) {
        if (seg->isChordRestType()) {
            if (track == muse::nidx) {
                return seg;
            }
            for (track_idx_t t = strack; t < etrack; ++t) {
                if (seg->element(t)) {
                    return seg;
                }
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   nextChordRest
//    get the next ChordRest, start at this segment
//---------------------------------------------------------

ChordRest* Segment::nextChordRest(track_idx_t track, bool backwards, bool stopAtMeasureBoundary) const
{
    const Segment* seg = this;
    while (seg) {
        EngravingItem* el = seg->element(track);
        if (el && el->isChordRest()) {
            return toChordRest(el);
        }

        if (backwards) {
            seg = stopAtMeasureBoundary ? seg->prev() : seg->prev1();
            continue;
        }

        seg = stopAtMeasureBoundary ? seg->next() : seg->next1();
    }
    return nullptr;
}

EngravingItem* Segment::element(track_idx_t track) const
{
    if (track >= m_elist.size()) {
        return nullptr;
    }

    return m_elist[track];
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(staff_idx_t staff)
{
    track_idx_t track = staff * VOICES;
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        m_elist.insert(m_elist.begin() + track, 0);
        m_preAppendedItems.insert(m_preAppendedItems.begin() + track, 0);
    }
    m_shapes.insert(m_shapes.begin() + staff, Shape());

    for (EngravingItem* e : m_annotations) {
        if (moveDownWhenAddingStaves(e, staff)) {
            e->setTrack(e->track() + VOICES);
        }
    }
    fixStaffIdx();
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(staff_idx_t staff)
{
    track_idx_t track = staff * VOICES;
    m_elist.erase(m_elist.begin() + track, m_elist.begin() + track + VOICES);
    m_preAppendedItems.erase(m_preAppendedItems.begin() + track, m_preAppendedItems.begin() + track + VOICES);
    m_shapes.erase(m_shapes.begin() + staff);

    for (EngravingItem* e : m_annotations) {
        staff_idx_t staffIdx = e->staffIdx();
        if (staffIdx > staff) {
            e->setTrack(e->track() - VOICES);
        }
    }

    fixStaffIdx();
}

//---------------------------------------------------------
//   checkElement
//---------------------------------------------------------

void Segment::checkElement(EngravingItem* el, track_idx_t track)
{
    // prevent segmentation fault on out of bounds index
    IF_ASSERT_FAILED(track < m_elist.size()) {
        return;
    }
    // generated elements can be overwritten
    if (m_elist[track] && !m_elist[track]->generated()) {
        LOGD("add(%s): there is already a %s at track %zu tick %d",
             el->typeName(),
             m_elist[track]->typeName(),
             track,
             tick().ticks()
             );
//            abort();
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(EngravingItem* el)
{
//      LOGD("%p segment %s add(%d, %d, %s)", this, subTypeName(), tick(), el->track(), el->typeName());

    if (el->explicitParent() != this) {
        el->setParent(this);
    }

    track_idx_t track = el->track();
    assert(track != muse::nidx);
    assert(el->score() == score());
    assert(score()->nstaves() * VOICES == m_elist.size());
    // make sure offset is correct for staff
    if (el->isStyled(Pid::OFFSET)) {
        el->setOffset(el->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    switch (el->type()) {
    case ElementType::MEASURE_REPEAT:
        m_elist[track] = el;
        setEmpty(false);
        break;

    case ElementType::TEMPO_TEXT:
    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::HARMONY:
    case ElementType::SYMBOL:
    case ElementType::FRET_DIAGRAM:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::FERMATA:
    case ElementType::STICKING:
    case ElementType::PARENTHESIS:
        m_annotations.push_back(el);
        break;

    case ElementType::STRING_TUNINGS: {
        m_annotations.push_back(el);
        el->part()->addStringTunings(toStringTunings(el));
        break;
    }

    case ElementType::STAFF_STATE:
        if (toStaffState(el)->staffStateType() == StaffStateType::INSTRUMENT) {
            StaffState* ss = toStaffState(el);
            Part* part = el->part();
            part->setInstrument(ss->instrument(), tick());
        }
        m_annotations.push_back(el);
        break;

    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* is = toInstrumentChange(el);
        Part* part = is->part();
        part->setInstrument(is->instrument(), tick());
        m_annotations.push_back(el);
        break;
    }

    case ElementType::HARP_DIAGRAM:
        // already a diagram in this segment
        if (el->part()->harpDiagrams.count(toHarpPedalDiagram(el)->segment()->tick().ticks()) > 0) {
            break;
        }
        el->part()->addHarpDiagram(toHarpPedalDiagram(el));
        m_annotations.push_back(el);
        break;

    case ElementType::CLEF:
        assert(m_segmentType & SegmentType::ClefType);
        checkElement(el, track);
        m_elist[track] = el;
        if (!el->generated()) {
            el->staff()->setClef(toClef(el));
        }
        setEmpty(false);
        break;

    case ElementType::TIMESIG:
        assert(segmentType() & SegmentType::TimeSigType);
        checkElement(el, track);
        m_elist[track] = el;
        el->staff()->addTimeSig(toTimeSig(el));
        setEmpty(false);
        if (segmentType() & SegmentType::CourtesyTimeSigType) {
            toTimeSig(el)->setIsCourtesy(true);
        }
        break;

    case ElementType::KEYSIG:
        assert(m_segmentType & SegmentType::KeySigType);
        checkElement(el, track);
        m_elist[track] = el;
        if (!el->generated()) {
            el->staff()->setKey(tick(), toKeySig(el)->keySigEvent());
        }
        setEmpty(false);
        if (m_segmentType == SegmentType::CourtesyKeySigType) {
            toKeySig(el)->setIsCourtesy(true);
        }
        break;

    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::MMREST:
        assert(m_segmentType == SegmentType::ChordRest);
        {
            if (track % VOICES) {
                bool v;
                if (el->isChord()) {
                    v = false;
                    // consider chord visible if any note is visible
                    Chord* c = toChord(el);
                    for (Note* n : c->notes()) {
                        if (n->visible()) {
                            v = true;
                            break;
                        }
                    }
                } else {
                    v = el->visible();
                }

                if (v && measure()->score()->ntracks() > track) {
                    measure()->setHasVoices(track / VOICES, true);
                }
            }
            score()->setPlaylistDirty();
        }
    // fall through

    case ElementType::BAR_LINE:
    case ElementType::BREATH:
        if (track < score()->nstaves() * VOICES) {
            checkElement(el, track);
            m_elist[track] = el;
        }
        setEmpty(false);
        break;

    case ElementType::AMBITUS:
        assert(m_segmentType == SegmentType::Ambitus);
        checkElement(el, track);
        m_elist[track] = el;
        setEmpty(false);
        break;

    case ElementType::TIME_TICK_ANCHOR:
        assert(m_segmentType == SegmentType::TimeTick);
        m_elist[track] = el;
        setEmpty(false);
        break;

    default:
        ASSERT_X(String(u"Segment::add() unknown %1").arg(String::fromAscii(el->typeName())));
        return;
    }

    el->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(EngravingItem* el)
{
// LOGD("%p Segment::remove %s %p", this, el->typeName(), el);

    track_idx_t track = el->track();

    switch (el->type()) {
    case ElementType::CHORD:
    case ElementType::REST:
    {
        m_elist[track] = 0;
        staff_idx_t staffIdx = el->staffIdx();
        measure()->checkMultiVoices(staffIdx);
        // spanners with this cr as start or end element will need relayout
        SpannerMap& smap = score()->spannerMap();
        auto spanners = smap.findOverlapping(tick().ticks(), tick().ticks());
        for (auto interval : spanners) {
            Spanner* s = interval.value;
            EngravingItem* start = s->startElement();
            EngravingItem* end = s->endElement();
            if (s->startElement() == el) {
                start = nullptr;
            }
            if (s->endElement() == el) {
                end = nullptr;
            }
            if (start != s->startElement() || end != s->endElement()) {
                score()->undo(new ChangeStartEndSpanner(s, start, end));
            }
        }
        score()->setPlaylistDirty();
    }
    break;

    case ElementType::MMREST:
    case ElementType::MEASURE_REPEAT:
    case ElementType::TIME_TICK_ANCHOR:
        m_elist[track] = 0;
        break;

    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::FIGURED_BASS:
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARMONY:
    case ElementType::IMAGE:
    case ElementType::MARKER:
    case ElementType::REHEARSAL_MARK:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::SYMBOL:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::TEMPO_TEXT:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::FERMATA:
    case ElementType::STICKING:
    case ElementType::PARENTHESIS:
        removeAnnotation(el);
        break;

    case ElementType::STRING_TUNINGS:
        el->part()->removeStringTunings(toStringTunings(el));
        removeAnnotation(el);
        break;

    case ElementType::STAFF_STATE:
        if (toStaffState(el)->staffStateType() == StaffStateType::INSTRUMENT) {
            Part* part = el->part();
            part->removeInstrument(tick());
        }
        removeAnnotation(el);
        break;

    case ElementType::INSTRUMENT_CHANGE:
    {
        InstrumentChange* is = toInstrumentChange(el);
        Part* part = is->part();
        part->removeInstrument(tick());
    }
        removeAnnotation(el);
        break;

    case ElementType::HARP_DIAGRAM:
        el->part()->removeHarpDiagram(toHarpPedalDiagram(el));
        removeAnnotation(el);
        break;

    case ElementType::TIMESIG:
        m_elist[track] = 0;
        el->staff()->removeTimeSig(toTimeSig(el));
        break;

    case ElementType::KEYSIG:
        m_elist[track] = 0;
        if (!el->generated()) {
            el->staff()->removeKey(tick());
        }
        break;

    case ElementType::CLEF:
        el->staff()->removeClef(toClef(el));
    // updateNoteLines(this, el->track());
    // fall through

    case ElementType::BAR_LINE:
    case ElementType::AMBITUS:
        m_elist[track] = 0;
        break;

    case ElementType::BREATH:
        m_elist[track] = 0;
        score()->setPause(tick(), 0);
        break;

    default:
        ASSERT_X(String(u"Segment::remove() unknown %1").arg(String::fromAscii(el->typeName())));
        return;
    }
    triggerLayout();
    checkEmpty();
    el->removed();
}

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of EngravingItem
//---------------------------------------------------------

SegmentType Segment::segmentType(ElementType type)
{
    switch (type) {
    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::MMREST:
    case ElementType::MEASURE_REPEAT:
    case ElementType::JUMP:
    case ElementType::MARKER:
        return SegmentType::ChordRest;
    case ElementType::CLEF:
        return SegmentType::Clef;
    case ElementType::KEYSIG:
        return SegmentType::KeySig;
    case ElementType::TIMESIG:
        return SegmentType::TimeSig;
    case ElementType::BAR_LINE:
        return SegmentType::StartRepeatBarLine;
    case ElementType::BREATH:
        return SegmentType::Breath;
    default:
        LOGD("Segment:segmentType():  bad type: <%s>", TConv::toXml(type).ascii());
        return SegmentType::Invalid;
    }
}

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Segment::sortStaves(std::vector<staff_idx_t>& dst)
{
    std::vector<EngravingItem*> dl;
    dl.reserve(dst.size());

    for (staff_idx_t i = 0; i < dst.size(); ++i) {
        track_idx_t startTrack = dst[i] * VOICES;
        track_idx_t endTrack   = startTrack + VOICES;
        for (track_idx_t k = startTrack; k < endTrack; ++k) {
            dl.push_back(m_elist[k]);
        }
    }
    std::swap(m_elist, dl);
    std::map<staff_idx_t, staff_idx_t> map;
    for (staff_idx_t k = 0; k < dst.size(); ++k) {
        map.insert({ dst[k], k });
    }
    for (EngravingItem* e : m_annotations) {
        if (e->isTopSystemObject()) {
            continue;
        }
        e->setTrack(map[e->staffIdx()] * VOICES + e->voice());
    }
    fixStaffIdx();
}

//---------------------------------------------------------
//   fixStaffIdx
//---------------------------------------------------------

void Segment::fixStaffIdx()
{
    int track = 0;
    for (EngravingItem* e : m_elist) {
        if (e) {
            e->setTrack(track);
        }
        ++track;
    }
}

//---------------------------------------------------------
//   checkEmpty
//---------------------------------------------------------

void Segment::checkEmpty() const
{
    if (!m_annotations.empty()) {
        setEmpty(false);
        return;
    }
    setEmpty(true);
    for (const EngravingItem* e : m_elist) {
        if (e) {
            setEmpty(false);
            break;
        }
    }
}

double Segment::xPosInSystemCoords() const
{
    return ldata()->pos().x() + measure()->x();
}

void Segment::setXPosInSystemCoords(double x)
{
    mutldata()->setPosX(x - measure()->x());
}

bool Segment::isTupletSubdivision() const
{
    int denom = tick().reduced().denominator();
    bool denomIsPowOfTwo = (denom & (denom - 1)) == 0;
    // A non-power-of-two denominator is possible only with tuplets
    return !denomIsPowOfTwo;
}

bool Segment::isInsideTupletOnStaff(staff_idx_t staffIdx) const
{
    const Segment* refCRSeg = isChordRestType() && hasElements(staffIdx) ? this : prev1WithElemsOnStaff(staffIdx, SegmentType::ChordRest);
    if (!refCRSeg || refCRSeg->measure() != measure()) {
        return false;
    }

    track_idx_t startTrack = staff2track(staffIdx);
    track_idx_t endTrack = startTrack + VOICES;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        ChordRest* chordRest = toChordRest(refCRSeg->elementAt(track));
        if (chordRest && chordRest->tuplet() && tick() != chordRest->topTuplet()->tick()) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   swapElements
//---------------------------------------------------------

void Segment::swapElements(track_idx_t i1, track_idx_t i2)
{
    std::iter_swap(m_elist.begin() + i1, m_elist.begin() + i2);
    if (m_elist[i1]) {
        m_elist[i1]->setTrack(i1);
    }
    if (m_elist[i2]) {
        m_elist[i2]->setTrack(i2);
    }
    triggerLayout();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Segment::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TICK:
        return m_tick;
    case Pid::LEADING_SPACE:
        return extraLeadingSpace();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Segment::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LEADING_SPACE:
        return Spatium(0.0);
    default:
        return EngravingItem::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Segment::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::TICK:
        setRtick(v.value<Fraction>());
        break;
    case Pid::LEADING_SPACE:
        setExtraLeadingSpace(v.value<Spatium>());
        for (EngravingItem* e : m_elist) {
            if (e) {
                e->setGenerated(false);
            }
        }
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   widthInStaff
//---------------------------------------------------------

double Segment::widthInStaff(staff_idx_t staffIdx, SegmentType nextSegType) const
{
    const double segX = x();
    double nextSegX = segX;

    Segment* nextSeg = nextInStaff(staffIdx, nextSegType);
    if (nextSeg) {
        nextSegX = nextSeg->x();
    } else {
        Segment* lastSeg = measure()->lastEnabled();
        if (lastSeg->segmentType() & nextSegType) {
            nextSegX = lastSeg->x() + lastSeg->width();
        } else {
            nextSegX = lastSeg->x();
        }
    }

    return nextSegX - segX;
}

//---------------------------------------------------------
//   ticksInStaff
//---------------------------------------------------------

Fraction Segment::ticksInStaff(staff_idx_t staffIdx) const
{
    const Fraction segTick = tick();
    Fraction nextSegTick = segTick;

    Segment* nextSeg = nextInStaff(staffIdx, durationSegmentsMask);
    if (nextSeg) {
        nextSegTick = nextSeg->tick();
    } else {
        Segment* lastSeg = measure()->last();
        nextSegTick = lastSeg->tick() + lastSeg->ticks();
    }

    return nextSegTick - segTick;
}

//---------------------------------------------------------
//   splitsTuplet
//---------------------------------------------------------

bool Segment::splitsTuplet() const
{
    for (EngravingItem* e : m_elist) {
        if (!(e && e->isChordRest())) {
            continue;
        }
        ChordRest* cr = toChordRest(e);
        Tuplet* t = cr->tuplet();
        while (t) {
            if (cr != t->elements().front()) {
                return true;
            }
            t = t->tuplet();
        }
    }
    return false;
}

//---------------------------------------------------------
//   operator<
///   return true if segment is before s in list
//---------------------------------------------------------

bool Segment::operator<(const Segment& s) const
{
    if (tick() < s.tick()) {
        return true;
    }
    if (tick() > s.tick()) {
        return false;
    }
    for (Segment* ns = next1(); ns && (ns->tick() == s.tick()); ns = ns->next1()) {
        if (ns == &s) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   operator>
///   return true if segment is after s in list
//---------------------------------------------------------

bool Segment::operator>(const Segment& s) const
{
    if (tick() > s.tick()) {
        return true;
    }
    if (tick() < s.tick()) {
        return false;
    }
    for (Segment* ns = prev1(); ns && (ns->tick() == s.tick()); ns = ns->prev1()) {
        if (ns == &s) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasElements
///  Returns true if the segment has at least one element.
///  Annotations are not considered.
//---------------------------------------------------------

bool Segment::hasElements() const
{
    for (const EngravingItem* e : m_elist) {
        if (e) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasElements
///  return true if an annotation of type type or and element is found in the track range
//---------------------------------------------------------

bool Segment::hasElements(track_idx_t minTrack, track_idx_t maxTrack) const
{
    for (track_idx_t curTrack = minTrack; curTrack <= maxTrack; curTrack++) {
        if (element(curTrack)) {
            return true;
        }
    }
    return false;
}

bool Segment::hasElements(staff_idx_t staffIdx) const
{
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES - 1;
    return hasElements(startTrack, endTrack);
}

//---------------------------------------------------------
//   allElementsInvisible
///  return true if all elements in the segment are invisible
//---------------------------------------------------------

bool Segment::allElementsInvisible() const
{
    if (isType(SegmentType::TimeTick)) {
        return true;
    }

    if (isType(SegmentType::BarLineType | SegmentType::ChordRest)) {
        return false;
    }

    System* sys = system();
    for (staff_idx_t staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
        Staff* staff = score()->staves().at(staffIdx);
        if (!staff->visible()) {
            continue;
        }
        if (sys && staffIdx < sys->staves().size() && !sys->staves().at(staffIdx)->show()) {
            continue;
        }
        track_idx_t endTrack = staffIdx * VOICES + VOICES;
        for (track_idx_t track = staffIdx * VOICES; track < endTrack; ++track) {
            EngravingItem* e = m_elist[track];
            if (e && e->visible() && !muse::RealIsEqual(e->width(), 0.0)) {
                return false;
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   hasAnnotationOrElement
///  return true if an annotation of type type or and element is found in the track range
//---------------------------------------------------------

bool Segment::hasAnnotationOrElement(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const
{
    for (const EngravingItem* e : m_annotations) {
        if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack) {
            return true;
        }
    }
    return hasElements(minTrack, maxTrack);
}

//---------------------------------------------------------
//   findAnnotation
///  Returns the first found annotation of type type
///  or nullptr if nothing was found.
//---------------------------------------------------------

EngravingItem* Segment::findAnnotation(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const
{
    for (EngravingItem* e : m_annotations) {
        if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack) {
            return e;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   findAnnotations
///  Returns the list of found annotations
///  or nullptr if nothing was found.
//---------------------------------------------------------

std::vector<EngravingItem*> Segment::findAnnotations(ElementType type, track_idx_t minTrack, track_idx_t maxTrack) const
{
    std::vector<EngravingItem*> found;
    for (EngravingItem* e : m_annotations) {
        if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack) {
            found.push_back(e);
        }
    }
    return found;
}

//---------------------------------------------------------
//   removeAnnotation
//---------------------------------------------------------

void Segment::removeAnnotation(EngravingItem* e)
{
    for (auto i = m_annotations.begin(); i != m_annotations.end(); ++i) {
        if (*i == e) {
            m_annotations.erase(i);
            break;
        }
    }
}

//---------------------------------------------------------
//   clearAnnotations
//---------------------------------------------------------

void Segment::clearAnnotations()
{
    m_annotations.clear();
}

//---------------------------------------------------------
//   elementAt
//    A variant of the element(int) function,
//    specifically intended to be called from QML plugins
//---------------------------------------------------------

EngravingItem* Segment::elementAt(track_idx_t track) const
{
    EngravingItem* e = track < m_elist.size() ? m_elist[track] : 0;
    return e;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Segment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    bool scanAllTimeSigs = (isType(SegmentType::TimeSigType)
                            && style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() != TimeSigPlacement::NORMAL);
    for (size_t track = 0; track < score()->nstaves() * VOICES; ++track) {
        size_t staffIdx = track / VOICES;
        bool thisMeasureVisible = measure()->visible(staffIdx) && score()->staff(staffIdx)->show();
        if (!all && !scanAllTimeSigs && !thisMeasureVisible) {
            Measure* nextMeasure = measure()->nextMeasure();
            bool nextMeasureVisible = nextMeasure
                                      && nextMeasure->system() == measure()->system()
                                      && nextMeasure->visible(staffIdx)
                                      && score()->staff(staffIdx)->show();
            if (!((isEndBarLineType() && (nextMeasureVisible || measure()->isCutawayClef(staffIdx)))
                  || (isClefType() && measure()->isCutawayClef(staffIdx)))) {
                track += VOICES - 1;
                continue;
            }
        }
        EngravingItem* e = element(track);
        if (e == 0) {
            continue;
        }
        e->scanElements(data, func, all);
    }
    for (EngravingItem* e : annotations()) {
        if (all || e->systemFlag() || measure()->visible(e->staffIdx())) {
            e->scanElements(data,  func, all);
        }
    }
}

RectF Segment::contentRect() const
{
    RectF result;
    for (const EngravingItem* element: elist()) {
        if (!element) {
            continue;
        }

        if (element->isChord()) {
            const Chord* chord = dynamic_cast<const Chord*>(element);
            for (const Note* note: chord->notes()) {
                result = result.united(note->ldata()->bbox());
            }

            Hook* hook = chord->hook();
            if (hook) {
                RectF rect = RectF(hook->pos().x(), hook->pos().y(), hook->width(), hook->height());
                result = result.united(rect);
            }

            continue;
        }

        result = result.united(element->ldata()->bbox());
    }

    return result;
}

//---------------------------------------------------------
//   firstElement
//   This function returns the first main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

EngravingItem* Segment::firstElementForNavigation(staff_idx_t staff) const
{
    if (isChordRestType()) {
        track_idx_t strack = staff * VOICES;
        track_idx_t etrack = strack + VOICES;
        for (track_idx_t v = strack; v < etrack; ++v) {
            EngravingItem* el = element(v);
            if (!el) {
                continue;
            }
            return el->isChord() ? toChord(el)->notes().back() : el;
        }
    } else {
        return getElement(staff);
    }
    return 0;
}

//---------------------------------------------------------
//   lastElement
//   This function returns the last main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

EngravingItem* Segment::lastElementForNavigation(staff_idx_t staff) const
{
    if (segmentType() == SegmentType::ChordRest) {
        for (int voice = static_cast<int>(staff * VOICES + (VOICES - 1)); voice / static_cast<int>(VOICES) == static_cast<int>(staff);
             voice--) {
            EngravingItem* el = element(voice);
            if (!el) {            //there is no chord or rest on this voice
                continue;
            }
            if (el->isChord()) {
                return toChord(el)->notes().front();
            } else {
                return el;
            }
        }
    } else {
        return getElement(staff);
    }

    return 0;
}

//---------------------------------------------------------
//   getElement
//   protected because it is used by the firstElement and
//   lastElement functions when segment types that have
//   just one element to avoid duplicated code
//
//   Use firstElement, or lastElement instead of this
//---------------------------------------------------------

EngravingItem* Segment::getElement(staff_idx_t staff) const
{
    segmentType();
    if (segmentType() == SegmentType::ChordRest) {
        return firstElementForNavigation(staff);
    } else if (segmentType() & (SegmentType::EndBarLine | SegmentType::BarLine | SegmentType::StartRepeatBarLine)) {
        for (int i = static_cast<int>(staff); i >= 0; i--) {
            if (!element(i * VOICES)) {
                continue;
            }
            BarLine* b = toBarLine(element(i * VOICES));
            if (i + b->spanStaff() >= static_cast<int>(staff)) {
                return element(i * VOICES);
            }
        }
    } else {
        return element(staff * VOICES);
    }
    return 0;
}

//---------------------------------------------------------
//   nextAnnotation
//   return next element in _annotations
//---------------------------------------------------------

EngravingItem* Segment::nextAnnotation(EngravingItem* e) const
{
    if (m_annotations.empty() || e == m_annotations.back()) {
        return nullptr;
    }
    auto ei = std::find(m_annotations.begin(), m_annotations.end(), e);
    if (ei == m_annotations.end()) {
        return nullptr;                   // element not found
    }
    // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
    auto resIt = std::find_if(ei + 1, m_annotations.end(), [e](EngravingItem* nextElem) {
        return nextElem && nextElem->staffIdx() == e->staffIdx();
    });

    return m_annotations.end() == resIt ? nullptr : *resIt;
}

//---------------------------------------------------------
//   prevAnnotation
//   return previous element in _annotations
//---------------------------------------------------------

EngravingItem* Segment::prevAnnotation(EngravingItem* e) const
{
    if (e == m_annotations.front()) {
        return nullptr;
    }
    auto reverseIt = std::find(m_annotations.rbegin(), m_annotations.rend(), e);
    if (reverseIt == m_annotations.rend()) {
        return nullptr;                   // element not found
    }
    // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
    auto resIt = std::find_if(reverseIt + 1, m_annotations.rend(), [e](EngravingItem* prevElem) {
        return prevElem && prevElem->staffIdx() == e->staffIdx();
    });

    return m_annotations.rend() == resIt ? nullptr : *resIt;
}

//---------------------------------------------------------
//   firstAnnotation
//---------------------------------------------------------

EngravingItem* Segment::firstAnnotation(staff_idx_t activeStaff) const
{
    for (auto i = m_annotations.begin(); i != m_annotations.end(); ++i) {
        // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
        if ((*i)->staffIdx() == activeStaff) {
            return *i;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   lastAnnotation
//---------------------------------------------------------

EngravingItem* Segment::lastAnnotation(staff_idx_t activeStaff) const
{
    for (auto i = m_annotations.rbegin(); i != m_annotations.rend(); ++i) {
        // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
        EngravingItem* e = *i;
        IF_ASSERT_FAILED(e) {
            continue;
        }
        if (e->staffIdx() == activeStaff) {
            return e;
        }
    }
    return nullptr;
}

//--------------------------------------------------------
//   firstInNextSegments
//   Searches for the next segment that has elements on the
//   active staff and returns its first element
//
//   Uses firstElement so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

EngravingItem* Segment::firstInNextSegments(staff_idx_t activeStaff) const
{
    EngravingItem* re = 0;
    const Segment* seg = this;
    while (!re) {
        seg = seg->next1MMenabled();
        if (!seg) {   //end of staff, or score
            break;
        }

        re = seg->firstElementForNavigation(activeStaff);
    }

    if (re) {
        return re;
    }

    if (!seg) {   //end of staff
        if (activeStaff + 1 >= score()->nstaves()) {   //end of score
            return 0;
        }
        seg = score()->firstSegmentMM(SegmentType::All);
        return seg->element((activeStaff + 1) * VOICES);
    }
    return 0;
}

//---------------------------------------------------------
//   firstElementOfSegment
//   returns the first non null element
//---------------------------------------------------------

EngravingItem* Segment::firstElementOfSegment(staff_idx_t activeStaff) const
{
    if (isTimeTickType()) {
        return nullptr;
    }

    for (auto i: m_elist) {
        if (i && i->staffIdx() == activeStaff) {
            if (i->isDurationElement()) {
                DurationElement* de = toDurationElement(i);
                Tuplet* tuplet = de->tuplet();
                if (tuplet && de == tuplet->elements().front()) {
                    return tuplet;
                }
            }

            if (i->type() == ElementType::CHORD) {
                Chord* chord = toChord(i);
                return chord->firstGraceOrNote();
            } else {
                return i;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   nextElementOfSegment
//   returns the next element after the given element
//---------------------------------------------------------

EngravingItem* Segment::nextElementOfSegment(EngravingItem* e, staff_idx_t activeStaff) const
{
    for (size_t track = 0; track < score()->nstaves() * VOICES - 1; ++track) {
        if (element(track) == 0) {
            continue;
        }
        EngravingItem* el = element(track);
        if (el == e) {
            EngravingItem* next = element(track + 1);
            while (track < score()->nstaves() * VOICES - 1
                   && (!next || next->staffIdx() != activeStaff)) {
                next = element(++track);
            }
            if (!next || next->staffIdx() != activeStaff) {
                return nullptr;
            }
            if (next->isChord()) {
                return toChord(next)->notes().back();
            } else {
                return next;
            }
        }
        if (el->type() == ElementType::CHORD) {
            std::vector<Note*> notes = toChord(el)->notes();
            auto i = std::find(notes.begin(), notes.end(), e);
            if (i == notes.end()) {
                continue;
            }
            if (i != notes.begin()) {
                return *(i - 1);
            } else {
                EngravingItem* nextEl = element(++track);
                while (track < score()->nstaves() * VOICES - 1
                       && (!nextEl || nextEl->staffIdx() != activeStaff)) {
                    nextEl = element(++track);
                }
                if (!nextEl || nextEl->staffIdx() != activeStaff) {
                    return nullptr;
                }
                if (nextEl->isChord()) {
                    return toChord(nextEl)->notes().back();
                }
                return nextEl;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   prevElementOfSegment
//   returns the previous element before the given element
//---------------------------------------------------------

EngravingItem* Segment::prevElementOfSegment(EngravingItem* e, staff_idx_t activeStaff) const
{
    for (int track = static_cast<int>(score()->nstaves() * VOICES) - 1; track > 0; --track) {
        if (element(track) == 0) {
            continue;
        }
        EngravingItem* el = element(track);
        if (el == e) {
            EngravingItem* prev = element(track - 1);
            while (track > 0
                   && (!prev || prev->staffIdx() != activeStaff)) {
                prev = element(--track);
            }
            if (!prev) {
                return nullptr;
            }
            if (prev->staffIdx() == e->staffIdx()) {
                if (prev->isChord()) {
                    return toChord(prev)->notes().front();
                } else {
                    return prev;
                }
            }
            return nullptr;
        }
        if (el->isChord()) {
            Chord* chord = toChord(el);
            GraceNotesGroup& graceNotesBefore = chord->graceNotesBefore();
            if (!graceNotesBefore.empty()) {
                ChordRest* next = prevChordRest(chord);
                if (next) {
                    if (next->isChord()) {
                        return toChord(next)->notes().back();
                    }
                    return toRest(next);
                }
            }

            std::vector<Note*> notes = chord->notes();
            auto i = std::find(notes.begin(), notes.end(), e);
            if (i == notes.end()) {
                continue;
            }
            if (i != --notes.end()) {
                return *(i + 1);
            } else {
                EngravingItem* prevEl = element(--track);
                while (track > 0
                       && (!prevEl || prevEl->staffIdx() != activeStaff)) {
                    prevEl = element(--track);
                }
                if (!prevEl) {
                    return nullptr;
                }
                if (prevEl->staffIdx() == e->staffIdx()) {
                    if (prevEl->isChord()) {
                        return toChord(prevEl)->notes().front();
                    }
                    return prevEl;
                }
                return nullptr;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   lastElementOfSegment
//   returns the last element
//---------------------------------------------------------

EngravingItem* Segment::lastElementOfSegment(staff_idx_t activeStaff) const
{
    if (isTimeTickType()) {
        return nullptr;
    }

    const std::vector<EngravingItem*>& elements = m_elist;
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        EngravingItem* item = *it;
        if (item && item->staffIdx() == activeStaff) {
            if (item->isChord()) {
                Chord* chord = toChord(item);
                if (!chord->graceNotesAfter().empty()) {
                    return chord->graceNotesAfter().back()->notes().back();
                }

                const std::vector<Articulation*>& articulations = chord->articulations();
                if (!articulations.empty()) {
                    return articulations.back();
                }
                return chord->upNote();
            }
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   firstSpanner
//---------------------------------------------------------

Spanner* Segment::firstSpanner(staff_idx_t activeStaff) const
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {  // range not empty
        for (auto i = range.first; i != range.second; ++i) {
            Spanner* s = i->second;
            if (s->segmentsEmpty()) {
                continue;
            }

            EngravingItem* e = s->startElement();
            if (!e) {
                continue;
            }
            if (s->startSegment() == this) {
                if (e->staffIdx() == activeStaff || (e->isMeasure() && activeStaff == 0)) {
                    return s;
                }
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   lastSpanner
//---------------------------------------------------------

Spanner* Segment::lastSpanner(staff_idx_t activeStaff) const
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {  // range not empty
        for (auto i = --range.second;; --i) {
            Spanner* s = i->second;
            if (s->segmentsEmpty()) {
                continue;
            }

            EngravingItem* e = s->startElement();
            if (!e) {
                continue;
            }
            if (s->startSegment() == this) {
                if (e->staffIdx() == activeStaff || (e->isMeasure() && activeStaff == 0)) {
                    return s;
                }
            }
            if (i == range.first) {
                break;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   notChordRestType
//---------------------------------------------------------

bool Segment::notChordRestType() const
{
    return segmentType() != SegmentType::ChordRest;
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* Segment::nextElement(staff_idx_t activeStaff)
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }
    if (!e) {
        return nullptr;
    }
    switch (e->type()) {
    case ElementType::DYNAMIC:
    case ElementType::HARMONY:
    case ElementType::EXPRESSION:
    case ElementType::SYMBOL:
    case ElementType::FERMATA:
    case ElementType::FRET_DIAGRAM:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::STRING_TUNINGS:
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::IMAGE:
    case ElementType::PARENTHESIS:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::HARP_DIAGRAM:
    case ElementType::STICKING:
    case ElementType::TUPLET: {
        EngravingItem* next = nullptr;
        if (e->explicitParent() == this) {
            next = nextAnnotation(e);
        }
        if (next) {
            return next;
        } else {
            Spanner* s = firstSpanner(activeStaff);
            if (s) {
                return s->spannerSegments().front();
            }
        }
        Segment* nextSegment = this->next1MMenabled();
        while (nextSegment) {
            EngravingItem* nextEl = nextSegment->firstElementOfSegment(activeStaff);
            if (nextEl) {
                return nextEl;
            }
            nextSegment = nextSegment->next1MMenabled();
        }
        break;
    }
    case ElementType::SEGMENT: {
        if (!m_annotations.empty()) {
            EngravingItem* next = firstAnnotation(activeStaff);
            if (next) {
                return next;
            }
        }
        Spanner* sp = firstSpanner(activeStaff);
        if (sp) {
            return sp->spannerSegments().front();
        }

        Segment* nextSegment = this->next1MMenabled();
        while (nextSegment) {
            EngravingItem* nextEl = nextSegment->firstElementOfSegment(activeStaff);
            if (nextEl) {
                return nextEl;
            }
            nextSegment = nextSegment->next1MMenabled();
        }
        break;
    }
    default: {
        EngravingItem* p;
        if (e->isTieSegment() || e->isGlissandoSegment() || e->isGuitarBendSegment()) {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            p = sp->startElement();
        } else {
            p = e;
            EngravingItem* pp = p->parentItem();
            if (pp->isNote() || pp->isRest() || (pp->isChord() && !p->isNote())) {
                p = pp;
            }
        }
        EngravingItem* el = p;
        for (; p && p->type() != ElementType::SEGMENT; p = p->parentItem()) {
        }
        Segment* seg = toSegment(p);
        // next in _elist
        EngravingItem* nextEl = seg->nextElementOfSegment(el, activeStaff);
        if (nextEl) {
            return nextEl;
        }
        if (!m_annotations.empty()) {
            EngravingItem* next = seg->firstAnnotation(activeStaff);
            if (next) {
                if (next->isStaffText()) {
                    if (SoundFlag* soundFlag = toStaffText(next)->soundFlag()) {
                        return soundFlag;
                    }
                }

                return next;
            }
        }
        Spanner* s = firstSpanner(activeStaff);
        if (s) {
            return s->spannerSegments().front();
        }
        Segment* nextSegment = seg->next1MMenabled();
        for (; nextSegment && nextSegment->isTimeTickType(); nextSegment = nextSegment->next1MMenabled()) {
            if (EngravingItem* annotation = nextSegment->firstAnnotation(activeStaff)) {
                return annotation;
            }
            if (Spanner* spanner = nextSegment->firstSpanner(activeStaff)) {
                return spanner->spannerSegments().front();
            }
        }
        if (!nextSegment) {
            MeasureBase* mb = measure()->next();
            return mb && mb->isBox() ? mb : score()->lastElement();
        }

        Measure* nsm = nextSegment->measure();
        if (nsm != measure()) {
            // check for frame, measure elements
            MeasureBase* nmb = measure()->nextMM();
            EngravingItem* nme = nsm->el().empty() ? nullptr : nsm->el().front();
            if (nsm != nmb) {
                return nmb;
            } else if (nme && nme->isTextBase() && nme->staffIdx() == e->staffIdx()) {
                return nme;
            } else if (nme && nme->isLayoutBreak() && e->staffIdx() == 0) {
                return nme;
            } else if (nmb->isEndOfSystemLock()) {
                System* system = nmb->system();
                SystemLockIndicator* lockInd = system ? system->lockIndicators().front() : nullptr;
                if (lockInd) {
                    return lockInd;
                }
            }
        }

        while (nextSegment) {
            nextEl = nextSegment->firstElementOfSegment(activeStaff);
            if (nextEl) {
                return nextEl;
            }
            nextSegment = nextSegment->next1MMenabled();
        }
    }
    break;
    }
    return nullptr;
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* Segment::prevElement(staff_idx_t activeStaff)
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().back();
    }
    if (!e) {
        return nullptr;
    }
    switch (e->type()) {
    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::HARMONY:
    case ElementType::SYMBOL:
    case ElementType::FERMATA:
    case ElementType::FRET_DIAGRAM:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SOUND_FLAG:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::CAPO:
    case ElementType::STRING_TUNINGS:
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::PARENTHESIS:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::HARP_DIAGRAM:
    case ElementType::STICKING: {
        if (e->isStaffText()) {
            if (SoundFlag* soundFlag = toStaffText(e)->soundFlag()) {
                return soundFlag;
            }
        }

        EngravingItem* prev = nullptr;
        if (e->explicitParent() == this) {
            prev = prevAnnotation(e);
        }
        if (prev) {
            return prev;
        }
        if (notChordRestType()) {
            EngravingItem* lastEl = lastElementOfSegment(activeStaff);
            if (lastEl) {
                return lastEl;
            }
        }
        if (isTimeTickType()) {
            Segment* prevSeg = prev1MMenabled();
            return prevSeg ? prevSeg->lastElementOfSegment(activeStaff) : nullptr;
        }
        track_idx_t track = score()->nstaves() * VOICES - 1;
        Segment* s = this;
        EngravingItem* el = s->element(track);
        while (track > 0 && (!el || el->staffIdx() != activeStaff)) {
            el = s->element(--track);
            if (track == 0) {
                track = score()->nstaves() * VOICES - 1;
                s = s->prev1MMenabled();
            }
        }
        if (el->staffIdx() != activeStaff) {
            return nullptr;
        }
        if (el->type() == ElementType::CHORD || el->type() == ElementType::REST
            || el->type() == ElementType::MMREST || el->type() == ElementType::MEASURE_REPEAT) {
            ChordRest* cr = this->cr(el->track());
            if (cr) {
                EngravingItem* elCr = cr->lastElementBeforeSegment();
                if (elCr) {
                    return elCr;
                }
            }
        }
        if (el->type() == ElementType::CHORD) {
            return toChord(el)->lastElementBeforeSegment();
        } else if (el->type() == ElementType::NOTE) {
            Chord* c = toNote(el)->chord();
            return c->lastElementBeforeSegment();
        } else {
            return el;
        }
    }
    case ElementType::ARPEGGIO:
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD: {
        EngravingItem* el = this->element(e->track());
        assert(el->type() == ElementType::CHORD);
        return toChord(el)->prevElement();
    }
    default: {
        EngravingItem* el = e;
        Segment* seg = this;
        if (e->type() == ElementType::TIE_SEGMENT || e->type() == ElementType::LAISSEZ_VIB_SEGMENT
            || e->type() == ElementType::PARTIAL_TIE_SEGMENT
            || e->type() == ElementType::GLISSANDO_SEGMENT || e->type() == ElementType::NOTELINE_SEGMENT) {
            SpannerSegment* s = toSpannerSegment(e);
            Spanner* sp = s->spanner();
            el = sp->startElement();
            seg = sp->startSegment();
        } else {
            EngravingItem* ep = e->parentItem();
            if (ep->isNote() || ep->isRest() || (ep->isChord() && !e->isNote())) {
                el = e->parentItem();
            }
        }

        EngravingItem* prev = seg->prevElementOfSegment(el, activeStaff);
        if (prev) {
            if (prev->type() == ElementType::CHORD || prev->type() == ElementType::REST
                || prev->type() == ElementType::MMREST || prev->type() == ElementType::MEASURE_REPEAT) {
                ChordRest* cr = seg->cr(prev->track());
                if (cr) {
                    EngravingItem* elCr = cr->lastElementBeforeSegment();
                    if (elCr) {
                        return elCr;
                    }
                }
            }
            if (prev->type() == ElementType::CHORD) {
                return toChord(prev)->lastElementBeforeSegment();
            } else if (prev->type() == ElementType::NOTE) {
                Chord* c = toNote(prev)->chord();
                return c->lastElementBeforeSegment();
            } else {
                return prev;
            }
        }
        Segment* prevSeg = seg->prev1MMenabled();
        for (; prevSeg && prevSeg->isTimeTickType(); prevSeg = prevSeg->prev1MMenabled()) {
            if (Spanner* spanner = prevSeg->lastSpanner(activeStaff)) {
                return spanner->spannerSegments().front();
            }
            if (EngravingItem* annotation = prevSeg->lastAnnotation(activeStaff)) {
                return annotation;
            }
        }
        if (!prevSeg) {
            MeasureBase* mb = measure()->prev();
            return mb && mb->isBox() ? mb : score()->firstElement();
        }

        Measure* psm = prevSeg->measure();
        if (psm != measure()) {
            // check for frame, measure elements
            MeasureBase* pmb = measure()->prevMM();
            EngravingItem* me = measure()->el().empty() ? nullptr : measure()->el().back();
            if (me && me->isTextBase() && me->staffIdx() == e->staffIdx()) {
                return me;
            } else if (me && me->isLayoutBreak() && e->staffIdx() == 0) {
                return me;
            } else if (measure()->isEndOfSystemLock()) {
                System* system = measure()->system();
                SystemLockIndicator* lockInd = system ? system->lockIndicators().front() : nullptr;
                if (lockInd) {
                    return lockInd;
                }
            } else if (psm != pmb) {
                return pmb;
            }
        }

        prev = prevSeg->lastElementOfSegment(activeStaff);
        while (!prev && prevSeg) {
            prevSeg = prevSeg->prev1MMenabled();
            prev = prevSeg->lastElementOfSegment(activeStaff);
        }
        if (!prevSeg) {
            return score()->firstElement();
        }

        if (prevSeg->notChordRestType()) {
            EngravingItem* lastEl = prevSeg->lastElementOfSegment(activeStaff);
            if (lastEl) {
                return lastEl;
            }
        }
        Spanner* s1 = prevSeg->lastSpanner(activeStaff);
        if (s1) {
            return s1->spannerSegments().front();
        } else if (!prevSeg->annotations().empty()) {
            EngravingItem* next = prevSeg->lastAnnotation(activeStaff);
            if (next) {
                return next;
            }
        }
        if (prev->type() == ElementType::CHORD || prev->type() == ElementType::NOTE || prev->type() == ElementType::REST
            || prev->type() == ElementType::MMREST || prev->type() == ElementType::MEASURE_REPEAT) {
            ChordRest* cr = prevSeg->cr(prev->track());
            if (cr) {
                EngravingItem* elCr = cr->lastElementBeforeSegment();
                if (elCr) {
                    return elCr;
                }
            }
        }
        if (prev->type() == ElementType::CHORD) {
            return toChord(prev)->lastElementBeforeSegment();
        } else if (prev->type() == ElementType::NOTE) {
            Chord* c = toNote(prev)->chord();
            return c->lastElementBeforeSegment();
        } else {
            return prev;
        }
    }
    }
}

EngravingItem* Segment::firstElement(staff_idx_t staffIdx) const
{
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;
    for (track_idx_t track =  startTrack; track < endTrack; ++track) {
        if (EngravingItem* item = m_elist[track]) {
            return item;
        }
    }
    return nullptr;
}

//--------------------------------------------------------
//   lastInPrevSegments
//   Searches for the previous segment that has elements on
//   the active staff and returns its last element
//
//   Uses lastElement so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

EngravingItem* Segment::lastInPrevSegments(staff_idx_t activeStaff) const
{
    EngravingItem* re = 0;
    const Segment* seg = this;

    while (!re) {
        seg = seg->prev1MMenabled();
        if (!seg) {   //end of staff, or score
            break;
        }

        re = seg->lastElementOfSegment(activeStaff);
    }

    if (re) {
        return re;
    }

    if (!seg) {   //end of staff
        if (activeStaff == 0) {   //end of score
            return 0;
        }

        re = 0;
        seg = score()->lastSegmentMM();
        while (true) {
            //if (seg->segmentType() == SegmentType::EndBarLine)
            //      score()->inputState().setTrack((activeStaff - 1) * VOICES ); //correction

            if ((re = seg->lastElementForNavigation(activeStaff - 1)) != 0) {
                return re;
            }

            seg = seg->prev1MMenabled();
        }
    }

    return 0;
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String Segment::accessibleExtraInfo() const
{
    String rez;
    if (!annotations().empty()) {
        String temp;
        for (const EngravingItem* a : annotations()) {
            if (!score()->selectionFilter().canSelect(a)) {
                continue;
            }
            switch (a->type()) {
            case ElementType::DYNAMIC:
                //they are added in the chordrest, because they are for only one staff
                break;
            default:
                temp = temp + u' ' + a->accessibleInfo();
            }
        }
        if (!temp.isEmpty()) {
            rez = rez + muse::mtrc("engraving", "Annotations:") + temp;
        }
    }

    String startSpanners;
    String endSpanners;

    auto spanners = score()->spannerMap().findOverlapping(tick().ticks(), tick().ticks());
    for (auto interval : spanners) {
        Spanner* s = interval.value;
        if (!score()->selectionFilter().canSelect(s)) {
            continue;
        }
        if (segmentType() == SegmentType::EndBarLine
            || segmentType() == SegmentType::BarLine
            || segmentType() == SegmentType::StartRepeatBarLine) {
            if (s->isVolta()) {
                continue;
            }
        } else {
            if (s->isVolta() || s->isTie()) {     //ties are added in Note
                continue;
            }
        }

        if (s->tick() == tick()) {
            startSpanners += u" " + muse::mtrc("engraving", "Start of %1").arg(s->accessibleInfo());
        }

        const Segment* seg = 0;
        switch (s->type()) {
        case ElementType::VOLTA:
        case ElementType::SLUR:
            seg = this;
            break;
        default:
            seg = next1MM(SegmentType::ChordRest);
            break;
        }

        if (seg && s->tick2() == seg->tick()) {
            endSpanners += u" " + muse::mtrc("engraving", "End of %1").arg(s->accessibleInfo());
        }
    }

    return rez + startSpanners + endSpanners;
}

//---------------------------------------------------------
//   createShapes
//---------------------------------------------------------

void Segment::createShapes()
{
    setVisible(false);
    for (size_t staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
        createShape(staffIdx);
    }
}

//---------------------------------------------------------
//   createShape
//---------------------------------------------------------

void Segment::createShape(staff_idx_t staffIdx)
{
    Shape& s = m_shapes[staffIdx];
    s.clear();

    if (const System* system = this->system()) {
        const std::vector<SysStaff*>& staves = system->staves();

        if (staffIdx < staves.size() && !staves[staffIdx]->show()) {
            return;
        }
    }

    if (!score()->staff(staffIdx)->show()) {
        return;
    }

    track_idx_t strack = staffIdx * VOICES;
    track_idx_t etrack = strack + VOICES;
    for (EngravingItem* e : m_elist) {
        if (!e) {
            continue;
        }
        track_idx_t effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
        if (effectiveTrack >= strack && effectiveTrack < etrack) {
            setVisible(true);
            if (e->isRest() && toRest(e)->ticks() >= measure()->ticks() && measure()->hasVoices(e->staffIdx())) {
                // A full measure rest in a measure with multiple voices must be ignored
                continue;
            }
            if (e->isMMRest() || (e->isMeasureRepeat() && toMeasureRepeat(e)->numMeasures() > 1)) {
                continue;
            }
            if (e->addToSkyline()) {
                s.add(e->shape().translate((e->isClef() ? e->ldata()->pos() : e->pos()) + e->staffOffset()));
            }
            // Non-standard trills display a cue note that we must add to shape here
            if (e->isChord()) {
                Ornament* orn = toChord(e)->findOrnament();
                Chord* cueNoteChord = orn ? orn->cueNoteChord() : nullptr;
                if (cueNoteChord && cueNoteChord->upNote()->visible()) {
                    s.add(cueNoteChord->shape().translate(cueNoteChord->pos() + cueNoteChord->staffOffset()));
                }
            }
        }
    }

    for (EngravingItem* e : m_annotations) {
        if (!e || e->staffIdx() != staffIdx) {
            continue;
        }

        setVisible(true);

        if (!e->addToSkyline()) {
            continue;
        }

        if (e->isHarmony() || e->isFretDiagram()) {
            // TODO: eliminate once and for all this addHorizontalSpace hack [M.S.]
            RectF bbox = e->ldata()->bbox().translated(e->pos());
            s.addHorizontalSpacing(e, bbox.left(), bbox.right());
            if (e->isFretDiagram()) {
                if (Harmony* harmony = toFretDiagram(e)->harmony()) {
                    RectF harmBbox = harmony->ldata()->bbox().translated(harmony->pos() + e->pos());
                    s.addHorizontalSpacing(harmony, harmBbox.left(), harmBbox.right());
                }
            }
        } else if (!e->isRehearsalMark()
                   && !e->isFretDiagram()
                   && !e->isHarmony()
                   && !e->isTempoText()
                   && !e->isDynamic()
                   && !e->isExpression()
                   && !e->isFiguredBass()
                   && !e->isSymbol()
                   && !e->isFSymbol()
                   && !e->isSystemText()
                   && !e->isTripletFeel()
                   && !e->isInstrumentChange()
                   && !e->isArticulationFamily()
                   && !e->isFermata()
                   && !e->isStaffText()
                   && !e->isHarpPedalDiagram()
                   && !e->isPlayTechAnnotation()
                   && !e->isCapo()
                   && !e->isStringTunings()) {
            // annotations added here are candidates for collision detection
            // lyrics, ...
            s.add(e->shape().translate(e->pos() + e->staffOffset()));
        }
    }

    for (unsigned track = strack; track < etrack; ++track) {
        if (!m_preAppendedItems[track]) {
            continue;
        }
        EngravingItem* item = m_preAppendedItems[track];
        if (item->isGraceNotesGroup()) {
            toGraceNotesGroup(item)->addToShape();
        } else {
            Shape& shape = m_shapes[item->vStaffIdx()];
            shape.add(item->shape().translate(item->pos() + item->staffOffset()));
        }
    }
}

//---------------------------------------------------------
//   minRight
//    calculate minimum distance needed to the right
//---------------------------------------------------------

double Segment::minRight() const
{
    double distance = 0.0;
    for (const Shape& sh : shapes()) {
        distance = std::max(distance, sh.right());
    }
    return distance;
}

double Segment::minLeft() const
{
    double distance = -DBL_MAX;
    for (const Shape& sh : shapes()) {
        double l = sh.left();
        if (l > distance) {
            distance = l;
        }
    }
    return distance != -DBL_MAX ? distance : 0.0;
}

void Segment::setSpacing(double val)
{
    m_spacing = val;
}

double Segment::spacing() const
{
    return m_spacing;
}

bool Segment::hasTimeSigAboveStaves() const
{
    return isType(SegmentType::TimeSigType)
           && style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() == TimeSigPlacement::ABOVE_STAVES;
}

bool Segment::makeSpaceForTimeSigAboveStaves() const
{
    bool makeSpace = style().styleB(Sid::timeSigCenterOnBarline)
                     ? style().styleV(Sid::timeSigVSMarginCentered).value<TimeSigVSMargin>() == TimeSigVSMargin::CREATE_SPACE
                     : style().styleV(Sid::timeSigVSMarginNonCentered).value<TimeSigVSMargin>() == TimeSigVSMargin::CREATE_SPACE;
    return hasTimeSigAboveStaves() && makeSpace;
}

bool Segment::hasTimeSigAcrossStaves() const
{
    return isType(SegmentType::TimeSigType)
           && style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>() == TimeSigPlacement::ACROSS_STAVES;
}

bool Segment::canWriteSpannerStartEnd(track_idx_t track, const Spanner* spanner) const
{
    staff_idx_t staffIdx = track2staff(track);
    if (isChordRestType() && (elementAt(track) || (!spanner->isVoiceSpecific() && hasElements(staffIdx)))) {
        return true;
    }

    if (isTimeTickType()) {
        Segment* crSegAtSameTick
            = score()->tick2segment(tick(), true, SegmentType::ChordRest, style().styleB(Sid::createMultiMeasureRests));
        if (!crSegAtSameTick || !crSegAtSameTick->canWriteSpannerStartEnd(track, spanner)) {
            return true;
        }
    }

    return score()->lastMeasure()->last() == this;
}

double Segment::elementsTopOffsetFromSkyline(staff_idx_t staffIndex) const
{
    System* segmentSystem = measure()->system();
    SysStaff* staffSystem = segmentSystem ? segmentSystem->staff(staffIndex) : nullptr;

    if (!staffSystem) {
        return 0;
    }

    SkylineLine north = staffSystem->skyline().north();
    int topOffset = INT_MAX;
    for (const ShapeElement& element : north.elements()) {
        Segment* seg = prev1enabled();
        if (!seg) {
            continue;
        }
        bool ok = seg->pagePos().x() <= element.left() && element.left() <= pagePos().x();
        if (!ok) {
            continue;
        }

        if (element.top() < topOffset) {
            topOffset = element.top();
        }
    }

    if (topOffset == INT_MAX) {
        topOffset = 0;
    }

    return topOffset;
}

double Segment::elementsBottomOffsetFromSkyline(staff_idx_t staffIndex) const
{
    System* segmentSystem = measure()->system();
    SysStaff* staffSystem = segmentSystem ? segmentSystem->staff(staffIndex) : nullptr;

    if (!staffSystem) {
        return 0;
    }

    SkylineLine south = staffSystem->skyline().south();
    int bottomOffset = INT_MIN;
    for (const ShapeElement& element : south.elements()) {
        Segment* seg = prev1enabled();
        if (!seg) {
            continue;
        }
        bool ok = seg->pagePos().x() <= element.left() && element.left() <= pagePos().x();
        if (!ok) {
            continue;
        }

        if (element.bottom() > bottomOffset) {
            bottomOffset = element.bottom();
        }
    }

    if (bottomOffset == INT_MIN) {
        bottomOffset = staffSystem->bbox().height();
    }

    return bottomOffset;
}

//------------------------------------------------------
// shortestChordRest()
// returns the shortest chordRest of a segment. IMPORTANT:
// this is not the same as the ticks() of the segment. The
// actual duration of the segment may be shorter than its
// shortest chordRest. Invisible chordRests are ignored.
//------------------------------------------------------
Fraction Segment::shortestChordRest() const
{
    Fraction shortest = measure()->ticks(); // Initializing at the highest possible value ( = time signature of the measure)
    Fraction cur = measure()->ticks();
    for (auto elem : elist()) {
        if (!elem || !elem->staff()->show() || !elem->isChordRest() || (elem->isRest() && toRest(elem)->isGap())
            || (!elem->visible()
                && measure()->hasVoices(elem->staffIdx(), measure()->tick(), measure()->ticks(), /*considerInvisible*/ true))) {
            continue;
        }
        cur = toChordRest(elem)->actualTicks();
        assert(cur.isNotZero());
        if (cur < shortest) {
            shortest = cur;
        }
    }
    return shortest;
}

bool Segment::hasAccidentals() const
{
    if (!isChordRestType()) {
        return false;
    }
    for (EngravingItem* e : elist()) {
        if (!e || !e->isChord() || (e->staff() && !e->staff()->show())) {
            continue;
        }
        for (Note* note : toChord(e)->notes()) {
            if (note->accidental() && note->accidental()->addToSkyline()) {
                return true;
            }
        }
    }
    return false;
}

bool Segment::goesBefore(const Segment* nextSegment) const
{
    Measure* meas = measure();
    Measure* prevMeasure = meas->prevMeasure();
    const bool thisMeasureIsStartRepeat = meas->repeatStart();
    const bool prevMeasureIsEndRepeat = prevMeasure ? prevMeasure->repeatEnd() : false;

    bool thisIsClef = isClefType();
    bool nextIsClef = nextSegment->isClefType();
    bool thisIsBarline = isType(SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine);
    bool nextIsBarline = nextSegment->isType(SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine);

    bool thisIsStartRepeat = isStartRepeatBarLineType();
    bool nextIsStartRepeat = nextSegment->isStartRepeatBarLineType();
    bool thisIsKeySig = isKeySigType();
    bool thisIsTimeSig = isTimeSigType();
    bool nextIsKeySig = nextSegment->isKeySigType();
    bool nextIsTimeSig = nextSegment->isTimeSigType();

    // Place non header clefs AFTER header clefs, key signatures, time signatures
    const bool firstSystemMeasure = meas->findSegmentR(SegmentType::HeaderClef, Fraction(0, 1));
    bool thisIsHeader = isHeaderClefType() || (rtick() == Fraction(0, 1) && firstSystemMeasure && !thisIsClef);
    bool nextIsHeader = nextSegment->isHeaderClefType() || (nextSegment->rtick() == Fraction(0, 1) && firstSystemMeasure && !nextIsClef);

    bool thisIsEndOfMeasure = endOfMeasureChange();

    // Place segments in correct place when "Place all changes before the barline" is enabled
    if (thisIsEndOfMeasure && nextSegment->tick() != meas->endTick()) {
        return false;
    }

    if (thisIsEndOfMeasure && nextIsBarline) {
        return true;
    }

    // Place key signatures and time signatures in correct place when "Allow changes between end-start repeats" is enabled
    if ((thisIsKeySig || thisIsTimeSig || thisIsClef) && nextIsStartRepeat && thisMeasureIsStartRepeat && prevMeasureIsEndRepeat) {
        return style().styleB(Sid::changesBetweenEndStartRepeat);
    }
    if ((nextIsKeySig || nextIsTimeSig || nextIsClef) && thisIsStartRepeat && thisMeasureIsStartRepeat && prevMeasureIsEndRepeat) {
        return !style().styleB(Sid::changesBetweenEndStartRepeat);
    }

    if (thisIsClef && nextIsBarline) {
        ClefToBarlinePosition clefPos = ClefToBarlinePosition::AUTO;
        for (EngravingItem* item : elist()) {
            if (item && item->isClef()) {
                clefPos = toClef(item)->clefToBarlinePosition();
                break;
            }
        }
        return clefPos != ClefToBarlinePosition::AFTER;
    }

    if (thisIsBarline && nextIsClef) {
        ClefToBarlinePosition clefPos = ClefToBarlinePosition::AUTO;
        for (EngravingItem* item : nextSegment->elist()) {
            if (item && item->isClef()) {
                clefPos = toClef(item)->clefToBarlinePosition();
                break;
            }
        }
        return clefPos == ClefToBarlinePosition::AFTER;
    }

    if (thisIsClef && (nextIsKeySig || nextIsTimeSig) && rtick() == Fraction(0, 1) && nextSegment->rtick() == Fraction(0, 1)
        && !nextIsHeader && thisMeasureIsStartRepeat && prevMeasureIsEndRepeat) {
        // Between repeats
        return true;
    }

    if ((thisIsKeySig || thisIsTimeSig) && nextIsClef && rtick() == Fraction(0, 1) && nextSegment->rtick() == Fraction(0, 1)
        && !thisIsHeader && thisMeasureIsStartRepeat && prevMeasureIsEndRepeat) {
        // Between repeats
        return false;
    }

    return segmentType() < nextSegment->segmentType();
}
} // namespace mu::engraving
