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

#include <QDebug>

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

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   Score
//---------------------------------------------------------

EngravingObject* Score::scanParent() const
{
    return nullptr;  // Score is root node
}

EngravingObject* Score::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return pages()[idx];
}

int Score::scanChildCount() const
{
    return pages().size();
}

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

EngravingObject* Page::scanParent() const
{
    return score();
}

EngravingObject* Page::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return systems()[idx];
}

int Page::scanChildCount() const
{
    return systems().size();
}

//---------------------------------------------------------
//   System
//---------------------------------------------------------

EngravingObject* System::scanParent() const
{
    return page();
}

EngravingObject* System::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (idx < int(brackets().size())) {
        return brackets()[idx];
    }
    idx -= brackets().size();
    if (systemDividerLeft()) {
        if (idx == 0) {
            return systemDividerLeft();
        }
        idx--;
    }
    if (systemDividerRight()) {
        if (idx == 0) {
            return systemDividerRight();
        }
        idx--;
    }
    for (SysStaff* ss : _staves) {
        if (idx < int(ss->instrumentNames.size())) {
            return ss->instrumentNames[idx];
        }
        idx -= ss->instrumentNames.size();
    }
    if (idx < int(measures().size())) {
        return measures()[idx];
    }
    idx -= int(measures().size());
    return nullptr;
}

int System::scanChildCount() const
{
    int numChildren = 0;
    numChildren += brackets().size();
    if (systemDividerLeft()) {
        numChildren++;
    }
    if (systemDividerRight()) {
        numChildren++;
    }
    for (SysStaff* ss : _staves) {
        numChildren += ss->instrumentNames.size();
    }
    numChildren += int(measures().size());
    return numChildren;
}

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

EngravingObject* MeasureBase::scanParent() const
{
    return system();
}

EngravingObject* MeasureBase::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return el()[idx];
}

int MeasureBase::scanChildCount() const
{
    return int(el().size());
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

EngravingObject* Measure::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());

    if (isMMRest()) {
        // if this measure is a MMR measure then add all child measures
        Measure* m1 = mmRestFirst();
        Measure* m2 = mmRestLast();
        while (true) {
            if (idx == 0) {
                return m1;
            }
            idx--;
            if (m1 == m2) {
                break;
            }
            m1 = m1->nextMeasure();
        }
    }

    Segment* seg = m_segments.first();
    while (seg) {
        if (idx == 0) {
            return seg;
        }
        idx--;
        seg = seg->next();
    }
    int nstaves = score()->nstaves();
    for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (m_mstaves[staffIdx]->lines()) {
            if (idx == 0) {
                return m_mstaves[staffIdx]->lines();
            }
            idx--;
        }
        if (vspacerUp(staffIdx)) {
            if (idx == 0) {
                return vspacerUp(staffIdx);
            }
            idx--;
        }
        if (vspacerDown(staffIdx)) {
            if (idx == 0) {
                return vspacerDown(staffIdx);
            }
            idx--;
        }
        if (noText(staffIdx)) {
            if (idx == 0) {
                return noText(staffIdx);
            }
            idx--;
        }
        if (mmRangeText(staffIdx)) {
            if (idx == 0) {
                return mmRangeText(staffIdx);
            }
            idx--;
        }
    }

    const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* sp = i->second;
        if (sp->anchor() == Spanner::Anchor::MEASURE) {
            if (idx == 0) {
                return sp;
            }
            idx--;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }

    return MeasureBase::scanChild(idx);
}

int Measure::scanChildCount() const
{
    int numChildren = 0;
    if (isMMRest()) {
        numChildren += mmRestCount();
    }
    numChildren += m_segments.size();

    int nstaves = score()->nstaves();
    for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (m_mstaves[staffIdx]->lines()) {
            numChildren++;
        }
        if (vspacerUp(staffIdx)) {
            numChildren++;
        }
        if (vspacerDown(staffIdx)) {
            numChildren++;
        }
        if (noText(staffIdx)) {
            numChildren++;
        }
        if (mmRangeText(staffIdx)) {
            numChildren++;
        }
    }

    const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* s = i->second;
        if (s->anchor() == Spanner::Anchor::MEASURE) {
            numChildren++;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            numChildren++;
        }
    }

    numChildren += MeasureBase::scanChildCount();
    return numChildren;
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

EngravingObject* Segment::scanParent() const
{
    return measure();
}

EngravingObject* Segment::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    for (int i = 0; i < int(_elist.size()); i++) {
        if (_elist[i]) {
            if (idx == 0) {
                return _elist[i];
            }
            idx--;
        }
    }
    if (idx < int(_annotations.size())) {
        return _annotations[idx];
    }
    idx -= int(_annotations.size());

    if (segmentType() == SegmentType::ChordRest) {
        const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
        int start_tick = tick().ticks();
        for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
            Spanner* s = i->second;
            if (s->anchor() == Spanner::Anchor::SEGMENT) {
                if (idx == 0) {
                    return s;
                }
                idx--;
            }
        }
        const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
        for (Spanner* s : unmanagedSpanners) {
            if (s->scanParent() == this) {
                if (idx == 0) {
                    return s;
                }
                idx--;
            }
        }
    }

    return nullptr;
}

