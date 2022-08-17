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

#include "accidental.h"
#include "ambitus.h"
#include "arpeggio.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "bracket.h"
#include "bsymbol.h"
#include "chord.h"
#include "duration.h"
#include "fret.h"
#include "glissando.h"
#include "hook.h"
#include "instrumentname.h"
#include "ledgerline.h"
#include "lyrics.h"
#include "measure.h"
#include "measurenumber.h"
#include "mmrestrange.h"
#include "note.h"
#include "page.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "spacer.h"
#include "spanner.h"
#include "staff.h"
#include "stafflines.h"
#include "stem.h"
#include "stemslash.h"
#include "system.h"
#include "systemdivider.h"
#include "textframe.h"
#include "tie.h"
#include "tremolo.h"
#include "trill.h"
#include "tuplet.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   Score
//---------------------------------------------------------

EngravingObject* Score::scanParent() const
{
    return nullptr;  // Score is root node
}

EngravingObjectList Score::scanChildren() const
{
    EngravingObjectList children;

    for (Page* page : pages()) {
        children.push_back(page);
    }

    return children;
}

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

EngravingObject* Page::scanParent() const
{
    return score();
}

EngravingObjectList Page::scanChildren() const
{
    EngravingObjectList children;

    for (System* system : systems()) {
        children.push_back(system);
    }

    return children;
}

//---------------------------------------------------------
//   System
//---------------------------------------------------------

EngravingObject* System::scanParent() const
{
    return page();
}

EngravingObjectList System::scanChildren() const
{
    EngravingObjectList children;

    for (Bracket* bracket : brackets()) {
        children.push_back(bracket);
    }

    if (auto dividerLeft = systemDividerLeft()) {
        children.push_back(dividerLeft);
    }

    if (auto dividerRight = systemDividerRight()) {
        children.push_back(dividerRight);
    }

    for (SysStaff* staff : _staves) {
        for (InstrumentName* instrName : staff->instrumentNames) {
            children.push_back(instrName);
        }
    }

    for (MeasureBase* measure : measures()) {
        children.push_back(measure);
    }

    return children;
}

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

EngravingObject* MeasureBase::scanParent() const
{
    return system();
}

EngravingObjectList MeasureBase::scanChildren() const
{
    EngravingObjectList children;

    for (EngravingItem* element : el()) {
        children.push_back(element);
    }

    return children;
}

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

EngravingObject* Measure::scanParent() const
{
    // A MMRest measure will contain Measures that it has replaced
    // System > MMR > Measure
    if (isMMRest()) {  // this is MMR
        return system();
    } else if (m_mmRestCount < 0) {  // this is part of MMR
        return const_cast<Measure*>(mmRest1());
    }
    // for a normal measure
    return system();
}

EngravingObjectList Measure::scanChildren() const
{
    EngravingObjectList children;

    for (EngravingItem* element : el()) {
        children.push_back(element);
    }

    if (isMMRest()) {
        Measure* m1 = mmRestFirst();
        Measure* m2 = mmRestLast();
        while (m1 != m2) {
            children.push_back(m1);
            m1 = m1->nextMeasure();
        }

        return children;
    }

    Segment* seg = m_segments.first();
    while (seg) {
        children.push_back(seg);
        seg = seg->next();
    }

    size_t nstaves = score()->nstaves();
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (auto _staffLines = m_mstaves[staffIdx]->lines()) {
            children.push_back(_staffLines);
        }

        if (auto _vspacerUp = vspacerUp(staffIdx)) {
            children.push_back(_vspacerUp);
        }

        if (auto _vspacerDown = vspacerDown(staffIdx)) {
            children.push_back(_vspacerDown);
        }

        if (auto _noText = noText(staffIdx)) {
            children.push_back(_noText);
        }

        if (auto _mmRangeText = mmRangeText(staffIdx)) {
            children.push_back(_mmRangeText);
        }
    }

    const std::multimap<int, Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* s = i->second;
        if (s->anchor() == Spanner::Anchor::MEASURE) {
            children.push_back(s);
        }
    }

    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            children.push_back(s);
        }
    }

    for (EngravingObject* obj : MeasureBase::scanChildren()) {
        children.push_back(obj);
    }

    return children;
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

