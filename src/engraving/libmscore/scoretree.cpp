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

EngravingObject* Score::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return pages()[idx];
}

size_t Score::scanChildCount() const
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

EngravingObject* Page::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return systems()[idx];
}

size_t Page::scanChildCount() const
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

EngravingObject* System::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (idx < brackets().size()) {
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
        if (idx < ss->instrumentNames.size()) {
            return ss->instrumentNames[idx];
        }
        idx -= ss->instrumentNames.size();
    }
    if (idx < measures().size()) {
        return measures()[idx];
    }
    idx -= measures().size();
    return nullptr;
}

size_t System::scanChildCount() const
{
    size_t numChildren = 0;
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

EngravingObject* MeasureBase::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return el()[idx];
}

size_t MeasureBase::scanChildCount() const
{
    return el().size();
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

EngravingObject* Measure::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());

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

size_t Measure::scanChildCount() const
{
    size_t numChildren = 0;
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

EngravingObject* Segment::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    for (size_t i = 0; i < _elist.size(); i++) {
        if (_elist[i]) {
            if (idx == 0) {
                return _elist[i];
            }
            idx--;
        }
    }
    if (idx < _annotations.size()) {
        return _annotations[idx];
    }
    idx -= _annotations.size();

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

size_t Segment::scanChildCount() const
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

    return numChildren;
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

EngravingObject* ChordRest::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (beam() && beam()->scanParent() == this) {
        if (idx == 0) {
            return beam();
        }
        idx--;
    }
    if (idx < _lyrics.size()) {
        return _lyrics[idx];
    }
    idx -= _lyrics.size();
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
    if (idx < _el.size()) {
        return _el[idx];
    }
    idx -= _el.size();

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

size_t ChordRest::scanChildCount() const
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

    return numChildren;
}

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

EngravingObject* Chord::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObject* Chord::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());

    if (idx < notes().size()) {
        return notes()[idx];
    }
    idx -= notes().size();
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
    if (idx < graceNotes().size()) {
        return graceNotes()[idx];
    }
    idx -= graceNotes().size();
    if (idx < articulations().size()) {
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

size_t Chord::scanChildCount() const
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
    numChildren += (int)ChordRest::scanChildCount();

    return numChildren;
}

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

EngravingObject* Rest::scanParent() const
{
    return ChordRest::scanParent();
}

EngravingObject* Rest::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (idx < m_dots.size()) {
        return m_dots[idx];
    }
    idx -= m_dots.size();
    return ChordRest::scanChild(idx);
}

size_t Rest::scanChildCount() const
{
    return m_dots.size() + ChordRest::scanChildCount();
}

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

EngravingObject* Note::scanParent() const
{
    return chord();
}

EngravingObject* Note::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (accidental()) {
        if (idx == 0) {
            return accidental();
        }
        idx--;
    }
    if (idx < dots().size()) {
        return dots()[idx];
    }
    idx -= dots().size();
    if (tieFor()) {
        if (idx == 0) {
            return tieFor();
        }
        idx--;
    }
    if (idx < el().size()) {
        return el()[idx];
    }
    idx -= el().size();
    if (idx < spannerFor().size()) {
        return spannerFor()[idx];
    }
    idx -= spannerFor().size();
    return nullptr;
}

size_t Note::scanChildCount() const
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
    return numChildren;
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

EngravingObject* Accidental::scanChild(size_t idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

size_t Accidental::scanChildCount() const
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

EngravingObject* Beam::scanChild(size_t idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

size_t Beam::scanChildCount() const
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

EngravingObject* Ambitus::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
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

size_t Ambitus::scanChildCount() const
{
    size_t numChildren = 0;
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

EngravingObject* FretDiagram::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (idx == 0) {
        return harmony();
    }
    return nullptr;
}

size_t FretDiagram::scanChildCount() const
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

EngravingObject* Spanner::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return spannerSegments()[idx];
}

size_t Spanner::scanChildCount() const
{
    return spannerSegments().size();
}

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

EngravingObject* SpannerSegment::scanParent() const
{
    return spanner();
}

EngravingObject* SpannerSegment::scanChild(size_t idx) const
{
#ifdef NDEBUG
    Q_UNUSED(idx)
#endif
    Q_ASSERT(idx < scanChildCount());
    return nullptr;
}

size_t SpannerSegment::scanChildCount() const
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

EngravingObject* BSymbol::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return _leafs[idx];
}

size_t BSymbol::scanChildCount() const
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

EngravingObject* Tuplet::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (idx == 0) {
        return _number;
    }
    return nullptr;
}

size_t Tuplet::scanChildCount() const
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

EngravingObject* BarLine::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    return _el[idx];
}

size_t BarLine::scanChildCount() const
{
    return _el.size();
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

EngravingObject* Trill::scanParent() const
{
    return Spanner::scanParent();
}

EngravingObject* Trill::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (accidental()) {
        if (idx == 0) {
            return accidental();
        }
        idx--;
    }
    return Spanner::scanChild(idx);
}

size_t Trill::scanChildCount() const
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

EngravingObject* TBox::scanChild(size_t idx) const
{
    Q_ASSERT(idx < scanChildCount());
    if (idx == 0) {
        return _text;
    }
    return nullptr;
}

size_t TBox::scanChildCount() const
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
    for (size_t i = 0; i < s->scanChildCount(); ++i) {
        EngravingObject* c = s->scanChild(i);
        _dumpScoreTree(c, depth + 1);
    }
}

void Score::dumpScoreTree()
{
    _dumpScoreTree(this, 0);
}
}  // namespace Ms