int Segment::scanChildCount() const
{
    size_t numChildren = 0;
    for (size_t i = 0; i < _elist.size(); i++) {
        if (_elist[i]) {
            numChildren++;
        }
    }
    numChildren += _annotations.size();
    if (segmentType() == SegmentType::ChordRest) {
        const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
        int start_tick = tick().ticks();
        for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
            Spanner* s = i->second;
            if (s->anchor() == Spanner::Anchor::SEGMENT) {
                numChildren++;
            }
        }
        const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
        for (Spanner* s : unmanagedSpanners) {
            if (s->scanParent() == this) {
                numChildren++;
            }
        }
    }

    return int(numChildren);
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

EngravingObject* ChordRest::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (beam() && beam()->scanParent() == this) {
        if (idx == 0) {
            return beam();
        }
        idx--;
    }
    if (idx < int(_lyrics.size())) {
        return _lyrics[idx];
    }
    idx -= int(_lyrics.size());
    const DurationElement* de = this;
    while (de->tuplet() && de->tuplet()->elements().front() == de) {
        if (idx == 0) {
            return de->tuplet();
        }
        idx--;
        de = de->tuplet();
    }
    if (_tabDur) {
        if (idx == 0) {
            return _tabDur;
        }
        idx--;
    }
    if (idx < int(_el.size())) {
        return _el[idx];
    }
    idx -= int(_el.size());

    const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* s = i->second;
        if (s->anchor() == Spanner::Anchor::CHORD && s->scanParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }

    return nullptr;
}

int ChordRest::scanChildCount() const
{
    size_t numChildren = 0;
    if (beam() && beam()->scanParent() == this) {
        numChildren++;
    }
    numChildren += _lyrics.size();
    const DurationElement* de = this;
    while (de->tuplet() && de->tuplet()->elements().front() == de) {
        numChildren++;
        de = de->tuplet();
    }
    if (_tabDur) {
        numChildren++;
    }
    numChildren += _el.size();

    const std::multimap<int, Ms::Spanner*>& spannerMap = score()->spanner();
    int start_tick = tick().ticks();
    for (auto i = spannerMap.lower_bound(start_tick); i != spannerMap.upper_bound(start_tick); ++i) {
        Spanner* s = i->second;
        if (s->anchor() == Spanner::Anchor::CHORD && s->scanParent() == this) {
            numChildren++;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->scanParent() == this) {
            numChildren++;
        }
    }

    return int(numChildren);
}

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

EngravingObject* Chord::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObject* Chord::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());

    if (idx < int(notes().size())) {
        return notes()[idx];
    }
    idx -= int(notes().size());
    if (_arpeggio) {
        if (idx == 0) {
            return _arpeggio;
        }
        idx--;
    }
    if (_tremolo && _tremolo->chord1() == this) {
        if (idx == 0) {
            return _tremolo;
        }
        idx--;
    }
    if (idx < int(graceNotes().size())) {
        return graceNotes()[idx];
    }
    idx -= graceNotes().size();
    if (idx < int(articulations().size())) {
        return articulations()[idx];
    }
    idx -= articulations().size();
    if (stem()) {
        if (idx == 0) {
            return stem();
        }
        idx--;
    }
    if (hook()) {
        if (idx == 0) {
            return hook();
        }
        idx--;
    }
    if (stemSlash()) {
        if (idx == 0) {
            return stemSlash();
        }
        idx--;
    }
    LedgerLine* ll = _ledgerLines;
    while (ll) {
        if (idx == 0) {
            return ll;
        }
        idx--;
        ll = ll->next();
    }
    return ChordRest::scanChild(idx);
}

int Chord::scanChildCount() const
{
    size_t numChildren = 0;

    numChildren += notes().size();
    if (_arpeggio) {
        numChildren++;
    }
    if (_tremolo && _tremolo->chord1() == this) {
        numChildren++;
    }
    numChildren += graceNotes().size();
    numChildren += articulations().size();
    if (stem()) {
        numChildren++;
    }
    if (hook()) {
        numChildren++;
    }
    if (stemSlash()) {
        numChildren++;
    }
    LedgerLine* ll = _ledgerLines;
    while (ll) {
        numChildren++;
        ll = ll->next();
    }
    numChildren += ChordRest::scanChildCount();

    return int(numChildren);
}

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

EngravingObject* Rest::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObject* Rest::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (idx < int(m_dots.size())) {
        return m_dots[idx];
    }
    idx -= int(m_dots.size());
    return ChordRest::scanChild(idx);
}

int Rest::scanChildCount() const
{
    return int(m_dots.size()) + ChordRest::scanChildCount();
}

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

EngravingObject* Note::scanParent() const
{
    return chord();
}