EngravingObject* Segment::scanParent() const
{
    return measure();
}

EngravingObjectList Segment::scanChildren() const
{
    EngravingObjectList children;

    for (EngravingItem* element : _elist) {
        if (element) {
            children.push_back(element);
        }
    }

    for (EngravingItem* annotation : _annotations) {
        children.push_back(annotation);
    }

    if (segmentType() == SegmentType::ChordRest) {
        const std::multimap<int, Spanner*>& spannerMap = score()->spanner();
        int start_tick = tick().ticks();
        for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
            Spanner* s = i->second;
            if (s->anchor() == Spanner::Anchor::SEGMENT) {
                children.push_back(s);
            }
        }
        const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
        for (Spanner* s : unmanagedSpanners) {
            if (s->scanParent() == this) {
                children.push_back(s);
            }
        }
    }

    return children;
}

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

EngravingObject* ChordRest::scanParent() const
{
    if (isGrace()) {
        // grace notes do not have a segment of their own
        // their parent is the chord they are attached to
        return explicitParent();
    }
    return segment();
}

EngravingObjectList ChordRest::scanChildren() const
{
    EngravingObjectList children;

    Beam* _b = beam();
    if (_b && _b->scanParent() == this) {
        children.push_back(_b);
    }

    for (Lyrics* lyrics : _lyrics) {
        children.push_back(lyrics);
    }

    const DurationElement* de = this;
    while (de->tuplet() && de->tuplet()->elements().front() == de) {
        children.push_back(de->tuplet());
        de = de->tuplet();
    }

    if (auto tabDuration = _tabDur) {
        children.push_back(tabDuration);
    }

    for (EngravingItem* element : _el) {
        children.push_back(element);
    }

    const std::multimap<int, Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* s = i->second;
        if (s->anchor() == Spanner::Anchor::CHORD && s->scanParent() == this) {
            children.push_back(s);
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            children.push_back(s);
        }
    }

    return children;
}

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

EngravingObject* Chord::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObjectList Chord::scanChildren() const
{
    EngravingObjectList children;

    for (Note* note : notes()) {
        children.push_back(note);
    }

    if (_arpeggio) {
        children.push_back(_arpeggio);
    }

    if (_tremolo && _tremolo->chord1() == this) {
        children.push_back(_tremolo);
    }

    for (Chord* chord : graceNotes()) {
        children.push_back(chord);
    }

    for (Articulation* art : articulations()) {
        children.push_back(art);
    }

    if (_stem) {
        children.push_back(_stem);
    }

    if (_hook) {
        children.push_back(_hook);
    }

    if (_stemSlash) {
        children.push_back(_stemSlash);
    }

    LedgerLine* ledgerLines = _ledgerLines;
    while (ledgerLines) {
        children.push_back(ledgerLines);
        ledgerLines = ledgerLines->next();
    }

    for (EngravingObject* child : ChordRest::scanChildren()) {
        children.push_back(child);
    }

    return children;
}

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

EngravingObject* Rest::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObjectList Rest::scanChildren() const
{
    EngravingObjectList children;

    for (NoteDot* noteDot : m_dots) {
        children.push_back(noteDot);
    }

    for (EngravingObject* child : ChordRest::scanChildren()) {
        children.push_back(child);
    }

    return children;
}

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

EngravingObject* Note::scanParent() const
{
    return chord();
}

EngravingObjectList Note::scanChildren() const
{
    EngravingObjectList children;

    if (_accidental) {
        children.push_back(_accidental);
    }

    for (NoteDot* noteDot : _dots) {
        children.push_back(noteDot);
    }

    if (_tieFor) {
        children.push_back(_tieFor);
    }

    for (EngravingItem* element : el()) {
        children.push_back(element);
    }

    for (Spanner* spanner : spannerFor()) {
        children.push_back(spanner);
    }

    return children;
}

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

