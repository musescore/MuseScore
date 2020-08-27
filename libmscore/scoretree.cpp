//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
#include "iname.h"
#include "ledgerline.h"
#include "lyrics.h"
#include "measure.h"
#include "measurenumber.h"
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

namespace Ms {
//---------------------------------------------------------
//   Score
//---------------------------------------------------------

ScoreElement* Score::treeParent() const
{
    return nullptr;  // Score is root node
}

ScoreElement* Score::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return pages()[idx];
}

int Score::treeChildCount() const
{
    return pages().size();
}

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

ScoreElement* Page::treeParent() const
{
    return score();
}

ScoreElement* Page::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return systems()[idx];
}

int Page::treeChildCount() const
{
    return systems().size();
}

//---------------------------------------------------------
//   System
//---------------------------------------------------------

ScoreElement* System::treeParent() const
{
    return page();
}

ScoreElement* System::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
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

int System::treeChildCount() const
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

ScoreElement* MeasureBase::treeParent() const
{
    return system();
}

ScoreElement* MeasureBase::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return el()[idx];
}

int MeasureBase::treeChildCount() const
{
    return int(el().size());
}

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

ScoreElement* Measure::treeParent() const
{
    // A MMRest measure will contain Measures that it has replaced
    // System > MMR > Measure
    if (isMMRest()) {  // this is MMR
        return system();
    } else if (_mmRestCount < 0) {  // this is part of MMR
        return const_cast<Measure*>(mmRest1());
    }
    // for a normal measure
    return system();
}

ScoreElement* Measure::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());

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

    Segment* seg = _segments.first();
    while (seg) {
        if (idx == 0) {
            return seg;
        }
        idx--;
        seg = seg->next();
    }
    int nstaves = score()->nstaves();
    for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (_mstaves[staffIdx]->lines()) {
            if (idx == 0) {
                return _mstaves[staffIdx]->lines();
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
        if (s->treeParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }

    return MeasureBase::treeChild(idx);
}

int Measure::treeChildCount() const
{
    int numChildren = 0;
    if (isMMRest()) {
        numChildren += mmRestCount();
    }
    numChildren += _segments.size();

    int nstaves = score()->nstaves();
    for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (_mstaves[staffIdx]->lines()) {
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
        if (s->treeParent() == this) {
            numChildren++;
        }
    }

    numChildren += MeasureBase::treeChildCount();
    return numChildren;
}

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

ScoreElement* Segment::treeParent() const
{
    return measure();
}

ScoreElement* Segment::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
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
            if (s->treeParent() == this) {
                if (idx == 0) {
                    return s;
                }
                idx--;
            }
        }
    }

    return nullptr;
}

int Segment::treeChildCount() const
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
            if (s->treeParent() == this) {
                numChildren++;
            }
        }
    }

    return int(numChildren);
}

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ScoreElement* ChordRest::treeParent() const
{
    if (isGrace()) {
        // grace notes do not have a segment of their own
        // their parent is the chord they are attached to
        return parent();
    }
    return segment();
}

ScoreElement* ChordRest::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (beam() && beam()->treeParent() == this) {
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
        if (s->anchor() == Spanner::Anchor::CHORD && s->treeParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->treeParent() == this) {
            if (idx == 0) {
                return s;
            }
            idx--;
        }
    }

    return nullptr;
}

int ChordRest::treeChildCount() const
{
    size_t numChildren = 0;
    if (beam() && beam()->treeParent() == this) {
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
        if (s->anchor() == Spanner::Anchor::CHORD && s->treeParent() == this) {
            numChildren++;
        }
    }
    const std::set<Spanner*>& unmanagedSpanners = score()->unmanagedSpanners();
    for (Spanner* s : unmanagedSpanners) {
        if (s->treeParent() == this) {
            numChildren++;
        }
    }

    return int(numChildren);
}

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

ScoreElement* Chord::treeParent() const
{
    return ChordRest::treeParent();
}

ScoreElement* Chord::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());

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
    return ChordRest::treeChild(idx);
}

int Chord::treeChildCount() const
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
    numChildren += ChordRest::treeChildCount();

    return int(numChildren);
}

//---------------------------------------------------------
//   Rest
//---------------------------------------------------------