EngravingObject* Note::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (accidental()) {
        if (idx == 0) {
            return accidental();
        }
        idx--;
    }
    if (idx < int(dots().size())) {
        return dots()[idx];
    }
    idx -= dots().size();
    if (tieFor()) {
        if (idx == 0) {
            return tieFor();
        }
        idx--;
    }
    if (idx < int(el().size())) {
        return el()[idx];
    }
    idx -= int(el().size());
    if (idx < int(spannerFor().size())) {
        return spannerFor()[idx];
    }
    idx -= spannerFor().size();
    return nullptr;
}

int Note::scanChildCount() const
{
    size_t numChildren = 0;
    if (accidental()) {
        numChildren++;
    }
    numChildren += dots().size();
    if (tieFor()) {
        numChildren++;
    }
    numChildren += el().size();
    numChildren += spannerFor().size();
    return int(numChildren);
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

EngravingObject* Accidental::scanChild(int idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

int Accidental::scanChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

EngravingObject* Beam::scanParent() const
{
    return _elements[0];
}

EngravingObject* Beam::scanChild(int idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

int Beam::scanChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

EngravingObject* Ambitus::scanParent() const
{
    return segment();
}

EngravingObject* Ambitus::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    Accidental* topAccid = const_cast<Accidental*>(_topAccid);
    Accidental* bottomAccid = const_cast<Accidental*>(_bottomAccid);
    if (topAccid && topAccid->accidentalType() != AccidentalType::NONE) {
        if (idx == 0) {
            return topAccid;
        }
        idx--;
    }
    if (bottomAccid && bottomAccid->accidentalType() != AccidentalType::NONE) {
        if (idx == 0) {
            return bottomAccid;
        }
        idx--;
    }
    return nullptr;
}

int Ambitus::scanChildCount() const
{
    int numChildren = 0;
    Accidental* topAccid = const_cast<Accidental*>(_topAccid);
    Accidental* bottomAccid = const_cast<Accidental*>(_bottomAccid);
    if (topAccid && topAccid->accidentalType() != AccidentalType::NONE) {
        numChildren++;
    }
    if (bottomAccid && bottomAccid->accidentalType() != AccidentalType::NONE) {
        numChildren++;
    }
    return numChildren;
}

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

EngravingObject* FretDiagram::scanParent() const
{
    return segment();
}

EngravingObject* FretDiagram::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (idx == 0) {
        return harmony();
    }
    return nullptr;
}

int FretDiagram::scanChildCount() const
{
    if (harmony()) {
        return 1;
    }
    return 0;
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

EngravingObject* Spanner::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return spannerSegments()[idx];
}

int Spanner::scanChildCount() const
{
    return int(spannerSegments().size());
}

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

EngravingObject* SpannerSegment::scanParent() const
{
    return spanner();
}

EngravingObject* SpannerSegment::scanChild(int idx) const
{
#ifdef NDEBUG
    Q_UNUSED(idx)
#endif
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return nullptr;
}

int SpannerSegment::scanChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

EngravingObject* BSymbol::scanParent() const
{
    return segment();
}

EngravingObject* BSymbol::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return _leafs[idx];
}

int BSymbol::scanChildCount() const
{
    return _leafs.size();
}

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

EngravingObject* Tuplet::scanParent() const
{
    return elements()[0];
}

EngravingObject* Tuplet::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (idx == 0) {
        return _number;
    }
    return nullptr;
}

int Tuplet::scanChildCount() const
{
    if (_number) {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

EngravingObject* BarLine::scanParent() const
{
    return segment();
}

EngravingObject* BarLine::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    return _el[idx];
}

int BarLine::scanChildCount() const
{
    return int(_el.size());
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

EngravingObject* Trill::scanParent() const
{
    return Spanner::scanParent();
}

EngravingObject* Trill::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (accidental()) {
        if (idx == 0) {
            return accidental();
        }
        idx--;
    }
    return Spanner::scanChild(idx);
}

int Trill::scanChildCount() const
{
    if (accidental()) {
        return 1 + Spanner::scanChildCount();
    }
    return Spanner::scanChildCount();
}

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

EngravingObject* TBox::scanParent() const
{
    return explicitParent();
}

EngravingObject* TBox::scanChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < scanChildCount());
    if (idx == 0) {
        return _text;
    }
    return nullptr;
}

int TBox::scanChildCount() const
{
    if (_text) {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------
//   dumpScoreTree
///   for debugging purposes
//---------------------------------------------------------

void _dumpScoreTree(EngravingObject* s, int depth)
{
    qDebug() << qPrintable(QString(" ").repeated(4 * depth)) << s->typeName() << "at" << s;
    for (int i = 0; i < s->scanChildCount(); ++i) {
        EngravingObject* c = s->scanChild(i);
        _dumpScoreTree(c, depth + 1);
    }
}

void Score::dumpScoreTree()
{
    _dumpScoreTree(this, 0);
}
}  // namespace Ms
