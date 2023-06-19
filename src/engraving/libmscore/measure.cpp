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

/**
 \file
 Implementation of most part of class Measure.
*/

#include "measure.h"

#include <cmath>

#include "accidental.h"
#include "actionicon.h"
#include "barline.h"
#include "beam.h"
#include "bracket.h"
#include "bracketItem.h"
#include "chord.h"
#include "clef.h"
#include "durationelement.h"
#include "factory.h"
#include "hook.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "measurenumber.h"
#include "measurerepeat.h"
#include "mmrestrange.h"
#include "mscoreview.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "spacer.h"
#include "staff.h"
#include "stafflines.h"
#include "stafftype.h"
#include "stafftypechange.h"
#include "stem.h"
#include "system.h"
#include "tie.h"
#include "tiemap.h"
#include "timesig.h"
#include "tremolo.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

namespace mu::engraving {
//---------------------------------------------------------
//   MStaff
///   Per staff values of measure.
//---------------------------------------------------------

MStaff::~MStaff()
{
    delete m_noText;
    delete m_mmRangeText;
    delete m_lines;
    delete m_vspacerUp;
    delete m_vspacerDown;
}

MStaff::MStaff(const MStaff& m)
{
    m_noText    = 0;
    m_mmRangeText = 0;
    m_lines     = Factory::copyStaffLines(*m.m_lines);
    m_hasVoices = m.m_hasVoices;
    m_vspacerUp = 0;
    m_vspacerDown = 0;
    m_visible   = m.m_visible;
    m_stemless  = m.m_stemless;
#ifndef NDEBUG
    m_corrupted = m.m_corrupted;
#endif
    m_measureRepeatCount = 0;
}

//---------------------------------------------------------
//   MStaff::setScore
//---------------------------------------------------------

void MStaff::setScore(Score* score)
{
    if (m_lines) {
        m_lines->setScore(score);
    }
    if (m_vspacerUp) {
        m_vspacerUp->setScore(score);
    }
    if (m_vspacerDown) {
        m_vspacerDown->setScore(score);
    }
    if (m_noText) {
        m_noText->setScore(score);
    }
    if (m_mmRangeText) {
        m_mmRangeText->setScore(score);
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MStaff::setTrack(track_idx_t track)
{
    if (m_lines) {
        m_lines->setTrack(track);
    }
    if (m_vspacerUp) {
        m_vspacerUp->setTrack(track);
    }
    if (m_vspacerDown) {
        m_vspacerDown->setTrack(track);
    }
    if (m_noText) {
        m_noText->setTrack(track);
    }
    if (m_mmRangeText) {
        m_mmRangeText->setTrack(track);
    }
}

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(System* parent)
    : MeasureBase(ElementType::MEASURE, parent), m_timesig(4, 4)
{
    setTicks(Fraction(4, 4));
    m_repeatCount = 2;

    size_t n = score()->nstaves();
    m_mstaves.reserve(n);
    for (staff_idx_t staffIdx = 0; staffIdx < n; ++staffIdx) {
        MStaff* ms = new MStaff;
        Staff* staff = score()->staff(staffIdx);
        ms->setLines(Factory::createStaffLines(this));
        ms->lines()->setTrack(staffIdx * VOICES);
        ms->lines()->setParent(this);
        ms->lines()->setVisible(!staff->isLinesInvisible(tick()));
        m_mstaves.push_back(ms);
    }
    setIrregular(false);
    m_noMode                = MeasureNumberMode::AUTO;
    m_userStretch           = 1.0;
    m_breakMultiMeasureRest = false;
    m_mmRest                = nullptr;
    m_mmRestCount           = 0;
    setFlag(ElementFlag::MOVABLE, false);
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure::Measure(const Measure& m)
    : MeasureBase(m)
{
    m_segments    = m.m_segments.clone();
    m_timesig     = m.m_timesig;
    _len          = m._len;
    m_repeatCount = m.m_repeatCount;
    m_noMode      = m.m_noMode;
    m_userStretch = m.m_userStretch;

    m_mstaves.reserve(m.m_mstaves.size());
    for (MStaff* ms : m.m_mstaves) {
        m_mstaves.push_back(new MStaff(*ms));
    }

    m_breakMultiMeasureRest = m.m_breakMultiMeasureRest;
    m_mmRest                = m.m_mmRest;
    m_mmRestCount           = m.m_mmRestCount;
    m_playbackCount         = m.m_playbackCount;
}

//---------------------------------------------------------
//   createStaves
//---------------------------------------------------------

void Measure::createStaves(staff_idx_t staffIdx)
{
    for (staff_idx_t n = m_mstaves.size(); n <= staffIdx; ++n) {
        Staff* staff = score()->staff(n);
        MStaff* s    = new MStaff;
        s->setLines(Factory::createStaffLines(this));
        s->lines()->setParent(this);
        s->lines()->setTrack(n * VOICES);
        s->lines()->setVisible(!staff->isLinesInvisible(tick()));
        m_mstaves.push_back(s);
    }
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Measure::setScore(Score* score)
{
    MeasureBase::setScore(score);
    for (Segment* s = first(); s; s = s->next()) {
        s->setScore(score);
    }
}

MStaff* Measure::mstaff(staff_idx_t staffIndex) const
{
    if (staffIndex < m_mstaves.size()) {
        return m_mstaves[staffIndex];
    }

    return nullptr;
}

bool Measure::hasVoices(staff_idx_t staffIdx) const
{
    MStaff* staff = mstaff(staffIdx);
    return staff ? staff->hasVoices() : false;
}

void Measure::setHasVoices(staff_idx_t staffIdx, bool v)
{
    MStaff* staff = mstaff(staffIdx);

    if (staff) {
        staff->setHasVoices(v);
    }
}

StaffLines* Measure::staffLines(staff_idx_t staffIdx)
{
    MStaff* staff = mstaff(staffIdx);

    return staff ? staff->lines() : nullptr;
}

Spacer* Measure::vspacerDown(staff_idx_t staffIdx) const
{
    MStaff* staff = mstaff(staffIdx);

    return staff ? staff->vspacerDown() : nullptr;
}

Spacer* Measure::vspacerUp(staff_idx_t staffIdx) const
{
    MStaff* staff = mstaff(staffIdx);

    return staff ? staff->vspacerUp() : nullptr;
}

void Measure::setStaffVisible(staff_idx_t staffIdx, bool visible)
{
    MStaff* staff = mstaff(staffIdx);

    if (staff) {
        staff->setVisible(visible);
    }
}

void Measure::setStaffStemless(staff_idx_t staffIdx, bool stemless)
{
    MStaff* staff = mstaff(staffIdx);

    if (staff) {
        staff->setStemless(stemless);
    }
}

void Measure::setMMRangeText(staff_idx_t staffIdx, MMRestRange* t)
{
    m_mstaves[staffIdx]->setMMRangeText(t);
}

MMRestRange* Measure::mmRangeText(staff_idx_t staffIdx) const
{
    return m_mstaves[staffIdx]->mmRangeText();
}

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::~Measure()
{
    for (Segment* s = first(); s;) {
        Segment* ns = s->next();
        delete s;
        s = ns;
    }
    DeleteAll(m_mstaves);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Measure::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

void Measure::setParent(System* s)
{
    MeasureBase::setParent(s);
}

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
    Note* note;
    double x;
};

//---------------------------------------------------------
//   findAccidental
///   return current accidental value at note position
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Note* note) const
{
    Chord* chord = note->chord();
    AccidentalState tversatz;    // state of already set accidentals for this measure
    tversatz.init(chord->staff()->keySigEvent(tick()));

    for (Segment* segment = first(); segment; segment = segment->next()) {
        track_idx_t startTrack = chord->staffIdx() * VOICES;
        if (segment->isKeySigType()) {
            KeySig* ks = toKeySig(segment->element(startTrack));
            if (!ks) {
                continue;
            }
            tversatz.init(chord->staff()->keySigEvent(segment->tick()));
        } else if (segment->segmentType() == SegmentType::ChordRest) {
            track_idx_t endTrack   = startTrack + VOICES;
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* e = segment->element(track);
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* crd = toChord(e);
                for (Chord* chord1 : crd->graceNotes()) {
                    for (Note* note1 : chord1->notes()) {
                        if (note1->tieBack() && note1->accidental() == 0) {
                            continue;
                        }
                        //
                        // compute accidental
                        //
                        int tpc  = note1->tpc();
                        int line = absStep(tpc, note1->epitch());

                        if (note == note1) {
                            return tversatz.accidentalVal(line);
                        }
                        tversatz.setAccidentalVal(line, tpc2alter(tpc));
                    }
                }
                for (Note* note1 : crd->notes()) {
                    if (note1->tieBack() && note1->accidental() == 0) {
                        continue;
                    }
                    //
                    // compute accidental
                    //
                    int tpc  = note1->tpc();
                    int line = absStep(tpc, note1->epitch());

                    if (note == note1) {
                        return tversatz.accidentalVal(line);
                    }
                    tversatz.setAccidentalVal(line, tpc2alter(tpc));
                }
            }
        }
    }
    LOGD("Measure::findAccidental: note not found");
    return AccidentalVal::NATURAL;
}

//---------------------------------------------------------
//   findAccidental
///   Compute accidental state at segment/staffIdx for
///   relative staff line.
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Segment* s, staff_idx_t staffIdx, int line, bool& error) const
{
    AccidentalState tversatz;    // state of already set accidentals for this measure
    Staff* staff = score()->staff(staffIdx);
    tversatz.init(staff->keySigEvent(tick()));

    SegmentType st = SegmentType::ChordRest;
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack   = startTrack + VOICES;
    for (Segment* segment = first(st); segment; segment = segment->next(st)) {
        if (segment == s && staff->isPitchedStaff(tick())) {
            ClefType clef = staff->clef(s->tick());
            int l = relStep(line, clef);
            return tversatz.accidentalVal(l, error);
        }
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* e = segment->element(track);
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            for (Chord* chord1 : chord->graceNotes()) {
                for (Note* note : chord1->notes()) {
                    if (note->tieBack() && note->accidental() == 0) {
                        continue;
                    }
                    int tpc  = note->tpc();
                    int l    = absStep(tpc, note->epitch());
                    tversatz.setAccidentalVal(l, tpc2alter(tpc));
                }
            }

            for (Note* note : chord->notes()) {
                if (note->tieBack() && note->accidental() == 0) {
                    continue;
                }
                int tpc    = note->tpc();
                int l      = absStep(tpc, note->epitch());
                tversatz.setAccidentalVal(l, tpc2alter(tpc));
            }
        }
    }
    LOGD("segment not found");
    return AccidentalVal::NATURAL;
}

//---------------------------------------------------------
//   tick2pos
///    return x position for tick relative to System
//---------------------------------------------------------

double Measure::tick2pos(Fraction tck) const
{
    tck -= ticks();
    if (isMMRest()) {
        Segment* s = first(SegmentType::ChordRest);
        double x1   = s->x();
        double w    = width() - x1;
        return x1 + (tck.ticks() * w) / (ticks().ticks() * mmRestCount());
    }

    Segment* s;
    double x1  = 0;
    double x2  = 0;
    Fraction tick1 = Fraction(0, 1);
    Fraction tick2 = Fraction(0, 1);
    for (s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        x2    = s->x();
        tick2 = s->rtick();
        if (tck == tick2) {
            return x2 + pos().x();
        }
        if (tck <= tick2) {
            break;
        }
        x1    = x2;
        tick1 = tick2;
    }
    if (s == 0) {
        x2    = width();
        tick2 = ticks();
    }
    double dx = x2 - x1;
    Fraction dt   = tick2 - tick1;
    x1      += dt.isZero() ? 0.0 : (dx * (tck.ticks() - tick1.ticks()) / dt.ticks());
    return x1 + pos().x();
}

//---------------------------------------------------------
//   showsMeasureNumberInAutoMode
///    Whether the measure will show measure number(s) when MeasureNumberMode is set to AUTO
//---------------------------------------------------------

bool Measure::showsMeasureNumberInAutoMode()
{
    // Check whether any measure number should be shown
    if (!score()->styleB(Sid::showMeasureNumber)) {
        return false;
    }

    // Measure numbers should not be shown on irregular measures.
    if (irregular()) {
        return false;
    }

    // Measure numbers should not show on first measure unless specified with Sid::showMeasureNumberOne
    if (!no()) {
        return score()->styleB(Sid::showMeasureNumberOne);
    }

    if (score()->styleB(Sid::measureNumberSystem)) {
        // Show either if
        //   1) This is the first measure of the system OR
        //   2) The previous measure in the system is the first, and is irregular.
        return isFirstInSystem()
               || (prevMeasure() && prevMeasure()->irregular() && prevMeasure()->isFirstInSystem());
    } else {
        // In the case of an interval, we should show the measure number either if:
        //   1) We should show them every measure
        int interval = score()->styleI(Sid::measureNumberInterval);
        if (interval == 1) {
            return true;
        }

        //   2) (measureNumber + 1) % interval == 0 (or 1 if measure number one is numbered.)
        // If measure number 1 is numbered, and the interval is let's say 5, then we should number #1, 6, 11, 16, etc.
        // If measure number 1 is not numbered, with the same interval (5), then we should number #5, 10, 15, 20, etc.
        return ((no() + 1) % score()->styleI(Sid::measureNumberInterval)) == (score()->styleB(Sid::showMeasureNumberOne) ? 1 : 0);
    }
}

//---------------------------------------------------------
//   showsMeasureNumber
///     Whether the Measure shows a MeasureNumber
//---------------------------------------------------------

bool Measure::showsMeasureNumber()
{
    if (m_noMode == MeasureNumberMode::SHOW) {
        return true;
    } else if (m_noMode == MeasureNumberMode::HIDE) {
        return false;
    } else {
        return showsMeasureNumberInAutoMode();
    }
}

//---------------------------------------------------------
//   findChord
///   Search for chord at position \a tick in \a track
//---------------------------------------------------------

Chord* Measure::findChord(Fraction t, track_idx_t track)
{
    t -= tick();
    for (Segment* seg = last(); seg; seg = seg->prev()) {
        if (seg->rtick() < t) {
            return 0;
        }
        if (seg->rtick() == t) {
            EngravingItem* el = seg->element(track);
            if (el && el->isChord()) {
                return toChord(el);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   findChordRest
///   Search for chord or rest at position \a tick at \a staff in \a voice.
//---------------------------------------------------------

ChordRest* Measure::findChordRest(Fraction t, track_idx_t track)
{
    t -= tick();
    for (const Segment& seg : m_segments) {
        if (seg.rtick() > t) {
            return 0;
        }
        if (seg.rtick() == t) {
            EngravingItem* el = seg.element(track);
            if (el && el->isChordRest()) {
                return toChordRest(el);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Measure::tick2segment(const Fraction& _t, SegmentType st)
{
    Fraction t = _t - tick();
    for (Segment& s : m_segments) {
        if (s.rtick() == t) {
            if (s.segmentType() & st) {
                return &s;
            }
        }
        if (s.rtick() > t) {
            break;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   findSegmentR
//    Search for a segment of type st at measure relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findSegmentR(SegmentType st, const Fraction& t) const
{
    Segment* s;
    if (t > (ticks() * Fraction(1, 2))) {
        // search backwards
        for (s = last(); s && s->rtick() > t; s = s->prev()) {
        }
        while (s && s->prev() && s->prev()->rtick() == t) {
            s = s->prev();
        }
    } else {
        // search forwards
        for (s = first(); s && s->rtick() < t; s = s->next()) {
        }
    }
    for (; s && s->rtick() == t; s = s->next()) {
        if (s->segmentType() & st) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   undoGetSegmentR
//---------------------------------------------------------

Segment* Measure::undoGetSegmentR(SegmentType type, const Fraction& t)
{
    Segment* s = findSegmentR(type, t);
    if (s == 0) {
        s = Factory::createSegment(this, type, t);
        score()->undoAddElement(s);
    }
    return s;
}

//---------------------------------------------------------
//   findFirstR
//    return first segment of type st at relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findFirstR(SegmentType st, const Fraction& t) const
{
    Segment* s;
    // search forwards
    for (s = first(); s && s->rtick() <= t; s = s->next()) {
        if (s->segmentType() == st) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   getSegmentR
///   Get a segment of type st at relative tick position t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegmentR(SegmentType st, const Fraction& t)
{
    Segment* s = findSegmentR(st, t);
    if (!s) {
        s = Factory::createSegment(this, st, t);
        add(s);
    }
    return s;
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a el to Measure.
//---------------------------------------------------------

void Measure::add(EngravingItem* e)
{
    if (e->explicitParent() != this) {
        e->setParent(this);
    }

    ElementType type = e->type();

    switch (type) {
    case ElementType::SEGMENT:
    {
        Segment* seg   = toSegment(e);
        Fraction t     = seg->rtick();
        Segment* s;

        for (s = first(); s && s->rtick() < t; s = s->next()) {
        }
        while (s && s->rtick() == t) {
            if (!seg->isChordRestType() && (seg->segmentType() == s->segmentType())) {
                LOGD("there is already a <%s> segment", seg->subTypeName());
                return;
            }
            if (seg->goesBefore(s)) {
                break;
            }
            s = s->next();
        }
        seg->setParent(this);
        m_segments.insert(seg, s);
        //
        // update measure flags
        //
        if (seg->header()) {
            seg->measure()->setHeader(true);
        }
        if (seg->trailer()) {
            seg->measure()->setTrailer(true);
        }
    }
    break;

    case ElementType::MEASURE_NUMBER:
        if (e->staffIdx() < m_mstaves.size()) {
            if (e->isStyled(Pid::OFFSET)) {
                e->setOffset(e->propertyDefault(Pid::OFFSET).value<PointF>());
            }
            m_mstaves[e->staffIdx()]->setNoText(toMeasureNumber(e));
        }
        break;

    case ElementType::MMREST_RANGE:
        if (e->staffIdx() < m_mstaves.size()) {
            if (e->isStyled(Pid::OFFSET)) {
                e->setOffset(e->propertyDefault(Pid::OFFSET).value<PointF>());
            }
            m_mstaves[e->staffIdx()]->setMMRangeText(toMMRestRange(e));
        }
        break;

    case ElementType::SPACER:
    {
        Spacer* sp = toSpacer(e);
        switch (sp->spacerType()) {
        case SpacerType::UP:
            m_mstaves[e->staffIdx()]->setVspacerUp(sp);
            break;
        case SpacerType::DOWN:
        case SpacerType::FIXED:
            m_mstaves[e->staffIdx()]->setVspacerDown(sp);
            break;
        }
        sp->setGap(sp->gap());            // trigger relayout
    }
    break;
    case ElementType::JUMP:
        setRepeatJump(true);
    // fall through

    case ElementType::MARKER:
        el().push_back(e);
        break;

    case ElementType::HBOX:
        if (e->staff()) {
            e->setMag(e->staff()->staffMag(tick()));                 // ?!
        }
        el().push_back(e);
        break;

    case ElementType::MEASURE:
        m_mmRest = toMeasure(e);
        break;

    case ElementType::STAFFTYPE_CHANGE:
    {
        StaffTypeChange* staffTypeChange = toStaffTypeChange(e);
        const StaffType* templateStaffType = staffTypeChange->staffType();

        Staff* staff = staffTypeChange->staff();

        // This will need to point to the stafftype element within the stafftypelist for the staff
        StaffType* newStaffType = nullptr;

        if (templateStaffType) {
            // executed on read, undo/redo, clone
            // setStaffType adds a copy to stafftypelist and returns a pointer to that element within stafftypelist
            newStaffType = staff->setStaffType(tick(), *templateStaffType);
        } else {
            // executed on add from palette
            // staffType returns a pointer to the current stafftype element in the list
            // setStaffType will make a copy and return a pointer to that element within list
            templateStaffType = staff->staffType(tick());
            newStaffType = staff->setStaffType(tick(), *templateStaffType);
        }

        staff->staffTypeListChanged(tick());
        staffTypeChange->setStaffType(newStaffType, false);

        MeasureBase::add(e);
    }
        return;

    default:
        MeasureBase::add(e);
        return;
    }

    e->added();
}

//---------------------------------------------------------
//   remove
///   Remove EngravingItem \a el from Measure.
//---------------------------------------------------------

void Measure::remove(EngravingItem* e)
{
    assert(e->explicitParent() == this);
    assert(e->score() == score());

    switch (e->type()) {
    case ElementType::SEGMENT:
    {
        Segment* s = toSegment(e);
        m_segments.remove(s);
        //
        // update measure flags
        //
        if (s->header()) {
            s->measure()->checkHeader();
        }
        if (s->trailer()) {
            s->measure()->checkTrailer();
        }
    }
    break;

    case ElementType::MEASURE_NUMBER:
        m_mstaves[e->staffIdx()]->setNoText(nullptr);
        break;

    case ElementType::MMREST_RANGE:
        m_mstaves[e->staffIdx()]->setMMRangeText(nullptr);
        break;

    case ElementType::SPACER:
        switch (toSpacer(e)->spacerType()) {
        case SpacerType::DOWN:
        case SpacerType::FIXED:
            m_mstaves[e->staffIdx()]->setVspacerDown(0);
            break;
        case SpacerType::UP:
            m_mstaves[e->staffIdx()]->setVspacerUp(0);
            break;
        }
        break;

    case ElementType::JUMP:
        setRepeatJump(false);
    // fall through

    case ElementType::MARKER:
    case ElementType::HBOX:
        if (!el().remove(e)) {
            LOGD("Measure(%p)::remove(%s,%p) not found", this, e->typeName(), e);
        }
        break;

    case ElementType::CLEF:
    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::MMREST:
    case ElementType::TIMESIG:
        for (Segment* segment = first(); segment; segment = segment->next()) {
            size_t staves = score()->nstaves();
            size_t tracks = staves * VOICES;
            for (track_idx_t track = 0; track < tracks; ++track) {
                EngravingItem* ee = segment->element(track);
                if (ee == e) {
                    segment->setElement(track, nullptr);
                    e->removed();
                    return;
                }
            }
        }
        LOGD("Measure::remove: %s %p not found", e->typeName(), e);
        break;

    case ElementType::MEASURE:
        m_mmRest = 0;
        break;

    case ElementType::STAFFTYPE_CHANGE:
    {
        StaffTypeChange* stc = toStaffTypeChange(e);
        Staff* staff = stc->staff();
        if (staff) {
            // st currently points to an list element that is about to be removed
            // make a copy now to use on undo/redo
            StaffType* st = new StaffType(*stc->staffType());
            if (!tick().isZero()) {
                staff->removeStaffType(tick());
            }
            stc->setStaffType(st, true);
        }
        MeasureBase::remove(e);
    }
        return;

    default:
        MeasureBase::remove(e);
        return;
    }

    e->removed();
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Measure::change(EngravingItem* o, EngravingItem* n)
{
    if (o->isTuplet()) {
        Tuplet* t = toTuplet(n);
        for (DurationElement* e : t->elements()) {
            e->setTuplet(t);
        }
    } else {
        remove(o);
        add(n);
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Measure::spatiumChanged(double /*oldValue*/, double /*newValue*/)
{
}

//-------------------------------------------------------------------
//   moveTicks
//    Also adjust endBarLine if measure len has changed. For this
//    diff == 0 cannot be optimized away
//-------------------------------------------------------------------

void Measure::moveTicks(const Fraction& diff)
{
    // This part must be run also for diff = 0
    for (Segment* segment = last(); segment; segment = segment->prev()) {
        if (segment->segmentType() & (SegmentType::EndBarLine | SegmentType::TimeSigAnnounce)) {
            segment->setRtick(ticks());
        }
    }
    if (diff.numerator() == 0) {
        return;
    }
    std::set<Tuplet*> tuplets;
    setTick(tick() + diff);
    for (Segment* segment = last(); segment; segment = segment->prev()) {
        if (segment->isChordRestType()) {
            // Tuplet ticks are stored as absolute ticks, so they must be adjusted.
            // But each tuplet must only be adjusted once.
            for (EngravingItem* e : segment->elist()) {
                if (e) {
                    ChordRest* cr = toChordRest(e);
                    Tuplet* tuplet = cr->tuplet();
                    if (tuplet && tuplets.count(tuplet) == 0) {
                        tuplet->setTick(tuplet->tick() + diff);
                        tuplets.insert(tuplet);
                    }
                }
            }
        }
    }
    tuplets.clear();
}

//---------------------------------------------------------
//   removeStaves
//---------------------------------------------------------

void Measure::removeStaves(staff_idx_t sStaff, staff_idx_t eStaff)
{
    for (Segment* s = first(); s; s = s->next()) {
        for (int staff = static_cast<int>(eStaff) - 1; staff >= static_cast<int>(sStaff); --staff) {
            s->removeStaff(staff);
        }
    }
    for (EngravingItem* e : el()) {
        if (e->track() == mu::nidx) {
            continue;
        }
        voice_idx_t voice = e->voice();
        staff_idx_t staffIdx = e->staffIdx();
        if (staffIdx >= eStaff) {
            staffIdx -= eStaff - sStaff;
            e->setTrack(staffIdx * VOICES + voice);
        }
    }
}

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(staff_idx_t sStaff, staff_idx_t eStaff)
{
    for (EngravingItem* e : el()) {
        if (e->track() == mu::nidx) {
            continue;
        }
        staff_idx_t staffIdx = e->staffIdx();
        if (moveDownWhenAddingStaves(e, sStaff, eStaff)) {
            voice_idx_t voice = e->voice();
            staffIdx += eStaff - sStaff;
            e->setTrack(staffIdx * VOICES + voice);
        }
    }
    for (Segment* s = first(); s; s = s->next()) {
        for (staff_idx_t staff = sStaff; staff < eStaff; ++staff) {
            s->insertStaff(staff);
        }
    }
}

//---------------------------------------------------------
//   cmdRemoveStaves
//---------------------------------------------------------

void Measure::cmdRemoveStaves(staff_idx_t sStaff, staff_idx_t eStaff)
{
    track_idx_t sTrack = sStaff * VOICES;
    track_idx_t eTrack = eStaff * VOICES;

    for (Segment* s = first(); s; s = s->next()) {
        for (int track = static_cast<int>(eTrack) - 1; track >= static_cast<int>(sTrack); --track) {
            EngravingItem* el = s->element(track);
            if (el) {
                el->undoUnlink();
                score()->undo(new RemoveElement(el));
            }
        }

        // Create copy, because s->annotations() will be modified during the loop
        std::vector<EngravingItem*> annotations = s->annotations();
        for (EngravingItem* e : annotations) {
            if (allowRemoveWhenRemovingStaves(e, sStaff, eStaff)) {
                e->undoUnlink();
                score()->undo(new RemoveElement(e));
            }
        }
    }

    for (EngravingItem* e : el()) {
        if (e->track() == mu::nidx) {
            continue;
        }

        if (allowRemoveWhenRemovingStaves(e, sStaff, eStaff)) {
            e->undoUnlink();
            score()->undo(new RemoveElement(e));
        }
    }

    score()->undo(new RemoveStaves(this, sStaff, eStaff));

    for (int i = static_cast<int>(eStaff) - 1; i >= static_cast<int>(sStaff); --i) {
        MStaff* ms = *(m_mstaves.begin() + i);
        score()->undo(new RemoveMStaff(this, ms, i));
    }
}

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(staff_idx_t sStaff, staff_idx_t eStaff, bool createRest)
{
    score()->undo(new InsertStaves(this, sStaff, eStaff));

    Segment* ts = findSegment(SegmentType::TimeSig, tick());
    Segment* bs = findSegmentR(SegmentType::EndBarLine, ticks());

    for (staff_idx_t i = sStaff; i < eStaff; ++i) {
        Staff* staff = score()->staff(i);
        MStaff* ms   = new MStaff;
        ms->setLines(Factory::createStaffLines(this));
        ms->lines()->setTrack(i * VOICES);
        ms->lines()->setParent(this);
        ms->lines()->setVisible(!staff->isLinesInvisible(tick()));
        score()->undo(new InsertMStaff(this, ms, i));
    }

    if (!createRest && !ts) {
        return;
    }

    // create list of unique staves (only one instance for linked staves):

    std::list<staff_idx_t> sl;
    for (staff_idx_t staffIdx = sStaff; staffIdx < eStaff; ++staffIdx) {
        Staff* s = score()->staff(staffIdx);
        if (s->links()) {
            bool alreadyInList = false;
            for (staff_idx_t idx : sl) {
                if (s->links()->contains(score()->staff(idx))) {
                    alreadyInList = true;
                    break;
                }
            }
            if (alreadyInList) {
                continue;
            }
        }
        sl.push_back(staffIdx);
    }

    for (staff_idx_t staffIdx : sl) {
        if (createRest) {
            score()->setRest(tick(), staffIdx * VOICES, ticks(), false, 0, m_timesig == ticks());
        }

        // replicate time signature
        if (ts) {
            TimeSig* ots = 0;
            bool constructed = false;
            for (track_idx_t track = 0; track < m_mstaves.size() * VOICES; ++track) {
                if (ts->element(track)) {
                    ots = toTimeSig(ts->element(track));
                    break;
                }
            }
            if (!ots) {
                // no time signature found; use measure timesig to construct one
                ots = Factory::createTimeSig(score()->dummy()->segment());
                ots->setSig(timesig());
                constructed = true;
            }
            // do no replicate local time signatures
            if (ots && !ots->isLocal()) {
                TimeSig* timesig = Factory::copyTimeSig(*ots);
                timesig->setTrack(staffIdx * VOICES);
                timesig->setParent(ts);
                timesig->setSig(ots->sig(), ots->timeSigType());
                score()->undoAddElement(timesig);
                if (constructed) {
                    delete ots;
                }
            }
        }

        // replicate barline
        if (bs) {
            BarLine* obl = nullptr;
            for (track_idx_t track = 0; track < m_mstaves.size() * VOICES; ++track) {
                EngravingItem* e = bs->element(track);
                if (e && !e->generated()) {
                    obl = toBarLine(e);
                    break;
                }
            }
            if (obl) {
                BarLine* barline = Factory::copyBarLine(*obl);
                barline->setSpanStaff(score()->staff(staffIdx)->barLineSpan());
                barline->setTrack(staffIdx * VOICES);
                barline->setParent(bs);
                barline->setGenerated(false);
                score()->undoAddElement(barline);
            }
        }
    }
}

//---------------------------------------------------------
//   insertMStaff
//---------------------------------------------------------

void Measure::insertMStaff(MStaff* staff, staff_idx_t idx)
{
    m_mstaves.insert(m_mstaves.begin() + idx, staff);
    for (staff_idx_t staffIdx = 0; staffIdx < m_mstaves.size(); ++staffIdx) {
        m_mstaves[staffIdx]->setTrack(staffIdx * VOICES);
    }
}

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, staff_idx_t idx)
{
    m_mstaves.erase(m_mstaves.begin() + idx);
    for (staff_idx_t staffIdx = 0; staffIdx < m_mstaves.size(); ++staffIdx) {
        m_mstaves[staffIdx]->setTrack(staffIdx * VOICES);
    }
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, staff_idx_t staffIdx)
{
    for (Segment* s = first(); s; s = s->next()) {
        s->insertStaff(staffIdx);
    }

    MStaff* ms = new MStaff;
    ms->setLines(Factory::createStaffLines(this));
    ms->lines()->setParent(this);
    ms->lines()->setTrack(staffIdx * VOICES);
    ms->lines()->setVisible(!staff->isLinesInvisible(tick()));
    insertMStaff(ms, staffIdx);
}

//---------------------------------------------------------
//   staffabbox
//---------------------------------------------------------

RectF Measure::staffabbox(staff_idx_t staffIdx) const
{
    System* s = system();
    RectF sb(s->staff(staffIdx)->bbox());
    RectF rrr(sb.translated(s->pagePos()));
    RectF rr(abbox());
    RectF r(rr.x(), rrr.y(), rr.width(), rrr.height());
    return r;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

/**
 Return true if an EngravingItem of type \a type can be dropped on a Measure
*/

bool Measure::acceptDrop(EditData& data) const
{
    MuseScoreView* viewer = data.view();
    PointF pos = data.pos;
    EngravingItem* e = data.dropElement;

    staff_idx_t staffIdx;
    Segment* seg;
    if (!score()->pos2measure(pos, &staffIdx, 0, &seg, 0)) {
        return false;
    }

    RectF staffR = system()->staff(staffIdx)->bbox().translated(system()->canvasPos());
    staffR.intersect(canvasBoundingRect());

    switch (e->type()) {
    case ElementType::MEASURE_LIST:
    case ElementType::JUMP:
    case ElementType::MARKER:
    case ElementType::LAYOUT_BREAK:
    case ElementType::STAFF_LIST:
        viewer->setDropRectangle(canvasBoundingRect());
        return true;

    case ElementType::KEYSIG:
    case ElementType::TIMESIG:
        if (data.modifiers & ControlModifier) {
            viewer->setDropRectangle(staffR);
        } else {
            viewer->setDropRectangle(canvasBoundingRect());
        }
        return true;

    case ElementType::MEASURE_NUMBER:
        viewer->setDropRectangle(canvasBoundingRect());
        return true;

    case ElementType::BRACKET:
    case ElementType::MEASURE_REPEAT:
    case ElementType::MEASURE:
    case ElementType::SPACER:
    case ElementType::IMAGE:
    case ElementType::BAR_LINE:
    case ElementType::SYMBOL:
    case ElementType::CLEF:
    case ElementType::STAFFTYPE_CHANGE:
        viewer->setDropRectangle(staffR);
        return true;

    case ElementType::ACTION_ICON:
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::VFRAME:
        case ActionIconType::HFRAME:
        case ActionIconType::TFRAME:
        case ActionIconType::FFRAME:
        case ActionIconType::MEASURE:
            viewer->setDropRectangle(canvasBoundingRect());
            return true;
        case ActionIconType::STAFF_TYPE_CHANGE:
            viewer->setDropRectangle(staffR);
            return true;
        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   drop
///   Drop element.
///   Handle a dropped element at position \a pos of given
///   element \a type and \a subtype.
//---------------------------------------------------------

EngravingItem* Measure::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    staff_idx_t staffIdx = mu::nidx;
    Segment* seg = nullptr;
    score()->pos2measure(data.pos, &staffIdx, 0, &seg, 0);

    if (staffIdx == mu::nidx) {
        return nullptr;
    }
    Staff* staff = score()->staff(staffIdx);
    //bool fromPalette = (e->track() == -1);

    switch (e->type()) {
    case ElementType::MEASURE_LIST:
        delete e;
        break;

    case ElementType::STAFF_LIST:
        delete e;
        break;

    case ElementType::MARKER:
    case ElementType::JUMP:
        e->setParent(this);
        e->setTrack(0);
        score()->undoAddElement(e);
        return e;

    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::FRET_DIAGRAM:
        e->setParent(seg);
        e->setTrack(staffIdx * VOICES);
        score()->undoAddElement(e);
        return e;

    case ElementType::IMAGE:
    case ElementType::SYMBOL:
        e->setParent(seg);
        e->setTrack(staffIdx * VOICES);

        layout()->layoutItem(e);

        {
            PointF uo(data.pos - e->canvasPos() - data.dragOffset);
            e->setOffset(uo);
        }
        score()->undoAddElement(e);
        return e;

    case ElementType::MEASURE_NUMBER:
        undoChangeProperty(Pid::MEASURE_NUMBER_MODE, static_cast<int>(MeasureNumberMode::SHOW));
        delete e;
        break;

    case ElementType::BRACKET:
    {
        Bracket* b = toBracket(e);
        size_t level = 0;
        staff_idx_t firstStaff = 0;
        for (Staff* s : score()->staves()) {
            for (const BracketItem* bi : s->brackets()) {
                staff_idx_t lastStaff = firstStaff + bi->bracketSpan() - 1;
                if (staffIdx >= firstStaff && staffIdx <= lastStaff) {
                    ++level;
                }
            }
            firstStaff++;
        }
        Selection sel = score()->selection();
        if (sel.isRange()) {
            score()->undoAddBracket(staff, level, b->bracketType(), sel.staffEnd() - sel.staffStart());
        } else {
            score()->undoAddBracket(staff, level, b->bracketType(), 1);
        }
        delete b;
    }
    break;

    case ElementType::CLEF:
        score()->undoChangeClef(staff, this, toClef(e)->clefType());
        delete e;
        break;

    case ElementType::KEYSIG:
    {
        KeySigEvent k = toKeySig(e)->keySigEvent();
        delete e;

        if (data.modifiers & ControlModifier) {
            // apply only to this stave
            score()->undoChangeKeySig(staff, tick(), k);
        } else {
            // apply to all staves:
            for (Staff* s : score()->staves()) {
                score()->undoChangeKeySig(s, tick(), k);
            }
        }

        break;
    }

    case ElementType::TIMESIG:
        score()->cmdAddTimeSig(this, staffIdx, toTimeSig(e), data.modifiers & ControlModifier);
        break;

    case ElementType::LAYOUT_BREAK: {
        LayoutBreak* b = toLayoutBreak(e);
        Measure* measure = isMMRest() ? mmRestLast() : this;
        switch (b->layoutBreakType()) {
        case  LayoutBreakType::PAGE:
            if (measure->pageBreak()) {
                delete b;
                b = 0;
            } else {
                measure->setLineBreak(false);
            }
            break;
        case  LayoutBreakType::LINE:
            if (measure->lineBreak()) {
                delete b;
                b = 0;
            } else {
                measure->setPageBreak(false);
            }
            break;
        case  LayoutBreakType::SECTION:
            if (measure->sectionBreak()) {
                delete b;
                b = 0;
            } else {
                measure->setLineBreak(false);
            }
            break;
        case LayoutBreakType::NOBREAK:
            if (measure->noBreak()) {
                delete b;
                b = 0;
            } else {
                measure->setLineBreak(false);
                measure->setPageBreak(false);
            }
            break;
        }
        if (b) {
            b->setTrack(mu::nidx);                   // these are system elements
            b->setParent(measure);
            score()->undoAddElement(b);
        }
        measure->cleanupLayoutBreaks(true);
        return b;
    }

    case ElementType::SPACER:
    {
        Spacer* spacer = toSpacer(e);
        spacer->setTrack(staffIdx * VOICES);
        spacer->setParent(this);
        if (spacer->spacerType() == SpacerType::FIXED) {
            double gap = spatium() * 10;
            System* s = system();
            const staff_idx_t nextVisStaffIdx = s->nextVisibleStaff(staffIdx);
            const bool systemEnd = (nextVisStaffIdx == mu::nidx);
            if (systemEnd) {
                System* ns = 0;
                for (System* ts : score()->systems()) {
                    if (ns) {
                        ns = ts;
                        break;
                    }
                    if (ts == s) {
                        ns = ts;
                    }
                }
                if (ns == s) {
                    double y1 = s->staffYpage(staffIdx);
                    double y2 = s->page()->height() - s->page()->bm();
                    gap = y2 - y1 - score()->staff(staffIdx)->height();
                } else if (ns && ns->page() == s->page()) {
                    double y1 = s->staffYpage(staffIdx);
                    double y2 = ns->staffYpage(0);
                    gap = y2 - y1 - score()->staff(staffIdx)->height();
                }
            } else {
                double y1 = s->staffYpage(staffIdx);
                double y2 = s->staffYpage(nextVisStaffIdx);
                gap = y2 - y1 - score()->staff(staffIdx)->height();
            }
            spacer->setGap(Millimetre(gap));
        }
        score()->undoAddElement(spacer);
        triggerLayout();
        return spacer;
    }

    case ElementType::BAR_LINE:
    {
        BarLine* bl = toBarLine(e);

        // if dropped bar line refers to span rather than to subtype
        // or if Ctrl key used
        if ((bl->spanFrom() && bl->spanTo()) || data.control()) {
            // get existing bar line for this staff, and drop the change to it
            seg = undoGetSegmentR(SegmentType::EndBarLine, ticks());
            BarLine* cbl = toBarLine(seg->element(staffIdx * VOICES));
            if (cbl) {
                cbl->drop(data);
            }
        } else if (bl->barLineType() == BarLineType::START_REPEAT) {
            Measure* m2 = isMMRest() ? mmRestFirst() : this;
            for (size_t stIdx = 0; stIdx < score()->nstaves(); ++stIdx) {
                if (m2->isMeasureRepeatGroupWithPrevM(stIdx)) {
                    MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                    return nullptr;
                }
            }
            for (Score* lscore : score()->scoreList()) {
                Measure* lmeasure = lscore->tick2measure(m2->tick());
                if (lmeasure) {
                    lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
                }
            }
        } else if (bl->barLineType() == BarLineType::END_REPEAT) {
            Measure* m2 = isMMRest() ? mmRestLast() : this;
            for (size_t stIdx = 0; stIdx < score()->nstaves(); ++stIdx) {
                if (m2->isMeasureRepeatGroupWithNextM(stIdx)) {
                    MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                    return nullptr;
                }
            }
            for (Score* lscore : score()->scoreList()) {
                Measure* lmeasure = lscore->tick2measure(m2->tick());
                if (lmeasure) {
                    lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
                }
            }
        } else if (bl->barLineType() == BarLineType::END_START_REPEAT) {
            Measure* m2 = isMMRest() ? mmRestLast() : this;
            for (size_t stIdx = 0; stIdx < score()->nstaves(); ++stIdx) {
                if (m2->isMeasureRepeatGroupWithNextM(stIdx)) {
                    MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                    return nullptr;
                }
            }
            for (Score* lscore : score()->scoreList()) {
                Measure* lmeasure = lscore->tick2measure(m2->tick());
                if (lmeasure) {
                    lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
                    lmeasure = lmeasure->nextMeasure();
                    if (lmeasure) {
                        lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
                    }
                }
            }
        } else {
            // drop to first end barline
            seg = findSegmentR(SegmentType::EndBarLine, ticks());
            if (seg) {
                for (EngravingItem* ee : seg->elist()) {
                    if (ee) {
                        ee->drop(data);
                        break;
                    }
                }
            } else {
                delete e;
            }
        }
        break;
    }

    case ElementType::MEASURE_REPEAT:
    {
        int numMeasures = toMeasureRepeat(e)->numMeasures();
        delete e;
        score()->cmdAddMeasureRepeat(this, numMeasures, staffIdx);
        break;
    }
    case ElementType::ACTION_ICON:
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX, this);
            break;
        case ActionIconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX, this);
            break;
        case ActionIconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX, this);
            break;
        case ActionIconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX, this);
            break;
        case ActionIconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE, this);
            break;
        case ActionIconType::STAFF_TYPE_CHANGE: {
            EngravingItem* stc = Factory::createStaffTypeChange(this);
            stc->setParent(this);
            stc->setTrack(staffIdx * VOICES);
            score()->undoAddElement(stc);
            break;
        }
        default:
            break;
        }
        break;

    case ElementType::STAFFTYPE_CHANGE:
    {
        e->setParent(this);
        e->setTrack(staffIdx * VOICES);
        score()->undoAddElement(e);
    }
    break;

    default:
        LOGD("Measure: cannot drop %s here", e->typeName());
        delete e;
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   adjustToLen
//    change actual measure len, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(Fraction nf, bool appendRestsIfNecessary)
{
    Fraction ol   = ticks();
    Fraction nl   = nf;
    Fraction diff = nl - ol;

    Fraction startTick = endTick();
    if (diff < Fraction(0, 1)) {
        startTick += diff;
    }

    score()->undoInsertTime(startTick, diff);
    score()->undo(new InsertTime(score(), startTick, diff));

    for (Score* s : score()->scoreList()) {
        Measure* m = s->tick2measure(tick());
        s->undo(new ChangeMeasureLen(m, nf));
        if (nl > ol) {
            // move EndBarLine, TimeSigAnnounce, KeySigAnnounce
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                if (seg->segmentType() & (SegmentType::EndBarLine | SegmentType::TimeSigAnnounce | SegmentType::KeySigAnnounce)) {
                    seg->setRtick(nl);
                }
            }
        }
    }
    Score* s      = score()->masterScore();
    Measure* m    = s->tick2measure(tick());
    std::list<staff_idx_t> sl = s->uniqueStaves();

    for (staff_idx_t staffIdx : sl) {
        int rests  = 0;
        int chords = 0;
        Rest* rest = 0;
        for (Segment* segment = m->first(); segment; segment = segment->next()) {
            track_idx_t strack = staffIdx * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                EngravingItem* e = segment->element(track);
                if (e) {
                    if (e->isRest()) {
                        ++rests;
                        rest = toRest(e);
                    } else if (e->isChord()) {
                        ++chords;
                    }
                }
            }
        }
        Fraction stretch = s->staff(staffIdx)->timeStretch(tick());
        // if just a single rest
        if (rests == 1 && chords == 0) {
            // if measure value didn't change, stick to whole measure rest
            if (m_timesig == nf) {
                rest->undoChangeProperty(Pid::DURATION, nf * stretch);
                rest->undoChangeProperty(Pid::DURATION_TYPE_WITH_DOTS, DurationTypeWithDots(DurationType::V_MEASURE));
            } else {          // if measure value did change, represent with rests actual measure value
                // convert the measure duration in a list of values (no dots for rests)
                std::vector<TDuration> durList = toDurationList(nf * stretch, false, 0);

                // set the existing rest to the first value of the duration list
                for (EngravingObject* e : rest->linkList()) {
                    e->undoChangeProperty(Pid::DURATION, durList[0].fraction());
                    e->undoChangeProperty(Pid::DURATION_TYPE_WITH_DOTS, durList[0].typeWithDots());
                }

                // add rests for any other duration list value
                Fraction tickOffset = tick() + rest->actualTicks();
                for (unsigned i = 1; i < durList.size(); i++) {
                    Rest* newRest = Factory::createRest(s->dummy()->segment());
                    newRest->setDurationType(durList.at(i));
                    newRest->setTicks(durList.at(i).fraction());
                    newRest->setTrack(rest->track());
                    score()->undoAddCR(newRest, this, tickOffset);
                    tickOffset += newRest->actualTicks();
                }
            }
            continue;
        }

        track_idx_t strack = staffIdx * VOICES;
        track_idx_t etrack = strack + VOICES;

        for (track_idx_t trk = strack; trk < etrack; ++trk) {
            Fraction n = diff;
            bool rFlag = false;
            if (n < Fraction(0, 1)) {
                for (Segment* segment = m->last(); segment;) {
                    Segment* pseg = segment->prev();
                    if (segment->segmentType() == SegmentType::ChordRest) {
                        const auto annotations = segment->annotations(); // make a copy since we alter the list
                        for (EngravingItem* a : annotations) {
                            if (a->track() == trk) {
                                s->undoRemoveElement(a);
                            }
                        }
                        EngravingItem* e = segment->element(trk);
                        if (e && e->isChordRest()) {
                            ChordRest* cr = toChordRest(e);
                            if (cr->durationType() == DurationType::V_MEASURE) {
                                Fraction actualTicks = cr->actualTicks();
                                n += actualTicks;
                                cr->setDurationType(TDuration(actualTicks));
                            } else {
                                n += cr->actualTicks();
                            }
                            s->undoRemoveElement(e);
                            if (n >= Fraction(0, 1)) {
                                break;
                            }
                        }
                    } else if (segment->segmentType() == SegmentType::Breath) {
                        EngravingItem* e = segment->element(trk);
                        if (e) {
                            s->undoRemoveElement(e);
                        }
                    }
                    segment = pseg;
                }
                rFlag = true;
            }
            voice_idx_t voice = trk % VOICES;
            if (appendRestsIfNecessary && (n > Fraction(0, 1)) && (rFlag || voice == 0)) {
                // add rest to measure
                Fraction rtick = tick() + nl - n;
                track_idx_t track = staffIdx * VOICES + voice;
                s->setRest(rtick, track, n * stretch, false, 0, false);
            }
        }
    }
    if (diff < Fraction(0, 1)) {
        //
        //  CHECK: do not remove all slurs
        //
        for (EngravingItem* e : m->el()) {
            if (e->isSlur()) {
                s->undoRemoveElement(e);
            }
        }
    }
}

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool Measure::visible(staff_idx_t staffIdx) const
{
    if (staffIdx >= score()->staves().size()) {
        LOGD("Measure::visible: bad staffIdx: %zu", staffIdx);
        return false;
    }
    if (system() && (system()->staves().empty() || !system()->staff(staffIdx)->show())) {
        return false;
    }
    if (score()->staff(staffIdx)->cutaway() && isEmpty(staffIdx)) {
        return false;
    }
    return score()->staff(staffIdx)->show() && m_mstaves[staffIdx]->visible();
}

//---------------------------------------------------------
//   stemless
//---------------------------------------------------------

bool Measure::stemless(staff_idx_t staffIdx) const
{
    const Staff* staff = score()->staff(staffIdx);
    return staff->stemless(tick()) || m_mstaves[staffIdx]->stemless() || staff->staffType(tick())->stemless();
}

//---------------------------------------------------------
//   isFinalMeasureOfSection
//    returns true if this measure is final actual measure of a section
//    takes into consideration fact that subsequent measures base objects
//    may have section break before encountering next actual measure
//---------------------------------------------------------

bool Measure::isFinalMeasureOfSection() const
{
    const MeasureBase* mb = static_cast<const MeasureBase*>(this);

    do {
        if (mb->sectionBreak()) {
            return true;
        }

        mb = mb->next();
    } while (mb && !mb->isMeasure());           // loop until reach next actual measure or end of score

    return false;
}

//---------------------------------------------------------
//   isAnacrusis
//---------------------------------------------------------

bool Measure::isAnacrusis() const
{
    TimeSigFrac timeSig = score()->sigmap()->timesig(tick().ticks()).nominal();
    return irregular() && ticks() < Fraction::fromTicks(timeSig.ticksPerMeasure());
}

//---------------------------------------------------------
//   isFirstInSystem
//---------------------------------------------------------

bool Measure::isFirstInSystem() const
{
    IF_ASSERT_FAILED(system()) {
        return false;
    }
    return system()->firstMeasure() == this;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Measure::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    size_t nstaves = score()->nstaves();
    if (!all && nstaves == 0) {
        return;
    }

    MeasureBase::scanElements(data, func, all);

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (!all && !(visible(staffIdx) && score()->staff(staffIdx)->show()) && !isCutawayClef(staffIdx)) {
            continue;
        }
        MStaff* ms = m_mstaves[staffIdx];
        func(data, ms->lines());
        if (ms->vspacerUp()) {
            func(data, ms->vspacerUp());
        }
        if (ms->vspacerDown()) {
            func(data, ms->vspacerDown());
        }
        if (ms->noText()) {
            func(data, ms->noText());
        }
        if (ms->mmRangeText()) {
            func(data, ms->mmRangeText());
        }
    }

    for (Segment* s = first(); s; s = s->next()) {
        if (!s->enabled()) {
            continue;
        }
        s->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   connectTremolo
///   Connect two-notes tremolo and update duration types
///   for the involved chords.
//---------------------------------------------------------

void Measure::connectTremolo()
{
    const size_t ntracks = score()->ntracks();
    constexpr SegmentType st = SegmentType::ChordRest;
    for (Segment* s = first(st); s; s = s->next(st)) {
        for (track_idx_t i = 0; i < ntracks; ++i) {
            EngravingItem* e = s->element(i);
            if (!e || !e->isChord()) {
                continue;
            }

            Chord* c = toChord(e);
            Tremolo* tremolo = c->tremolo();
            if (tremolo && tremolo->twoNotes()) {
                // Ensure correct duration type for chord
                c->setDurationType(tremolo->durationType());

                // If it is the first tremolo's chord, find the second
                // chord for tremolo, if needed.
                if (!tremolo->chord1()) {
                    tremolo->setChords(c, tremolo->chord2());
                } else if (tremolo->chord1() != c || tremolo->chord2()) {
                    continue;
                }

                for (Segment* ls = s->next(st); ls; ls = ls->next(st)) {
                    if (EngravingItem* element = ls->element(i)) {
                        if (!element->isChord()) {
                            LOGD("cannot connect tremolo");
                            continue;
                        }
                        Chord* nc = toChord(element);
                        tremolo->setChords(c, nc);
                        nc->setTremolo(tremolo);
                        break;
                    }
                }

                if (!tremolo->chord2()) {
                    // this is an invalid tremolo! a continued tremolo was started on one note without a valid next note in that measure
                    // remove the tremolo entirely
                    c->setTremolo(nullptr);
                    score()->removeElement(tremolo);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   createVoice
//    Create a voice on demand by filling the measure
//    with a whole measure rest.
//    Check if there are any chord/rests in track; if
//    not create a whole measure rest
//---------------------------------------------------------

void Measure::createVoice(int track)
{
    for (Segment* s = first(); s; s = s->next()) {
        if (s->segmentType() != SegmentType::ChordRest) {
            continue;
        }
        if (s->element(track) == 0) {
            score()->setRest(s->tick(), track, ticks(), true, 0);
        }
        break;
    }
}

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(std::vector<staff_idx_t>& dst)
{
    std::vector<MStaff*> ms;
    for (staff_idx_t idx : dst) {
        ms.push_back(m_mstaves[idx]);
    }
    m_mstaves = ms;

    for (staff_idx_t staffIdx = 0; staffIdx < m_mstaves.size(); ++staffIdx) {
        m_mstaves[staffIdx]->lines()->setTrack(staffIdx * VOICES);
    }
    for (Segment& s : m_segments) {
        s.sortStaves(dst);
    }

    for (EngravingItem* e : el()) {
        if (e->track() == mu::nidx || e->isTopSystemObject()) {
            continue;
        }
        voice_idx_t voice    = e->voice();
        staff_idx_t staffIdx = e->staffIdx();
        staff_idx_t idx = mu::indexOf(dst, staffIdx);
        e->setTrack(idx * VOICES + voice);
    }
}

//---------------------------------------------------------
//   exchangeVoice
//---------------------------------------------------------

void Measure::exchangeVoice(track_idx_t strack, track_idx_t dtrack, staff_idx_t staffIdx)
{
    for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        s->swapElements(strack, dtrack);
    }

    auto spanners = score()->spannerMap().findOverlapping(tick().ticks(), endTick().ticks() - 1);
    Fraction start = tick();
    Fraction end = start + ticks();
    for (auto i = spanners.begin(); i < spanners.end(); i++) {
        Spanner* sp = i->value;
        Fraction spStart = sp->tick();
        Fraction spEnd = spStart + sp->ticks();
        LOGD("Start %d End %d Diff %d \n Measure Start %d End %d", spStart.ticks(), spEnd.ticks(), (spEnd - spStart).ticks(),
             start.ticks(), end.ticks());
        if (sp->isSlur() && (spStart >= start || spEnd < end)) {
            if (sp->track() == strack && spStart >= start) {
                sp->setTrack(dtrack);
            } else if (sp->track() == dtrack && spStart >= start) {
                sp->setTrack(strack);
            }
            if (sp->track2() == strack && spEnd < end) {
                sp->setTrack2(dtrack);
            } else if (sp->track2() == dtrack && spEnd < end) {
                sp->setTrack2(strack);
            }
        }
    }
    checkMultiVoices(staffIdx);     // probably true, but check for invisible notes & rests
}

//---------------------------------------------------------
//   checkMultiVoices
///   Check for more than on voice in this measure and staff and
///   set MStaff->hasVoices
//---------------------------------------------------------

void Measure::checkMultiVoices(staff_idx_t staffIdx)
{
    if (hasVoices(staffIdx, tick(), ticks())) {
        m_mstaves[staffIdx]->setHasVoices(true);
    } else {
        m_mstaves[staffIdx]->setHasVoices(false);
    }
}

//---------------------------------------------------------
//   hasVoices
//---------------------------------------------------------

bool Measure::hasVoices(staff_idx_t staffIdx, Fraction stick, Fraction len, bool considerInvisible) const
{
    Staff* st = score()->staff(staffIdx);
    if (st->isTabStaff(stick)) {
        // TODO: tab staves use different rules for stem direction etc
        // see for example https://musescore.org/en/node/308371
        // we should consider coming up with a more comprehensive solution
        // but for now, we are forcing measures on tab staves to be consider as a whole -
        // either they have voices or not
        // (rather than checking tick ranges)
        stick = tick();
        len = stretchedLen(st);
    }

    track_idx_t strack = staffIdx * VOICES + 1;
    track_idx_t etrack = staffIdx * VOICES + VOICES;
    Fraction etick = stick + len;

    for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->tick() >= etick) {
            break;
        }
        for (track_idx_t track = strack; track < etrack; ++track) {
            ChordRest* cr = toChordRest(s->element(track));
            if (cr) {
                if (cr->tick() + cr->actualTicks() <= stick) {
                    continue;
                }
                if (considerInvisible) {
                    return true;
                }
                bool v = false;
                if (cr->isChord()) {
                    Chord* c = toChord(cr);

                    // consider a chord visible if stem, hook(s) or beam(s) are visible
                    if ((c->stem() && c->stem()->visible())
                        || (c->hook() && c->hook()->visible())
                        || (c->beam() && c->beam()->visible())) {
                        v = true;
                    } else {
                        // or any of its notes
                        for (Note* n : c->notes()) {
                            if (n->visible()) {
                                v = true;
                                break;
                            }
                        }
                    }
                } else if (cr->isRest()) {
                    v = cr->visible() && !toRest(cr)->isGap();
                }
                if (v) {
                    return true;
                }
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasVoice
//---------------------------------------------------------

bool Measure::hasVoice(track_idx_t track) const
{
    if (track >= score()->ntracks()) {
        return false;
    }
    for (Segment* s = first(); s; s = s->next()) {
        if (s->segmentType() != SegmentType::ChordRest) {
            continue;
        }
        if (s->element(track)) {
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------
//   isEmpty
///   Check if the measure is filled by a full-measure rest, or is
///   full of rests on this staff, that may have fermatas on them.
///   If staff is -1, then check for all staves.
//-------------------------------------------------------------------

bool Measure::isEmpty(staff_idx_t staffIdx) const
{
    if (isMMRest()) {
        return true;
    }
    track_idx_t strack = 0;
    track_idx_t etrack = 0;
    if (staffIdx == mu::nidx) {
        strack = 0;
        etrack = score()->nstaves() * VOICES;
    } else {
        strack = staffIdx * VOICES;
        etrack = strack + VOICES;
    }
    for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* e = s->element(track);
            if (e && !e->isRest()) {
                return false;
            }
            // Check for cross-staff chords
            bool hasStaves = score()->staff(track / VOICES)->part()->staves().size() > 1;
            if (hasStaves) {
                if (strack >= VOICES) {
                    e = s->element(track - VOICES);
                    if (e && !e->isRest() && e->vStaffIdx() == staffIdx) {
                        return false;
                    }
                }
                if (etrack < score()->nstaves() * VOICES) {
                    e = s->element(track + VOICES);
                    if (e && !e->isRest() && e->vStaffIdx() == staffIdx) {
                        return false;
                    }
                }
            }
        }
        for (EngravingItem* a : s->annotations()) {
            if (a && staffIdx == mu::nidx) {
                return false;
            }
            if (!a || a->systemFlag() || !a->visible() || a->isFermata()) {
                continue;
            }
            size_t atrack = a->track();
            if (atrack >= strack && atrack < etrack) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//   isCutawayClef
///    Check for empty measure with only
///    a Courtesy Clef before End Bar Line
//---------------------------------------------------------

bool Measure::isCutawayClef(staff_idx_t staffIdx) const
{
    if (!score()->staff(staffIdx) || !m_mstaves[staffIdx]) {
        return false;
    }
    bool empty = (score()->staff(staffIdx)->cutaway() && isEmpty(staffIdx)) || !m_mstaves[staffIdx]->visible();
    if (!empty) {
        return false;
    }
    track_idx_t strack;
    track_idx_t etrack;
    if (staffIdx == mu::nidx) {
        strack = 0;
        etrack = score()->nstaves() * VOICES;
    } else {
        strack = staffIdx * VOICES;
        etrack = strack + VOICES;
    }
    // find segment before EndBarLine
    Segment* s = nullptr;
    for (Segment* ls = last(); ls; ls = ls->prev()) {
        if (ls->segmentType() == SegmentType::EndBarLine) {
            s = ls->prev();
            break;
        }
    }
    if (!s) {
        return false;
    }
    for (track_idx_t track = strack; track < etrack; ++track) {
        EngravingItem* e = s->element(track);
        if (!e || !e->isClef()) {
            continue;
        }
        if ((nextMeasure() && (nextMeasure()->system() == system())) || toClef(e)->showCourtesy()) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   isFullMeasureRest
//    Check for an empty measure, filled with full measure
//    rests.
//---------------------------------------------------------

bool Measure::isFullMeasureRest() const
{
    track_idx_t strack = 0;
    track_idx_t etrack = score()->nstaves() * VOICES;

    Segment* s = first(SegmentType::ChordRest);
    for (track_idx_t track = strack; track < etrack; ++track) {
        EngravingItem* e = s->element(track);
        if (e) {
            if (!e->isRest()) {
                return false;
            }
            Rest* rest = toRest(e);
            if (rest->durationType().type() != DurationType::V_MEASURE) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Measure::empty() const
{
    if (irregular()) {
        return false;
    }
    int n = 0;
    track_idx_t tracks = m_mstaves.size() * VOICES;
    static const SegmentType st = SegmentType::ChordRest;
    for (const Segment* s = first(st); s; s = s->next(st)) {
        bool restFound = false;
        for (track_idx_t track = 0; track < tracks; ++track) {
            if ((track % VOICES) == 0 && !score()->staff(track / VOICES)->show()) {
                track += VOICES - 1;
                continue;
            }
            if (s->element(track)) {
                if (!s->element(track)->isRest()) {
                    return false;
                }
                restFound = true;
            }
        }
        if (restFound) {
            ++n;
        }
        // measure is not empty if there is more than one rest
        if (n > 1) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   isOnlyRests
//---------------------------------------------------------

bool Measure::isOnlyRests(track_idx_t track) const
{
    static const SegmentType st = SegmentType::ChordRest;
    for (const Segment* s = first(st); s; s = s->next(st)) {
        if (s->segmentType() != st || !s->element(track)) {
            continue;
        }
        if (!s->element(track)->isRest()) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   isOnlyDeletedRests
//---------------------------------------------------------

bool Measure::isOnlyDeletedRests(track_idx_t track) const
{
    static const SegmentType st { SegmentType::ChordRest };
    for (const Segment* s = first(st); s; s = s->next(st)) {
        if (s->segmentType() != st || !s->element(track)) {
            continue;
        }
        if (s->element(track)->isRest() ? !toRest(s->element(track))->isGap() : !s->element(track)->isRest()) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   stretchedLen
//---------------------------------------------------------

Fraction Measure::stretchedLen(Staff* staff) const
{
    return ticks() * staff->timeStretch(tick());
}

//---------------------------------------------------------
//   cloneMeasure
//---------------------------------------------------------

Measure* Measure::cloneMeasure(Score* sc, const Fraction& tick, TieMap* tieMap)
{
    Measure* m      = new Measure(sc->dummy()->system());
    m->m_timesig    = m_timesig;
    m->_len         = _len;
    m->m_repeatCount = m_repeatCount;

    assert(sc->staves().size() >= m_mstaves.size());   // destination score we're cloning into must have at least as many staves as measure being cloned

    m->setNo(no());
    m->setNoOffset(noOffset());
    m->setIrregular(irregular());
    m->m_userStretch           = m_userStretch;
    m->m_breakMultiMeasureRest = m_breakMultiMeasureRest;
    m->m_playbackCount         = m_playbackCount;

    m->setTick(tick);
    m->setLineBreak(lineBreak());
    m->setPageBreak(pageBreak());
    m->setSectionBreak(sectionBreak());

    m->setHeader(header());
    m->setTrailer(trailer());

    size_t tracks = sc->nstaves() * VOICES;
    TupletMap tupletMap;

    for (Segment* oseg = first(); oseg; oseg = oseg->next()) {
        Segment* s = Factory::createSegment(m, oseg->segmentType(), oseg->rtick());
        s->setEnabled(oseg->enabled());
        s->setVisible(oseg->visible());
        s->setHeader(oseg->header());
        s->setTrailer(oseg->trailer());

        m->m_segments.push_back(s);
        for (track_idx_t track = 0; track < tracks; ++track) {
            EngravingItem* oe = oseg->element(track);
            for (EngravingItem* e : oseg->annotations()) {
                if (e->generated() || e->track() != track) {
                    continue;
                }
                EngravingItem* ne = e->clone();
                ne->setTrack(track);
                ne->setOffset(e->offset());
                ne->setScore(sc);
                s->add(ne);
            }
            if (!oe) {
                continue;
            }
            EngravingItem* ne = oe->clone();
            if (oe->isChordRest()) {
                ChordRest* ocr = toChordRest(oe);
                ChordRest* ncr = toChordRest(ne);
                Tuplet* ot     = ocr->tuplet();
                if (ot) {
                    Tuplet* nt = tupletMap.findNew(ot);
                    if (nt == 0) {
                        nt = Factory::copyTuplet(*ot);
                        nt->clear();
                        nt->setTrack(track);
                        nt->setScore(sc);
                        nt->setParent(m);
                        nt->setTick(m->tick() + ot->rtick());
                        tupletMap.add(ot, nt);
                    }
                    ncr->setTuplet(nt);
                    nt->add(ncr);
                }
                if (oe->isChord()) {
                    Chord* och = toChord(ocr);
                    Chord* nch = toChord(ncr);
                    size_t n = och->notes().size();
                    for (size_t i = 0; i < n; ++i) {
                        Note* on = och->notes().at(i);
                        Note* nn = nch->notes().at(i);
                        if (on->tieFor()) {
                            Tie* tie = on->tieFor()->clone();
                            tie->setScore(sc);
                            nn->setTieFor(tie);
                            tie->setStartNote(nn);
                            tieMap->add(on->tieFor(), tie);
                        }
                        if (on->tieBack()) {
                            Tie* tie = tieMap->findNew(on->tieBack());
                            if (tie) {
                                nn->setTieBack(tie);
                                tie->setEndNote(nn);
                            } else {
                                LOGD("cloneMeasure: cannot find tie, track %zu", track);
                            }
                        }
                    }
                }
            }
            ne->setOffset(oe->offset());
            ne->setScore(sc);
            s->add(ne);
        }
    }
    for (EngravingItem* e : el()) {
        EngravingItem* ne = e->clone();
        ne->setScore(sc);
        ne->setOffset(e->offset());
        m->add(ne);
    }
    return m;
}

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction Measure::snap(const Fraction& tick, const PointF p) const
{
    Segment* s = first();
    for (; s->next(); s = s->next()) {
        double x  = s->x();
        double dx = s->next()->x() - x;
        if (s->tick() == tick) {
            x += dx / 3.0 * 2.0;
        } else if (s->next()->tick() == tick) {
            x += dx / 3.0;
        } else {
            x += dx * .5;
        }
        if (p.x() < x) {
            break;
        }
    }
    return s->tick();
}

//---------------------------------------------------------
//   snapNote
//---------------------------------------------------------

Fraction Measure::snapNote(const Fraction& /*tick*/, const PointF p, int staff) const
{
    Segment* s = first();
    for (;;) {
        Segment* ns = s->next();
        while (ns && ns->element(staff) == 0) {
            ns = ns->next();
        }
        if (ns == 0) {
            break;
        }
        double x  = s->x();
        double nx = x + (ns->x() - x) * .5;
        if (p.x() < nx) {
            break;
        }
        s = ns;
    }
    return s->tick();
}

//---------------------------------------------------------
//   searchSegment
///   Finds a segment which x position is most close to the
///   given \p x.
///   \param x The x coordinate in measure coordinates.
///   \param st Type of segments to search.
///   \param strack start of track range (strack included)
///   in which the found segment should contain elements.
///   \param etrack end of track range (etrack excluded)
///   in which the found segment should contain elements.
///   \param preferredSegment If not nullptr, will give
///   more space to the given segment when searching it by
///   coordinate.
///   \returns The segment that was found.
//---------------------------------------------------------

Segment* Measure::searchSegment(double x, SegmentType st, track_idx_t strack, track_idx_t etrack, const Segment* preferredSegment,
                                double spacingFactor) const
{
    const track_idx_t lastTrack = etrack - 1;
    for (Segment* segment = first(st); segment; segment = segment->next(st)) {
        if (!segment->hasElements(strack, lastTrack)) {
            continue;
        }
        Segment* ns = segment->next(st);
        for (; ns; ns = ns->next(st)) {
            if (ns->hasElements(strack, lastTrack)) {
                break;
            }
        }
        if (!ns) {
            return segment;
        }
        if (preferredSegment == segment) {
            if (x < (segment->x() + (ns->x() - segment->x()))) {
                return segment;
            }
        } else if (preferredSegment == ns) {
            if (x <= segment->x()) {
                return segment;
            }
        } else {
            if (x < (segment->x() + (ns->x() - segment->x()) * spacingFactor)) {
                return segment;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Measure::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TIMESIG_NOMINAL:
        return PropertyValue::fromValue(m_timesig);
    case Pid::TIMESIG_ACTUAL:
        return PropertyValue::fromValue(_len);
    case Pid::MEASURE_NUMBER_MODE:
        return int(measureNumberMode());
    case Pid::BREAK_MMR:
        return breakMultiMeasureRest();
    case Pid::REPEAT_COUNT:
        return repeatCount();
    case Pid::USER_STRETCH:
        return userStretch();
    default:
        return MeasureBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Measure::setProperty(Pid propertyId, const PropertyValue& value)
{
    switch (propertyId) {
    case Pid::TIMESIG_NOMINAL:
        m_timesig = value.value<Fraction>();
        break;
    case Pid::TIMESIG_ACTUAL:
        _len = value.value<Fraction>();
        break;
    case Pid::MEASURE_NUMBER_MODE:
        setMeasureNumberMode(MeasureNumberMode(value.toInt()));
        break;
    case Pid::BREAK_MMR:
        setBreakMultiMeasureRest(value.toBool());
        break;
    case Pid::REPEAT_COUNT:
        setRepeatCount(value.toInt());
        break;
    case Pid::USER_STRETCH:
        setUserStretch(value.toDouble());
        break;
    default:
        return MeasureBase::setProperty(propertyId, value);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Measure::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TIMESIG_NOMINAL:
    case Pid::TIMESIG_ACTUAL:
        return PropertyValue();
    case Pid::MEASURE_NUMBER_MODE:
        return int(MeasureNumberMode::AUTO);
    case Pid::BREAK_MMR:
        return false;
    case Pid::REPEAT_COUNT:
        return 2;
    case Pid::USER_STRETCH:
        return 1.0;
    case Pid::NO_OFFSET:
        return 0;
    case Pid::IRREGULAR:
        return false;
    default:
        break;
    }
    return MeasureBase::propertyDefault(propertyId);
}

//-------------------------------------------------------------------
//   mmRestFirst
//    this is a multi measure rest
//    returns first measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestFirst() const
{
    assert(isMMRest());
    if (prev()) {
        return toMeasure(prev()->next());
    }
    return score()->firstMeasure();
}

//-------------------------------------------------------------------
//   mmRestLast
//    this is a multi measure rest
//    returns last measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestLast() const
{
    assert(isMMRest());
    if (next()) {
        return toMeasure(next()->prev());
    }
    return score()->lastMeasure();
}

//---------------------------------------------------------
//   mmRest1
//    return the multi measure rest this measure is covered
//    by
//---------------------------------------------------------

const Measure* Measure::mmRest1() const
{
    if (m_mmRest) {
        return m_mmRest;
    }
    if (m_mmRestCount != -1) {
        // return const_cast<Measure*>(this);
        return this;
    }
    const Measure* m = this;
    while (m && !m->m_mmRest) {
        m = m->prevMeasure();
    }
    if (m) {
        return const_cast<Measure*>(m->m_mmRest);
    }
    return 0;
}

int Measure::measureRepeatCount(staff_idx_t staffIdx) const
{
    if (staffIdx >= m_mstaves.size()) {
        return 0;
    }

    return m_mstaves[staffIdx]->measureRepeatCount();
}

bool Measure::containsMeasureRepeat(const staff_idx_t staffIdxFrom, const staff_idx_t staffIdxTo) const
{
    for (staff_idx_t idx = staffIdxFrom; idx <= staffIdxTo; ++idx) {
        if (measureRepeatCount(idx) > 0) {
            return true;
        }
    }

    return false;
}

void Measure::setMeasureRepeatCount(int n, staff_idx_t staffIdx)
{
    if (staffIdx >= m_mstaves.size()) {
        return;
    }

    m_mstaves[staffIdx]->setMeasureRepeatCount(n);
}

bool Measure::isMeasureRepeatGroup(staff_idx_t staffIdx) const
{
    return measureRepeatCount(staffIdx) > 0;
}

//---------------------------------------------------------
//   isMeasureRepeatGroupWithNextM
//    true if this and next measure are part of same MeasureRepeat group
//---------------------------------------------------------

bool Measure::isMeasureRepeatGroupWithNextM(staff_idx_t staffIdx) const
{
    if (!isMeasureRepeatGroup(staffIdx) || !nextMeasure() || !nextMeasure()->isMeasureRepeatGroup(staffIdx)) {
        return false;
    }
    if (measureRepeatCount(staffIdx) == nextMeasure()->measureRepeatCount(staffIdx) - 1) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   isMeasureRepeatGroupWithPrevM
//    true if this and prev measure are part of same MeasureRepeat group
//---------------------------------------------------------

bool Measure::isMeasureRepeatGroupWithPrevM(staff_idx_t staffIdx) const
{
    return measureRepeatCount(staffIdx) > 1;
}

//---------------------------------------------------------
//   firstMeasureOfGroup
//    for measures within group containing MeasureRepeat,
//    return the measure (possibly this) at start of group
//---------------------------------------------------------

Measure* Measure::firstOfMeasureRepeatGroup(staff_idx_t staffIdx) const
{
    if (!isMeasureRepeatGroup(staffIdx)) {
        return nullptr;
    }
    Measure* m = const_cast<Measure*>(this);
    for (int i = 1; i < measureRepeatCount(staffIdx); ++i) {
        m = m->prevMeasure();
    }
    return m;
}

//---------------------------------------------------------
//   measureRepeatElement
//    access MeasureRepeat element from anywhere in related group
//---------------------------------------------------------

MeasureRepeat* Measure::measureRepeatElement(staff_idx_t staffIdx) const
{
    Measure* m = firstOfMeasureRepeatGroup(staffIdx);
    if (!m) {
        return nullptr;
    }
    while (m && m->isMeasureRepeatGroup(staffIdx)) {
        track_idx_t strack = staff2track(staffIdx);
        track_idx_t etrack = staff2track(staffIdx + 1);
        for (track_idx_t track = strack; track < etrack; ++track) {
            // should only be in first track, but just in case
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                // should only be in first segment, but just in case
                EngravingItem* e = s->element(track);
                if (e && e->isMeasureRepeat()) {
                    return toMeasureRepeat(e);
                }
            }
        }
        m = m->nextMeasure();
    }
    return nullptr;
}

//---------------------------------------------------------
//   measureRepeatNumMeasures
//---------------------------------------------------------

int Measure::measureRepeatNumMeasures(staff_idx_t staffIdx) const
{
    MeasureRepeat* el = measureRepeatElement(staffIdx);
    if (!el) {
        return 0;
    }
    return el->numMeasures();
}

//---------------------------------------------------------
//   isOneMeasureRepeat
//---------------------------------------------------------

bool Measure::isOneMeasureRepeat(staff_idx_t staffIdx) const
{
    return measureRepeatNumMeasures(staffIdx) == 1;
}

//---------------------------------------------------------
//   nextIsOneMeasureRepeat
//---------------------------------------------------------

bool Measure::nextIsOneMeasureRepeat(staff_idx_t staffIdx) const
{
    if (!nextMeasure()) {
        return false;
    }
    return nextMeasure()->isOneMeasureRepeat(staffIdx);
}

//---------------------------------------------------------
//   prevIsOneMeasureRepeat
//---------------------------------------------------------

bool Measure::prevIsOneMeasureRepeat(staff_idx_t staffIdx) const
{
    if (!prevMeasure()) {
        return false;
    }
    return prevMeasure()->isOneMeasureRepeat(staffIdx);
}

//-------------------------------------------------------------------
//   userStretch
//-------------------------------------------------------------------

double Measure::userStretch() const
{
    return score()->layoutOptions().isMode(LayoutMode::FLOAT) ? 1.0 : m_userStretch;
}

//---------------------------------------------------------
//   nextElementStaff
//---------------------------------------------------------

EngravingItem* Measure::nextElementStaff(staff_idx_t staff)
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }

    // handle measure elements
    if (e->explicitParent() == this) {
        auto i = std::find(el().begin(), el().end(), e);
        if (i != el().end()) {
            if (++i != el().end()) {
                EngravingItem* resElement = *i;
                if (resElement) {
                    return resElement;
                }
            }
        }
    }

    for (; e && e->type() != ElementType::SEGMENT; e = e->parentItem()) {
    }
    Segment* seg = toSegment(e);
    Segment* nextSegment = seg ? seg->next() : first();
    EngravingItem* next = seg->firstElementOfSegment(nextSegment, staff);
    if (next) {
        return next;
    }

    return score()->lastElement();
}

//---------------------------------------------------------
//   prevElementStaff
//---------------------------------------------------------

EngravingItem* Measure::prevElementStaff(staff_idx_t staff)
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }

    // handle measure elements
    if (e->explicitParent() == this) {
        auto i = std::find(el().rbegin(), el().rend(), e);
        if (i != el().rend()) {
            if (++i != el().rend()) {
                EngravingItem* resElement = *i;
                if (resElement) {
                    return resElement;
                }
            }
        }
    }

    Measure* prevM = prevMeasureMM();
    if (prevM) {
        Segment* seg = prevM->last();
        if (seg) {
            return seg->lastElement(staff);
        }
    }
    return score()->firstElement();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Measure::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), String::number(no() + 1));
}

//---------------------------------------------------
//    computeTicks
//    set ticks for all segments
//       return minTick
//---------------------------------------------------

Fraction Measure::computeTicks()
{
    Fraction minTick = ticks();
    if (minTick <= Fraction(0, 1)) {
        LOGD("=====minTick %d measure %p", minTick.ticks(), this);
    }
    assert(minTick > Fraction(0, 1));

    Segment* ns = first();
    while (ns && !ns->enabled()) {
        ns = ns->next();
    }
    while (ns) {
        Segment* s = ns;
        ns         = s->nextActive();
        Fraction nticks = (ns ? ns->rtick() : ticks()) - s->rtick();
        if (nticks.isNotZero()) {
            if (nticks < minTick) {
                minTick = nticks;
            }
        }
        s->setTicks(nticks);
    }
    return minTick;
}

//---------------------------------------------------------
//   endBarLine
//      return the first one
//---------------------------------------------------------

const BarLine* Measure::endBarLine() const
{
    // search barline segment:
    Segment* s = last();
    while (s && !s->isEndBarLineType()) {
        s = s->prev();
    }
    // search first element
    if (s) {
        for (const EngravingItem* e : s->elist()) {
            if (e) {
                return toBarLine(e);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   endBarLineType
//    Assume all barlines have same type if there is more
//    than one.
//---------------------------------------------------------

BarLineType Measure::endBarLineType() const
{
    const BarLine* bl = endBarLine();
    return bl ? bl->barLineType() : BarLineType::NORMAL;
}

//---------------------------------------------------------
//   endBarLineVisible
//    Assume all barlines have same visibility if there is more
//    than one.
//---------------------------------------------------------

bool Measure::endBarLineVisible() const
{
    const BarLine* bl = endBarLine();
    return bl ? bl->visible() : true;
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Measure::triggerLayout() const
{
    if (prev() || next()) { // avoid triggering layout before getting added to a score
        score()->setLayout(tick(), endTick(), 0, score()->nstaves() - 1, this);
    }
}

//---------------------------------------------------------
//   setEndBarLineType
//     Create a *generated* barline with the given type and
//     properties if none exists. Modify if it exists.
//     Useful for import filters.
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, track_idx_t track, bool visible, mu::draw::Color color)
{
    Segment* seg = undoGetSegment(SegmentType::EndBarLine, endTick());
    // get existing bar line for this staff, if any
    BarLine* bl = toBarLine(seg->element(track));
    if (!bl) {
        // no suitable bar line: create a new one
        bl = Factory::createBarLine(seg);
        bl->setParent(seg);
        bl->setTrack(track);
        Part* part = score()->staff(track / VOICES)->part();
        // by default, barlines for multi-staff parts should span across staves
        if (part && part->nstaves() > 1) {
            bl->setSpanStaff(true);
        }
        score()->addElement(bl);
    }
    bl->setGenerated(false);
    bl->setBarLineType(val);
    bl->setVisible(visible);
    bl->setColor(color.isValid() ? color : curColor());
}

//---------------------------------------------------------
//   basicStretch
//---------------------------------------------------------

double Measure::basicStretch() const
{
    double stretch = userStretch() * score()->styleD(Sid::measureSpacing);
    if (stretch < 1.0) {
        stretch = 1.0;
    }
    return stretch;
}

//---------------------------------------------------------
//   basicWidth
//---------------------------------------------------------

double Measure::basicWidth() const
{
    Segment* ls = last();
    double w = (ls->x() + ls->width()) * basicStretch();
    double minMeasureWidth = score()->styleMM(Sid::minMeasureWidth);
    if (w < minMeasureWidth) {
        w = minMeasureWidth;
    }
    return w;
}

//---------------------------------------------------------
//   checkHeader
//---------------------------------------------------------

void Measure::checkHeader()
{
    for (Segment* seg = first(); seg; seg = seg->next()) {
        if (seg->enabled() && seg->header()) {
            setHeader(seg->header());
            break;
        }
    }
}

//---------------------------------------------------------
//   checkTrailer
//---------------------------------------------------------

void Measure::checkTrailer()
{
    for (Segment* seg = last(); seg != first(); seg = seg->prev()) {
        if (seg->enabled() && seg->trailer()) {
            setTrailer(seg->trailer());
            break;
        }
    }
}

void Measure::spaceRightAlignedSegments()
{
    // Collect all the right-aligned segments starting from the back
    std::vector<Segment*> rightAlignedSegments;
    for (Segment* segment = m_segments.last(); segment; segment = segment->prev()) {
        if (segment->enabled() && segment->isRightAligned()) {
            rightAlignedSegments.push_back(segment);
        }
    }
    // Compute spacing
    static constexpr double arbitraryLowReal = -10000.0;
    for (Segment* raSegment : rightAlignedSegments) {
        // 1) right-align the segment against the following ones
        double minDistAfter = arbitraryLowReal;
        for (Segment* seg = raSegment->next(); seg; seg = seg->next()) {
            double xDiff = seg->x() - raSegment->x();
            double minDist = raSegment->minHorizontalCollidingDistance(seg);
            minDistAfter = std::max(minDistAfter, minDist - xDiff);
        }
        if (minDistAfter != arbitraryLowReal && raSegment->prevActive()) {
            Segment* prevSegment = raSegment->prev();
            prevSegment->setWidth(prevSegment->width() - minDistAfter);
            prevSegment->setWidthOffset(prevSegment->widthOffset() - minDistAfter);
            raSegment->movePosX(-minDistAfter);
            raSegment->setWidth(raSegment->width() + minDistAfter);
        }
        // 2) Make sure the segment isn't colliding with anything behind
        double minDistBefore = 0.0;
        for (Segment* seg = raSegment->prevActive(); seg; seg = seg->prevActive()) {
            double xDiff = raSegment->x() - seg->x();
            double minDist = seg->minHorizontalCollidingDistance(raSegment);
            minDistBefore = std::max(minDistBefore, minDist - xDiff);
        }
        Segment* prevSegment = raSegment->prevActive();
        if (prevSegment) {
            prevSegment->setWidth(prevSegment->width() + minDistBefore);
        }
        for (Segment* seg = raSegment; seg; seg = seg->next()) {
            seg->movePosX(minDistBefore);
        }
        setWidth(width() + minDistBefore);
    }
}

void Measure::stretchToTargetWidth(double targetWidth)
{
    if (targetWidth < width()) {
        return;
    }
    std::vector<Spring> springs;
    for (Segment& s : m_segments) {
        if (s.isChordRestType() && s.visible() && s.enabled() && !s.allElementsInvisible()) {
            double springConst = 1 / s.stretch();
            double width = s.width() - s.widthOffset();
            double preTension = width * springConst;
            springs.push_back(Spring(springConst, width, preTension, &s));
        }
    }
    Segment::stretchSegmentsToWidth(springs, targetWidth - width());
    respaceSegments();
}

void Measure::layoutSegmentsInPracticeMode(const std::vector<int>& visibleParts)
{
    layoutSegmentsWithDuration(visibleParts);
}

double Measure::computeFirstSegmentXPosition(Segment* segment)
{
    double x = 0;

    Shape ls(RectF(0.0, 0.0, 0.0, spatium() * 4));

    // First, try to compute first segment x-position by padding against end barline of previous measure
    Measure* prevMeas = (prev() && prev()->isMeasure() && prev()->system() == system()) ? toMeasure(prev()) : nullptr;
    Segment* prevMeasEnd = prevMeas ? prevMeas->lastEnabled() : nullptr;
    bool ignorePrev = !prevMeas || prevMeas->system() != system() || !prevMeasEnd
                      || (prevMeasEnd->segmentType() & SegmentType::BarLineType && segment->segmentType() & SegmentType::BarLineType);
    if (!ignorePrev) {
        x = prevMeasEnd->minHorizontalCollidingDistance(segment);
        x -= prevMeas->width() - prevMeasEnd->x();
    }

    // If that doesn't succeed (e.g. first bar) then just use left-margins
    if (x <= 0) {
        x = segment->minLeft(ls);
        if (segment->isChordRestType()) {
            x += score()->styleMM(segment->hasAccidentals() ? Sid::barAccidentalDistance : Sid::barNoteDistance);
        } else if (segment->isClefType() || segment->isHeaderClefType()) {
            x += score()->styleMM(Sid::clefLeftMargin);
        } else if (segment->isKeySigType()) {
            x = std::max(x, score()->styleMM(Sid::keysigLeftMargin).val());
        } else if (segment->isTimeSigType()) {
            x = std::max(x, score()->styleMM(Sid::timesigLeftMargin).val());
        }
    }

    // Special case: the start-repeat should overlap the end-repeat of the previous measure
    bool prevIsEndRepeat = prevMeas && prevMeas->repeatEnd() && prevMeasEnd && prevMeasEnd->isEndBarLineType();
    if (prevIsEndRepeat && segment->isStartRepeatBarLineType() && (prevMeas->system() == system())) {
        x -= score()->styleMM(Sid::endBarWidth);
    }

    // Do a final check of chord distances (invisible items may in some cases elude the 2 previous steps)
    if (segment->isChordRestType()) {
        double barNoteDist = score()->styleMM(Sid::barNoteDistance).val();
        for (EngravingItem* e : segment->elist()) {
            if (!e || !e->isChordRest() || (e->staff() && e->staff()->isTabStaff(e->tick()))) {
                continue;
            }
            x = std::max(x, barNoteDist * e->mag() - e->pos().x());
        }
    }
    x += segment->extraLeadingSpace().val() * spatium();
    return x;
}

static Segment* findFirstEnabledSegment(Measure* measure)
{
    Segment* current = measure->first();
    while (current && !current->enabled()) {
        current = current->next();
    }

    return current;
}

void Measure::layoutSegmentsWithDuration(const std::vector<int>& visibleParts)
{
    calculateQuantumCell(visibleParts);

    double currentXPos = 0;

    Segment* current = findFirstEnabledSegment(this);

    auto [spacing, width] = current->computeCellWidth(visibleParts);
    currentXPos = computeFirstSegmentXPosition(current);
    current->setPosX(currentXPos);
    current->setWidth(width);
    current->setSpacing(spacing);
    currentXPos += width;

    current = current->next();
    while (current) {
//        if (!current->enabled() || !current->visible()) {
//            current = current->next();
//            continue;
//        }

        auto [spacing, width] = current->computeCellWidth(visibleParts);
        current->setWidth(width + spacing);
        current->setSpacing(spacing);
        currentXPos += spacing;
        current->setPosX(currentXPos);
        currentXPos += width;
        current = current->next();
    }

    setWidth(currentXPos);
}

void Measure::calculateQuantumCell(const std::vector<int>& visibleParts)
{
    for (const Segment& s : m_segments) {
        ChordRest* cr = Segment::ChordRestWithMinDuration(&s, visibleParts);

        if (cr && cr->actualTicks() < m_quantumOfSegmentCell) {
            m_quantumOfSegmentCell = cr->actualTicks();
        }
    }
}

Fraction Measure::quantumOfSegmentCell() const
{
    return m_quantumOfSegmentCell;
}

Fraction Measure::maxTicks() const
{
    Segment* s = first();
    Fraction maxticks = Fraction(0, 1);
    if (isMMRest()) {
        return timesig();
    }
    while (s) {
        if (s->enabled() && s->isChordRestType()) {
            maxticks = std::max(maxticks, s->ticks());
        }
        s = s->next();
    }
    return maxticks;
}

Fraction Measure::shortestChordRest() const
{
    Fraction shortest = Fraction::max(); // Initializing at arbitrary high value
    Fraction cur = Fraction::max();
    Segment* s = first();
    while (s) {
        if (s->isChordRestType() && !s->allElementsInvisible()) {
            cur = s->shortestChordRest();
            if (cur < shortest) {
                shortest = cur;
            }
        }
        s = s->next();
    }
    return shortest;
}

void Measure::respaceSegments()
{
    double x = 0.0;
    // Find starting x position (i.e. position of first relevant segment)
    for (Segment& s : m_segments) {
        if (s.enabled() && s.visible() && !s.allElementsInvisible()) {
            x = s.x();
            break;
        }
    }
    // Start respacing segments
    for (Segment& s : m_segments) {
        s.setPosX(x);
        if (s.enabled() && s.visible() && !s.allElementsInvisible()) {
            x += s.width();
        }
    }
    // Update measure width
    setWidth(x);
}
}