ScoreElement* Rest::treeParent() const
{
    return ChordRest::treeParent();
}

ScoreElement* Rest::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (idx < int(m_dots.size())) {
        return m_dots[idx];
    }
    idx -= int(m_dots.size());
    return ChordRest::treeChild(idx);
}

int Rest::treeChildCount() const
{
    return int(m_dots.size()) + ChordRest::treeChildCount();
}

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

ScoreElement* Note::treeParent() const
{
    return chord();
}

ScoreElement* Note::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
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

int Note::treeChildCount() const
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

ScoreElement* Accidental::treeParent() const
{
    if (parent()->isTrillSegment()) {
        return parent()->treeParent();
    }
    return note();
}

ScoreElement* Accidental::treeChild(int idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

int Accidental::treeChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

ScoreElement* Beam::treeParent() const
{
    return _elements[0];
}

ScoreElement* Beam::treeChild(int idx) const
{
    Q_UNUSED(idx);
    return nullptr;
}

int Beam::treeChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

ScoreElement* Ambitus::treeParent() const
{
    return segment();
}

ScoreElement* Ambitus::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    Accidental* topAccid = const_cast<Accidental*>(&_topAccid);
    Accidental* bottomAccid = const_cast<Accidental*>(&_bottomAccid);
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

int Ambitus::treeChildCount() const
{
    int numChildren = 0;
    Accidental* topAccid = const_cast<Accidental*>(&_topAccid);
    Accidental* bottomAccid = const_cast<Accidental*>(&_bottomAccid);
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

ScoreElement* FretDiagram::treeParent() const
{
    return segment();
}

ScoreElement* FretDiagram::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (idx == 0) {
        return harmony();
    }
    return nullptr;
}

int FretDiagram::treeChildCount() const
{
    if (harmony()) {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

ScoreElement* Spanner::treeParent() const
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

ScoreElement* Spanner::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return spannerSegments()[idx];
}

int Spanner::treeChildCount() const
{
    return int(spannerSegments().size());
}

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

ScoreElement* SpannerSegment::treeParent() const
{
    return spanner();
}

ScoreElement* SpannerSegment::treeChild(int idx) const
{
#ifdef NDEBUG
    Q_UNUSED(idx)
#endif
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return nullptr;
}

int SpannerSegment::treeChildCount() const
{
    return 0;
}

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

ScoreElement* BSymbol::treeParent() const
{
    return segment();
}

ScoreElement* BSymbol::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return _leafs[idx];
}

int BSymbol::treeChildCount() const
{
    return _leafs.size();
}

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

ScoreElement* Tuplet::treeParent() const
{
    return elements()[0];
}

ScoreElement* Tuplet::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (idx == 0) {
        return _number;
    }
    return nullptr;
}

int Tuplet::treeChildCount() const
{
    if (_number) {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

ScoreElement* BarLine::treeParent() const
{
    return segment();
}

ScoreElement* BarLine::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    return _el[idx];
}

int BarLine::treeChildCount() const
{
    return int(_el.size());
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

ScoreElement* Trill::treeParent() const
{
    return Spanner::treeParent();
}

ScoreElement* Trill::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (accidental()) {
        if (idx == 0) {
            return accidental();
        }
        idx--;
    }
    return Spanner::treeChild(idx);
}

int Trill::treeChildCount() const
{
    if (accidental()) {
        return 1 + Spanner::treeChildCount();
    }
    return Spanner::treeChildCount();
}

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

ScoreElement* TBox::treeParent() const
{
    return parent();
}

ScoreElement* TBox::treeChild(int idx) const
{
    Q_ASSERT(0 <= idx && idx < treeChildCount());
    if (idx == 0) {
        return _text;
    }
    return nullptr;
}

int TBox::treeChildCount() const
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

void _dumpScoreTree(ScoreElement* s, int depth)
{
    qDebug() << qPrintable(QString(" ").repeated(4 * depth)) << s->name() << "at" << s;
    for (ScoreElement* c : *s) {
        _dumpScoreTree(c, depth + 1);
    }
}

void Score::dumpScoreTree()
{
    _dumpScoreTree(this, 0);
}
}  // namespace Ms
