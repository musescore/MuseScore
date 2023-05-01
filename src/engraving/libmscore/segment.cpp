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

#include "segment.h"

#include <climits>

#include "translation.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "barline.h"
#include "beam.h"
#include "chord.h"
#include "clef.h"
#include "engravingitem.h"
#include "factory.h"
#include "harmony.h"
#include "hook.h"
#include "instrchange.h"
#include "keysig.h"
#include "masterscore.h"
#include "measure.h"
#include "mmrest.h"
#include "mscore.h"
#include "note.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "sig.h"
#include "staff.h"
#include "staffstate.h"
#include "system.h"
#include "timesig.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"

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
    return subTypeName(_segmentType);
}

const char* Segment::subTypeName(SegmentType t)
{
    switch (t) {
    case SegmentType::Invalid:              return "Invalid";
    case SegmentType::BeginBarLine:         return "BeginBarLine";
    case SegmentType::HeaderClef:           return "HeaderClef";
    case SegmentType::Clef:                 return "Clef";
    case SegmentType::KeySig:               return "Key Signature";
    case SegmentType::Ambitus:              return "Ambitus";
    case SegmentType::TimeSig:              return "Time Signature";
    case SegmentType::StartRepeatBarLine:   return "Begin Repeat";
    case SegmentType::BarLine:              return "BarLine";
    case SegmentType::Breath:               return "Breath";
    case SegmentType::ChordRest:            return "ChordRest";
    case SegmentType::EndBarLine:           return "EndBarLine";
    case SegmentType::KeySigAnnounce:       return "Key Sig Precaution";
    case SegmentType::TimeSigAnnounce:      return "Time Sig Precaution";
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
        _elist[track] = el;
        setEmpty(false);
    } else {
        _elist[track] = 0;
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
    _segmentType = st;
    _tick = t;
    init();
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(const Segment& s)
    : EngravingItem(s)
{
    _segmentType        = s._segmentType;
    _tick               = s._tick;
    _extraLeadingSpace  = s._extraLeadingSpace;

    for (EngravingItem* e : s._annotations) {
        add(e->clone());
    }

    _elist.reserve(s._elist.size());
    for (EngravingItem* e : s._elist) {
        EngravingItem* ne = 0;
        if (e) {
            ne = e->clone();
            ne->setParent(this);
        }
        _elist.push_back(ne);
    }
    _shapes  = s._shapes;
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
    assert(_segmentType != SegmentType::Clef || t != SegmentType::ChordRest);
    _segmentType = t;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Segment::setScore(Score* score)
{
    EngravingItem::setScore(score);
    for (EngravingItem* e : _elist) {
        if (e) {
            e->setScore(score);
        }
    }
    for (EngravingItem* e : _annotations) {
        e->setScore(score);
    }
}

Segment::~Segment()
{
    for (EngravingItem* e : _elist) {
        if (!e) {
            continue;
        }

        Staff* staff = e->staff();

        if (e->isTimeSig() && staff) {
            staff->removeTimeSig(toTimeSig(e));
        }

        delete e;
    }

    DeleteAll(_annotations);
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
    _elist.assign(tracks, 0);
    _preAppendedItems.assign(tracks, 0);
    _shapes.assign(staves, Shape());
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Segment::tick() const
{
    return _tick + measure()->tick();
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
            if (track == mu::nidx) {
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

ChordRest* Segment::nextChordRest(track_idx_t track, bool backwards) const
{
    for (const Segment* seg = this; seg; seg = backwards ? seg->prev1() : seg->next1()) {
        EngravingItem* el = seg->element(track);
        if (el && el->isChordRest()) {
            return toChordRest(el);
        }
    }
    return 0;
}

EngravingItem* Segment::element(track_idx_t track) const
{
    if (track >= _elist.size()) {
        return nullptr;
    }

    return _elist[track];
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(staff_idx_t staff)
{
    track_idx_t track = staff * VOICES;
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        _elist.insert(_elist.begin() + track, 0);
        _preAppendedItems.insert(_preAppendedItems.begin() + track, 0);
    }
    _shapes.insert(_shapes.begin() + staff, Shape());

    for (EngravingItem* e : _annotations) {
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
    _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);
    _preAppendedItems.erase(_preAppendedItems.begin() + track, _preAppendedItems.begin() + track + VOICES);
    _shapes.erase(_shapes.begin() + staff);

    for (EngravingItem* e : _annotations) {
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
    // generated elements can be overwritten
    if (_elist[track] && !_elist[track]->generated()) {
        LOGD("add(%s): there is already a %s at track %zu tick %d",
             el->typeName(),
             _elist[track]->typeName(),
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
    assert(track != mu::nidx);
    assert(el->score() == score());
    assert(score()->nstaves() * VOICES == _elist.size());
    // make sure offset is correct for staff
    if (el->isStyled(Pid::OFFSET)) {
        el->setOffset(el->propertyDefault(Pid::OFFSET).value<PointF>());
    }

    switch (el->type()) {
    case ElementType::MEASURE_REPEAT:
        _elist[track] = el;
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
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::FERMATA:
    case ElementType::STICKING:
        _annotations.push_back(el);
        break;

    case ElementType::STAFF_STATE:
        if (toStaffState(el)->staffStateType() == StaffStateType::INSTRUMENT) {
            StaffState* ss = toStaffState(el);
            Part* part = el->part();
            part->setInstrument(ss->instrument(), tick());
        }
        _annotations.push_back(el);
        break;

    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* is = toInstrumentChange(el);
        Part* part = is->part();
        part->setInstrument(is->instrument(), tick());
        _annotations.push_back(el);
        break;
    }

    case ElementType::CLEF:
        assert(_segmentType == SegmentType::Clef || _segmentType == SegmentType::HeaderClef);
        checkElement(el, track);
        _elist[track] = el;
        if (!el->generated()) {
            el->staff()->setClef(toClef(el));
        }
        setEmpty(false);
        break;

    case ElementType::TIMESIG:
        assert(segmentType() == SegmentType::TimeSig || segmentType() == SegmentType::TimeSigAnnounce);
        checkElement(el, track);
        _elist[track] = el;
        el->staff()->addTimeSig(toTimeSig(el));
        setEmpty(false);
        break;

    case ElementType::KEYSIG:
        assert(_segmentType == SegmentType::KeySig || _segmentType == SegmentType::KeySigAnnounce);
        checkElement(el, track);
        _elist[track] = el;
        if (!el->generated()) {
            el->staff()->setKey(tick(), toKeySig(el)->keySigEvent());
        }
        setEmpty(false);
        break;

    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::MMREST:
        assert(_segmentType == SegmentType::ChordRest);
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
            // the tick position of a tuplet is the tick position of its
            // first element:
//                  ChordRest* cr = toChordRest(el);
//                  if (cr->tuplet() && !cr->tuplet()->elements().empty() && cr->tuplet()->elements().front() == cr && cr->tuplet()->tick() < 0)
//                        cr->tuplet()->setTick(cr->tick());
            score()->setPlaylistDirty();
        }
    // fall through

    case ElementType::BAR_LINE:
    case ElementType::BREATH:
        if (track < score()->nstaves() * VOICES) {
            checkElement(el, track);
            _elist[track] = el;
        }
        setEmpty(false);
        break;

    case ElementType::AMBITUS:
        assert(_segmentType == SegmentType::Ambitus);
        checkElement(el, track);
        _elist[track] = el;
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
        _elist[track] = 0;
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
        _elist[track] = 0;
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
    case ElementType::SYMBOL:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::TEMPO_TEXT:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::FERMATA:
    case ElementType::STICKING:
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

    case ElementType::TIMESIG:
        _elist[track] = 0;
        el->staff()->removeTimeSig(toTimeSig(el));
        break;

    case ElementType::KEYSIG:
        _elist[track] = 0;
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
        _elist[track] = 0;
        break;

    case ElementType::BREATH:
        _elist[track] = 0;
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
        LOGD("Segment:segmentType():  bad type: <%s>", TConv::toXml(type));
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
            dl.push_back(_elist[k]);
        }
    }
    std::swap(_elist, dl);
    std::map<staff_idx_t, staff_idx_t> map;
    for (staff_idx_t k = 0; k < dst.size(); ++k) {
        map.insert({ dst[k], k });
    }
    for (EngravingItem* e : _annotations) {
        ElementType et = e->type();
        // the set of system objects that are allowed to move staves if they are clones / excerpts
        static const std::set<ElementType> allowedTypes {
            ElementType::REHEARSAL_MARK,
            ElementType::SYSTEM_TEXT,
            ElementType::TRIPLET_FEEL,
            ElementType::PLAYTECH_ANNOTATION,
            ElementType::JUMP,
            ElementType::MARKER,
            ElementType::TEMPO_TEXT,
            ElementType::VOLTA,
            ElementType::GRADUAL_TEMPO_CHANGE,
            ElementType::TEXTLINE
        };
        if (!e->systemFlag() || (e->isLinked() && (allowedTypes.find(et) != allowedTypes.end()))) {
            e->setTrack(map[e->staffIdx()] * VOICES + e->voice());
        }
    }
    fixStaffIdx();
}

//---------------------------------------------------------
//   fixStaffIdx
//---------------------------------------------------------

void Segment::fixStaffIdx()
{
    int track = 0;
    for (EngravingItem* e : _elist) {
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
    if (!_annotations.empty()) {
        setEmpty(false);
        return;
    }
    setEmpty(true);
    for (const EngravingItem* e : _elist) {
        if (e) {
            setEmpty(false);
            break;
        }
    }
}

//---------------------------------------------------------
//   swapElements
//---------------------------------------------------------

void Segment::swapElements(track_idx_t i1, track_idx_t i2)
{
    std::iter_swap(_elist.begin() + i1, _elist.begin() + i2);
    if (_elist[i1]) {
        _elist[i1]->setTrack(i1);
    }
    if (_elist[i2]) {
        _elist[i2]->setTrack(i2);
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
        return _tick;
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
        for (EngravingItem* e : _elist) {
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

double Segment::widthInStaff(staff_idx_t staffIdx, SegmentType t) const
{
    const double segX = x();
    double nextSegX = segX;

    Segment* nextSeg = nextInStaff(staffIdx, t);
    if (nextSeg) {
        nextSegX = nextSeg->x();
    } else {
        Segment* lastSeg = measure()->lastEnabled();
        if (lastSeg->segmentType() & t) {
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
    for (EngravingItem* e : _elist) {
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
    for (const EngravingItem* e : _elist) {
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

//---------------------------------------------------------
//   allElementsInvisible
///  return true if all elements in the segment are invisible
//---------------------------------------------------------

bool Segment::allElementsInvisible() const
{
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
            EngravingItem* e = _elist[track];
            if (e && e->visible() && !RealIsEqual(e->width(), 0.0)) {
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
    for (const EngravingItem* e : _annotations) {
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
    for (EngravingItem* e : _annotations) {
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
    for (EngravingItem* e : _annotations) {
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
    for (auto i = _annotations.begin(); i != _annotations.end(); ++i) {
        if (*i == e) {
            _annotations.erase(i);
            break;
        }
    }
}

//---------------------------------------------------------
//   clearAnnotations
//---------------------------------------------------------

void Segment::clearAnnotations()
{
    _annotations.clear();
}

//---------------------------------------------------------
//   elementAt
//    A variant of the element(int) function,
//    specifically intended to be called from QML plugins
//---------------------------------------------------------

EngravingItem* Segment::elementAt(track_idx_t track) const
{
    EngravingItem* e = track < _elist.size() ? _elist[track] : 0;
    return e;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Segment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (size_t track = 0; track < score()->nstaves() * VOICES; ++track) {
        size_t staffIdx = track / VOICES;
        bool thisMeasureVisible = measure()->visible(staffIdx) && score()->staff(staffIdx)->show();
        if (!all && !thisMeasureVisible) {
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
                result = result.united(note->bbox());
            }

            Hook* hook = chord->hook();
            if (hook) {
                RectF rect = RectF(hook->pos().x(), hook->pos().y(), hook->width(), hook->height());
                result = result.united(rect);
            }

            continue;
        }

        result = result.united(element->bbox());
    }

    return result;
}

//---------------------------------------------------------
//   firstElement
//   This function returns the first main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

EngravingItem* Segment::firstElement(staff_idx_t staff)
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

EngravingItem* Segment::lastElement(staff_idx_t staff)
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

EngravingItem* Segment::getElement(staff_idx_t staff)
{
    segmentType();
    if (segmentType() == SegmentType::ChordRest) {
        return firstElement(staff);
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

EngravingItem* Segment::nextAnnotation(EngravingItem* e)
{
    if (_annotations.empty() || e == _annotations.back()) {
        return nullptr;
    }
    auto ei = std::find(_annotations.begin(), _annotations.end(), e);
    if (ei == _annotations.end()) {
        return nullptr;                   // element not found
    }
    // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
    auto resIt = std::find_if(ei + 1, _annotations.end(), [e](EngravingItem* nextElem) {
        return nextElem && nextElem->staffIdx() == e->staffIdx();
    });

    return _annotations.end() == resIt ? nullptr : *resIt;
}

//---------------------------------------------------------
//   prevAnnotation
//   return previous element in _annotations
//---------------------------------------------------------

EngravingItem* Segment::prevAnnotation(EngravingItem* e)
{
    if (e == _annotations.front()) {
        return nullptr;
    }
    auto reverseIt = std::find(_annotations.rbegin(), _annotations.rend(), e);
    if (reverseIt == _annotations.rend()) {
        return nullptr;                   // element not found
    }
    // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
    auto resIt = std::find_if(reverseIt + 1, _annotations.rend(), [e](EngravingItem* prevElem) {
        return prevElem && prevElem->staffIdx() == e->staffIdx();
    });

    return _annotations.rend() == resIt ? nullptr : *resIt;
}

//---------------------------------------------------------
//   firstAnnotation
//---------------------------------------------------------

EngravingItem* Segment::firstAnnotation(Segment* s, staff_idx_t activeStaff)
{
    for (auto i = s->annotations().begin(); i != s->annotations().end(); ++i) {
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

EngravingItem* Segment::lastAnnotation(Segment* s, staff_idx_t activeStaff)
{
    for (auto i = --s->annotations().end(); i != s->annotations().begin(); --i) {
        // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
        if ((*i)->staffIdx() == activeStaff) {
            return *i;
        }
    }
    auto i = s->annotations().begin();
    if ((*i)->staffIdx() == activeStaff) {
        return *i;
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

EngravingItem* Segment::firstInNextSegments(staff_idx_t activeStaff)
{
    EngravingItem* re = 0;
    Segment* seg = this;
    while (!re) {
        seg = seg->next1MMenabled();
        if (!seg) {   //end of staff, or score
            break;
        }

        re = seg->firstElement(activeStaff);
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
//   returns the first non null element in the given segment
//---------------------------------------------------------

EngravingItem* Segment::firstElementOfSegment(Segment* s, staff_idx_t activeStaff)
{
    for (auto i: s->elist()) {
        if (i && i->staffIdx() == activeStaff) {
            if (i->type() == ElementType::CHORD) {
                return toChord(i)->notes().back();
            } else {
                return i;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   nextElementOfSegment
//   returns the next element in the given segment
//---------------------------------------------------------

EngravingItem* Segment::nextElementOfSegment(Segment* s, EngravingItem* e, staff_idx_t activeStaff)
{
    for (size_t track = 0; track < score()->nstaves() * VOICES - 1; ++track) {
        if (s->element(track) == 0) {
            continue;
        }
        EngravingItem* el = s->element(track);
        if (el == e) {
            EngravingItem* next = s->element(track + 1);
            while (track < score()->nstaves() * VOICES - 1
                   && (!next || next->staffIdx() != activeStaff)) {
                next = s->element(++track);
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
                EngravingItem* nextEl = s->element(++track);
                while (track < score()->nstaves() * VOICES - 1
                       && (!nextEl || nextEl->staffIdx() != activeStaff)) {
                    nextEl = s->element(++track);
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
//   returns the previous element in the given segment
//---------------------------------------------------------

EngravingItem* Segment::prevElementOfSegment(Segment* s, EngravingItem* e, staff_idx_t activeStaff)
{
    for (int track = static_cast<int>(score()->nstaves() * VOICES) - 1; track > 0; --track) {
        if (s->element(track) == 0) {
            continue;
        }
        EngravingItem* el = s->element(track);
        if (el == e) {
            EngravingItem* prev = s->element(track - 1);
            while (track > 0
                   && (!prev || prev->staffIdx() != activeStaff)) {
                prev = s->element(--track);
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
            std::vector<Note*> notes = toChord(el)->notes();
            auto i = std::find(notes.begin(), notes.end(), e);
            if (i == notes.end()) {
                continue;
            }
            if (i != --notes.end()) {
                return *(i + 1);
            } else {
                EngravingItem* prevEl = s->element(--track);
                while (track > 0
                       && (!prevEl || prevEl->staffIdx() != activeStaff)) {
                    prevEl = s->element(--track);
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
//   returns the last element in the given segment
//---------------------------------------------------------

EngravingItem* Segment::lastElementOfSegment(Segment* s, staff_idx_t activeStaff)
{
    std::vector<EngravingItem*> elements = s->elist();
    for (auto i = --elements.end(); i != elements.begin(); --i) {
        if (*i && (*i)->staffIdx() == activeStaff) {
            if ((*i)->isChord()) {
                return toChord(*i)->notes().front();
            } else {
                return *i;
            }
        }
    }
    auto i = elements.begin();
    if (*i && (*i)->staffIdx() == activeStaff) {
        if ((*i)->type() == ElementType::CHORD) {
            return toChord(*i)->notes().front();
        } else {
            return *i;
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   firstSpanner
//---------------------------------------------------------

Spanner* Segment::firstSpanner(staff_idx_t activeStaff)
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {  // range not empty
        for (auto i = range.first; i != range.second; ++i) {
            Spanner* s = i->second;
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

Spanner* Segment::lastSpanner(staff_idx_t activeStaff)
{
    std::multimap<int, Spanner*> mmap = score()->spanner();
    auto range = mmap.equal_range(tick().ticks());
    if (range.first != range.second) {  // range not empty
        for (auto i = --range.second;; --i) {
            Spanner* s = i->second;
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

bool Segment::notChordRestType(Segment* s)
{
    if (s->segmentType() == SegmentType::KeySig
        || s->segmentType() == SegmentType::TimeSig
        || s->segmentType() == SegmentType::Clef
        || s->segmentType() == SegmentType::HeaderClef
        || s->segmentType() == SegmentType::BeginBarLine
        || s->segmentType() == SegmentType::EndBarLine
        || s->segmentType() == SegmentType::BarLine
        || s->segmentType() == SegmentType::KeySigAnnounce
        || s->segmentType() == SegmentType::TimeSigAnnounce) {
        return true;
    } else {
        return false;
    }
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
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::STICKING: {
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
            EngravingItem* nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
            if (nextEl) {
                return nextEl;
            }
            nextSegment = nextSegment->next1MMenabled();
        }
        break;
    }
    case ElementType::SEGMENT: {
        if (!_annotations.empty()) {
            EngravingItem* next = firstAnnotation(this, activeStaff);
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
            EngravingItem* nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
            if (nextEl) {
                return nextEl;
            }
            nextSegment = nextSegment->next1MMenabled();
        }
        break;
    }
    default: {
        EngravingItem* p;
        if (e->isTieSegment() || e->isGlissandoSegment()) {
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
        EngravingItem* nextEl = nextElementOfSegment(seg, el, activeStaff);
        if (nextEl) {
            return nextEl;
        }
        if (!_annotations.empty()) {
            EngravingItem* next = firstAnnotation(seg, activeStaff);
            if (next) {
                return next;
            }
        }
        Spanner* s = firstSpanner(activeStaff);
        if (s) {
            return s->spannerSegments().front();
        }
        Segment* nextSegment =  seg->next1MMenabled();
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
            }
        }

        while (nextSegment) {
            nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
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
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::REHEARSAL_MARK:
    case ElementType::MARKER:
    case ElementType::IMAGE:
    case ElementType::TEXT:
    case ElementType::TREMOLOBAR:
    case ElementType::TAB_DURATION_SYMBOL:
    case ElementType::FIGURED_BASS:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::STICKING: {
        EngravingItem* prev = nullptr;
        if (e->explicitParent() == this) {
            prev = prevAnnotation(e);
        }
        if (prev) {
            return prev;
        }
        if (notChordRestType(this)) {
            EngravingItem* lastEl = lastElementOfSegment(this, activeStaff);
            if (lastEl) {
                return lastEl;
            }
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
    case ElementType::TREMOLO: {
        EngravingItem* el = this->element(e->track());
        assert(el->type() == ElementType::CHORD);
        return toChord(el)->prevElement();
    }
    default: {
        EngravingItem* el = e;
        Segment* seg = this;
        if (e->type() == ElementType::TIE_SEGMENT
            || e->type() == ElementType::GLISSANDO_SEGMENT) {
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

        EngravingItem* prev = seg->prevElementOfSegment(seg, el, activeStaff);
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
            } else if (psm != pmb) {
                return pmb;
            }
        }

        prev = lastElementOfSegment(prevSeg, activeStaff);
        while (!prev && prevSeg) {
            prevSeg = prevSeg->prev1MMenabled();
            prev = lastElementOfSegment(prevSeg, activeStaff);
        }
        if (!prevSeg) {
            return score()->firstElement();
        }

        if (notChordRestType(prevSeg)) {
            EngravingItem* lastEl = lastElementOfSegment(prevSeg, activeStaff);
            if (lastEl) {
                return lastEl;
            }
        }
        Spanner* s1 = prevSeg->lastSpanner(activeStaff);
        if (s1) {
            return s1->spannerSegments().front();
        } else if (!prevSeg->annotations().empty()) {
            EngravingItem* next = lastAnnotation(prevSeg, activeStaff);
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

//--------------------------------------------------------
//   lastInPrevSegments
//   Searches for the previous segment that has elements on
//   the active staff and returns its last element
//
//   Uses lastElement so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

EngravingItem* Segment::lastInPrevSegments(staff_idx_t activeStaff)
{
    EngravingItem* re = 0;
    Segment* seg = this;

    while (!re) {
        seg = seg->prev1MMenabled();
        if (!seg) {   //end of staff, or score
            break;
        }

        re = seg->lastElementOfSegment(seg, activeStaff);
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

            if ((re = seg->lastElement(activeStaff - 1)) != 0) {
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
            rez = rez + mtrc("engraving", "Annotations:") + temp;
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
            startSpanners += u" " + mtrc("engraving", "Start of %1").arg(s->accessibleInfo());
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
            endSpanners += u" " + mtrc("engraving", "End of %1").arg(s->accessibleInfo());
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
    addPreAppendedToShape();
}

//---------------------------------------------------------
//   createShape
//---------------------------------------------------------

void Segment::createShape(staff_idx_t staffIdx)
{
    Shape& s = _shapes[staffIdx];
    s.setSqueezeFactor(1);
    s.clear();

    if (const System* system = this->system()) {
        const std::vector<SysStaff*>& staves = system->staves();

        if (staffIdx < staves.size() && !staves[staffIdx]->show()) {
            return;
        }
    }

    if (segmentType() & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
        setVisible(true);
        BarLine* bl = toBarLine(element(staffIdx * VOICES));
        if (bl) {
            RectF r = bl->layoutRect();
            s.add(r.translated(bl->pos()), bl);
        }
        s.addHorizontalSpacing(bl, 0, 0);
        return;
    }

    if (!score()->staff(staffIdx)->show()) {
        return;
    }

    track_idx_t strack = staffIdx * VOICES;
    track_idx_t etrack = strack + VOICES;
    for (EngravingItem* e : _elist) {
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
            if (e->isMMRest()) {
                continue;
            }
            if (e->addToSkyline()) {
                s.add(e->shape().translate(e->isClef() ? e->ipos() : e->pos()));
            }
        }
    }

    for (EngravingItem* e : _annotations) {
        if (!e || e->staffIdx() != staffIdx) {
            continue;
        }
        setVisible(true);
        if (!e->addToSkyline()) {
            continue;
        }

        if (e->isHarmony()) {
            // use same spacing calculation as for chordrest
            toHarmony(e)->layout();
            double x1 = e->bbox().x() + e->pos().x();
            double x2 = e->bbox().x() + e->bbox().width() + e->pos().x();
            s.addHorizontalSpacing(e, x1, x2);
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
                   && !e->isArticulation()
                   && !e->isFermata()
                   && !e->isStaffText()
                   && !e->isPlayTechAnnotation()) {
            // annotations added here are candidates for collision detection
            // lyrics, ...
            s.add(e->shape().translate(e->pos()));
        }
    }
}

void Segment::addPreAppendedToShape()
{
    track_idx_t tracks = score()->ntracks();
    for (unsigned track = 0; track < tracks; ++track) {
        if (!_preAppendedItems[track]) {
            continue;
        }
        EngravingItem* item = _preAppendedItems[track];
        if (item->isGraceNotesGroup()) {
            toGraceNotesGroup(item)->addToShape();
        } else {
            Shape& shape = _shapes[item->vStaffIdx()];
            shape.add(item->shape().translated(item->pos()));
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
    if (isClefType()) {
        distance += score()->styleMM(Sid::clefBarlineDistance);
    }
    if (trailer()) {
        distance += score()->styleMM(Sid::systemTrailerRightMargin);
    }
    return distance;
}

//---------------------------------------------------------
//   minLeft
//    Calculate minimum distance needed to the left shape
//    sl. Sl is the same for all staves.
//---------------------------------------------------------

double Segment::minLeft(const Shape& sl) const
{
    double distance = 0.0;
    for (const Shape& sh : shapes()) {
        double d = sl.minHorizontalDistance(sh);
        if (d > distance) {
            distance = d;
        }
    }
    return distance;
}

double Segment::minLeft() const
{
    double distance = 0.0;
    for (const Shape& sh : shapes()) {
        double l = sh.left();
        if (l > distance) {
            distance = l;
        }
    }
    return distance;
}

std::pair<double, double> Segment::computeCellWidth(const std::vector<int>& visibleParts) const
{
    if (!this->enabled()) {
        return { 0, 0 };
    }

    auto calculateWidth = [measure = measure(), sc = score()->masterScore()](ChordRest* cr) {
        auto quantum = measure->quantumOfSegmentCell();
        return sc->widthOfSegmentCell()
               * sc->spatium()
               * cr->globalTicks().numerator() / cr->globalTicks().denominator()
               * quantum.denominator() / quantum.numerator();
    };

    if (isChordRestType()) {
        float width{ 0 };
        float spacing{ 0 };

        ChordRest* cr{ nullptr };

        cr = ChordRestWithMinDuration(this, visibleParts);

        if (cr) {
            width = calculateWidth(cr);

            if (cr->type() == ElementType::REST) {
                //spacing = 0; //!not necessary. It is to more clearly understanding code
            } else if (cr->type() == ElementType::CHORD) {
                Chord* ch = toChord(cr);

                //! check that gracenote exist. If exist add additional spacing
                //! to avoid colliding between grace note and previous chord
                if (!ch->graceNotes().empty()) {
                    Segment* prevSeg = prev();
                    if (prevSeg && prevSeg->segmentType() == SegmentType::ChordRest) {
                        ChordRest* prevCR = ChordRestWithMinDuration(prevSeg, visibleParts);

                        if (prevCR && prevCR->globalTicks() < measure()->quantumOfSegmentCell()) {
                            spacing = calculateWidth(prevCR);
                            return { spacing, width };
                        }
                    }
                }

                //! check that accidental exist in the chord. If exist add additional spacing
                //! to avoid colliding between grace note and previous chord
                for (auto note : ch->notes()) {
                    if (note->accidental()) {
                        Segment* prevSeg = prev();
                        if (prevSeg && prevSeg->segmentType() == SegmentType::ChordRest) {
                            ChordRest* prevCR = ChordRestWithMinDuration(prevSeg, visibleParts);

                            if (prevCR && prevCR->globalTicks() < measure()->quantumOfSegmentCell()) {
                                spacing = calculateWidth(prevCR);
                                return { spacing, width };
                            }
                        }
                    }
                }
            }
        }

        return { spacing, width };
    }

    Segment* nextSeg = nextActive();
    if (!nextSeg) {
        nextSeg = next(SegmentType::BarLineType);
    }

    if (nextSeg) {
        return { 0, minHorizontalDistance(nextSeg, false) };
    }

    return { 0, minRight() };
}

ChordRest* Segment::ChordRestWithMinDuration(const Segment* seg, const std::vector<int>& visibleParts)
{
    ChordRest* chordRestWithMinDuration{ nullptr };
    int minTicks = std::numeric_limits<int>::max();
    for (int partIdx : visibleParts) {
        for (const Staff* staff : seg->score()->parts().at(partIdx)->staves()) {
            staff_idx_t staffIdx = staff->idx();
            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                if (auto element = seg->elist().at(staffIdx * VOICES + voice)) {
                    if (!element->isChordRest()) {
                        continue;
                    }

                    ChordRest* cr = toChordRest(element);
                    int chordTicks = cr->ticks().ticks();
                    if (chordTicks > minTicks) {
                        continue;
                    }
                    minTicks = chordTicks;
                    chordRestWithMinDuration = cr;
                }
            }
        }
    }

    return chordRestWithMinDuration;
}

void Segment::setSpacing(double val)
{
    m_spacing = val;
}

double Segment::spacing() const
{
    return m_spacing;
}

//---------------------------------------------------------
//   minHorizontalCollidingDistance
//    calculate the minimum distance to ns avoiding collisions
//---------------------------------------------------------

double Segment::minHorizontalCollidingDistance(Segment* ns) const
{
    double w = -100000.0; // This can remain negative in some cases (for instance, mid-system clefs)
    for (unsigned staffIdx = 0; staffIdx < _shapes.size(); ++staffIdx) {
        double d = staffShape(staffIdx).minHorizontalDistance(ns->staffShape(staffIdx));
        w       = std::max(w, d);
    }
    return w;
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
    for (SkylineSegment segment: north) {
        Segment* seg = prev1enabled();
        if (!seg) {
            continue;
        }
        bool ok = seg->pagePos().x() <= segment.x && segment.x <= pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y < topOffset) {
            topOffset = segment.y;
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
    for (SkylineSegment segment: south) {
        Segment* seg = prev1enabled();
        if (!seg) {
            continue;
        }
        bool ok = seg->pagePos().x() <= segment.x && segment.x <= pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y > bottomOffset) {
            bottomOffset = segment.y;
        }
    }

    if (bottomOffset == INT_MIN) {
        bottomOffset = staffSystem->bbox().height();
    }

    return bottomOffset;
}

//---------------------------------------------------------
//   minHorizontalDistance
//    calculate the minimum layout distance to Segment ns
//---------------------------------------------------------

double Segment::minHorizontalDistance(Segment* ns, bool systemHeaderGap) const
{
    double ww = -1000000.0;          // can remain negative
    double d = 0.0;
    for (unsigned staffIdx = 0; staffIdx < _shapes.size(); ++staffIdx) {
        d = ns ? staffShape(staffIdx).minHorizontalDistance(ns->staffShape(staffIdx)) : 0.0;
        // first chordrest of a staff should clear the widest header for any staff
        // so make sure segment is as wide as it needs to be
        if (systemHeaderGap) {
            d = std::max(d, staffShape(staffIdx).right());
        }
        ww      = std::max(ww, d);
    }
    double w = std::max(ww, 0.0);        // non-negative

    // Header exceptions that need additional space (more than the padding)
    double absoluteMinHeaderDist = 1.5 * spatium();
    if (systemHeaderGap) {
        if (isTimeSigType()) {
            w = std::max(w, minRight() + score()->styleMM(Sid::systemHeaderTimeSigDistance));
        } else {
            w = std::max(w, minRight() + score()->styleMM(Sid::systemHeaderDistance));
        }
        if (ns && ns->isStartRepeatBarLineType()) {
            // Align the thin barline of the start repeat to the header
            w -= score()->styleMM(Sid::endBarWidth) + score()->styleMM(Sid::endBarDistance);
        }
        double diff = w - minRight() - ns->minLeft();
        if (diff < absoluteMinHeaderDist) {
            w += absoluteMinHeaderDist - diff;
        }
    }

    // Multimeasure rest exceptions that need special handling
    if (measure() && measure()->isMMRest()) {
        if (ns->isChordRestType()) {
            double minDist = minRight();
            if (isClefType()) {
                minDist += score()->paddingTable().at(ElementType::CLEF).at(ElementType::REST);
            } else if (isKeySigType()) {
                minDist += score()->paddingTable().at(ElementType::KEYSIG).at(ElementType::REST);
            } else if (isTimeSigType()) {
                minDist += score()->paddingTable().at(ElementType::TIMESIG).at(ElementType::REST);
            }
            w = std::max(w, minDist);
        } else if (isChordRestType()) {
            double minWidth = score()->styleMM(Sid::minMMRestWidth).val();
            if (!score()->styleB(Sid::oldStyleMultiMeasureRests)) {
                minWidth += score()->styleMM(Sid::multiMeasureRestMargin).val();
            }
            w = std::max(w, minWidth);
        }
    }

    // Allocate space to ensure minimum length of "dangling" ties at start of system
    if (systemHeaderGap && ns && ns->isChordRestType()) {
        for (EngravingItem* e : ns->elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            double headerTieMargin = score()->styleMM(Sid::HeaderToLineStartDistance);
            for (Note* note : toChord(e)->notes()) {
                if (!note->tieBack() || note->lineAttachPoints().empty()) {
                    continue;
                }
                double tieStartPointX = minRight() + headerTieMargin;
                double notePosX = w + note->pos().x() + toChord(e)->pos().x() + note->headWidth() / 2;
                double tieEndPointX = notePosX + note->lineAttachPoints().at(0).pos().x();
                double tieLength = tieEndPointX - tieStartPointX;
                if (tieLength < score()->styleMM(Sid::MinTieLength)) {
                    w += score()->styleMM(Sid::MinTieLength) - tieLength;
                }
            }
        }
    }

    return w;
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

/************************************************************************
 * computeCrossBeamType
 * Looks at the chords of this segment and next segment to detect beams
 * with alternating stem directions (upDown: this chord is up, next chord
 * is down; downUp: this chord is down, next chord is up). Needed for
 * correct horizontal spacing of cross-beam situations.
 * **********************************************************************/

void Segment::computeCrossBeamType(Segment* nextSeg)
{
    _crossBeamType.reset();
    if (!isChordRestType() || !nextSeg || !nextSeg->isChordRestType()) {
        return;
    }
    bool upDown = false;
    bool downUp = false;
    bool canBeAdjusted = true;
    // Spacing can be adjusted for cross-beam cases only if there aren't
    // chords in other voices in this or next segment.
    for (EngravingItem* e : elist()) {
        if (!e || !e->isChordRest() || !e->staff()->visible()) {
            continue;
        }
        ChordRest* thisCR = toChordRest(e);
        if (!thisCR->visible() || thisCR->isFullMeasureRest()) {
            continue;
        }
        if (!thisCR->beam()) {
            canBeAdjusted = false;
            continue;
        }
        Beam* beam = thisCR->beam();
        for (EngravingItem* ee : nextSeg->elist()) {
            if (!ee || !ee->isChordRest() || !ee->staff()->visible()) {
                continue;
            }
            ChordRest* nextCR = toChordRest(ee);
            if (!nextCR->visible() || nextCR->isFullMeasureRest()) {
                continue;
            }
            if (!nextCR->beam()) {
                canBeAdjusted = false;
                continue;
            }
            if (nextCR->beam() != beam) {
                continue;
            }
            if (thisCR->up() == nextCR->up()) {
                return;
            }
            if (thisCR->up() && !nextCR->up()) {
                upDown = true;
            }
            if (!thisCR->up() && nextCR->up()) {
                downUp = true;
            }
            if (upDown && downUp) {
                return;
            }
        }
    }
    _crossBeamType.upDown = upDown;
    _crossBeamType.downUp = downUp;
    _crossBeamType.canBeAdjusted = canBeAdjusted;
}

/***********************************************
 * stretchSegmentsToWidth
 * Stretch a group of (chordRestType) segments
 * by the specified amount. Uses the spring-rod
 * method.
 * *********************************************/

void Segment::stretchSegmentsToWidth(std::vector<Spring>& springs, double width)
{
    if (springs.empty() || RealIsEqualOrLess(width, 0.0)) {
        return;
    }

    std::sort(springs.begin(), springs.end(), [](const Spring& a, const Spring& b) { return a.preTension < b.preTension; });
    double inverseSpringConst = 0.0;
    double force = 0.0;

    //! NOTE springs.cbegin() != springs.cend() because of the emptiness check at the top
    auto spring = springs.cbegin();
    do {
        inverseSpringConst += 1 / spring->springConst;
        width += spring->width;
        force = width / inverseSpringConst;
        ++spring;
    } while (spring != springs.cend() && !(force < spring->preTension));

    for (const Spring& spring : springs) {
        if (force > spring.preTension) {
            double newWidth = force / spring.springConst;
            spring.segment->setWidth(newWidth + spring.segment->widthOffset());
        }
    }
}

double Segment::computeDurationStretch(Segment* prevSeg, Fraction minTicks, Fraction maxTicks)
{
    auto doComputeDurationStretch = [&] (Fraction curTicks) -> double
    {
        double slope = score()->styleD(Sid::measureSpacing);

        static constexpr double longNoteThreshold = Fraction(1, 16).toDouble();

        if (measure()->isMMRest() && isChordRestType()) { // This is an MM rest segment
            static constexpr int minMMRestCount  = 2; // MMRests with less bars than this don't receive additional spacing
            int count =std::max(measure()->mmRestCount() - minMMRestCount, 0);
            Fraction timeSig = measure()->timesig();
            curTicks = timeSig + Fraction(count, timeSig.denominator());
        }

        // Prevent long notes from being too wide
        static constexpr double maxRatio = 32.0;
        double dMinTicks = minTicks.toDouble();
        double dMaxTicks = maxTicks.toDouble();
        double maxSysRatio = dMaxTicks / dMinTicks;
        if (RealIsEqualOrMore(dMaxTicks / dMinTicks, 2.0) && dMinTicks < longNoteThreshold) {
            /* HACK: we trick the system to ignore the shortest note and use the "next"
             * shortest. For example, if the shortest is a 32nd, we make it a 16th. */
            dMinTicks *= 2.0;
        }
        double ratio = curTicks.toDouble() / dMinTicks;
        if (maxSysRatio > maxRatio) {
            double A = (dMinTicks * (maxRatio - 1)) / (dMaxTicks - dMinTicks);
            double B = (dMaxTicks - (maxRatio * dMinTicks)) / (dMaxTicks - dMinTicks);
            ratio = A * ratio + B;
        }

        double str = pow(slope, log2(ratio));

        // Prevents long notes from being too narrow.
        if (dMinTicks > longNoteThreshold) {
            double empFactor = 0.6;
            str = str * (1 - empFactor + empFactor * sqrt(dMinTicks / longNoteThreshold));
        }

        return str;
    };

    bool hasAdjacent = isChordRestType() && shortestChordRest() == ticks();
    bool prevHasAdjacent = prevSeg && (prevSeg->isChordRestType() && prevSeg->shortestChordRest() == prevSeg->ticks());
    // The actual duration of a segment, i.e. ticks(), can be shorter than its shortest note if
    // another voice comes in. In such case, hasAdjacent = false. This info is key to correct spacing.
    double durStretch;
    if (hasAdjacent || measure()->isMMRest()) { // Normal segments
        durStretch = doComputeDurationStretch(ticks());
    } else { // The following calculations are key to correct spacing of polyrythms
        Fraction curShortest = shortestChordRest();
        Fraction prevShortest = prevSeg ? prevSeg->shortestChordRest() : Fraction(0, 1);
        if (prevSeg && !prevHasAdjacent && prevShortest < curShortest) {
            durStretch = doComputeDurationStretch(prevShortest) * (ticks() / prevShortest).toDouble();
        } else {
            durStretch = doComputeDurationStretch(curShortest) * (ticks() / curShortest).toDouble();
        }
    }

    return durStretch;
}

bool Segment::goesBefore(const Segment* nextSegment) const
{
    bool thisIsClef = isClefType();
    bool nextIsClef = nextSegment->isClefType();
    bool thisIsBarline = isType(SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine);
    bool nextIsBarline = nextSegment->isType(SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine);

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

    return segmentType() < nextSegment->segmentType();
}
} // namespace mu::engraving