EngravingObject* Accidental::scanParent() const
{
    if (explicitParent() && explicitParent()->isTrillSegment()) {
        return explicitParent()->scanParent();
    }
    return note();
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

EngravingObject* Beam::scanParent() const
{
    return _elements[0];
}

//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

EngravingObject* Ambitus::scanParent() const
{
    return segment();
}

EngravingObjectList Ambitus::scanChildren() const
{
    EngravingObjectList children;

    Accidental* topAccid = const_cast<Accidental*>(_topAccid);
    if (topAccid && topAccid->accidentalType() != AccidentalType::NONE) {
        children.push_back(topAccid);
    }

    Accidental* bottomAccid = const_cast<Accidental*>(_bottomAccid);
    if (bottomAccid && bottomAccid->accidentalType() != AccidentalType::NONE) {
        children.push_back(bottomAccid);
    }

    return children;
}

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

EngravingObject* FretDiagram::scanParent() const
{
    return segment();
}

EngravingObjectList FretDiagram::scanChildren() const
{
    EngravingObjectList children;

    if (_harmony) {
        children.push_back(_harmony);
    }

    return children;
}

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

EngravingObject* Spanner::scanParent() const
{
    switch (anchor()) {
    case Anchor::SEGMENT:
        return startSegment();
    case Anchor::MEASURE:
        return startMeasure();
    case Anchor::CHORD:
        return findStartCR();
    case Anchor::NOTE:
        return startElement();
    default:
        return nullptr;
    }
}

EngravingObjectList Spanner::scanChildren() const
{
    EngravingObjectList children;

    for (SpannerSegment* segment : spannerSegments()) {
        children.push_back(segment);
    }

    return children;
}

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

EngravingObject* SpannerSegment::scanParent() const
{
    return spanner();
}

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

EngravingObject* BSymbol::scanParent() const
{
    return segment();
}

EngravingObjectList BSymbol::scanChildren() const
{
    EngravingObjectList children;

    for (EngravingItem* leaf : _leafs) {
        children.push_back(leaf);
    }

    return children;
}

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

EngravingObject* Tuplet::scanParent() const
{
    return elements()[0];
}

EngravingObjectList Tuplet::scanChildren() const
{
    EngravingObjectList children;

    if (_number) {
        children.push_back(_number);
    }

    return children;
}

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

EngravingObject* BarLine::scanParent() const
{
    return segment();
}

EngravingObjectList BarLine::scanChildren() const
{
    EngravingObjectList children;

    for (EngravingItem* element : _el) {
        children.push_back(element);
    }

    return children;
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

EngravingObject* Trill::scanParent() const
{
    return Spanner::scanParent();
}

EngravingObjectList Trill::scanChildren() const
{
    EngravingObjectList children;

    if (_accidental) {
        children.push_back(_accidental);
    }

    for (EngravingObject* child : Spanner::scanChildren()) {
        children.push_back(child);
    }

    return children;
}

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

EngravingObject* TBox::scanParent() const
{
    return explicitParent();
}

EngravingObjectList TBox::scanChildren() const
{
    EngravingObjectList children;

    if (m_text) {
        children.push_back(m_text);
    }

    return children;
}

void TBox::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    m_text->scanElements(data, func, all);
    Box::scanElements(data, func, all);
}

//---------------------------------------------------------
//   dumpScoreTree
///   for debugging purposes
//---------------------------------------------------------

void _dumpScoreTree(EngravingObject* s, int depth)
{
    for (EngravingObject* child : s->scanChildren()) {
        _dumpScoreTree(child, depth + 1);
    }
}

void Score::dumpScoreTree()
{
    _dumpScoreTree(this, 0);
}
} // namespace mu::engraving
