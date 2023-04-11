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

#include "beam.h"

#include <cmath>
#include <set>
#include <algorithm>

#include "containers.h"
#include "realfn.h"

#include "draw/types/brush.h"
#include "rw/xml.h"

#include "actionicon.h"
#include "chord.h"
#include "groups.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "spanner.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "stemslash.h"
#include "system.h"
#include "tremolo.h"
#include "tuplet.h"

#include "layout/layoutbeams.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle beamStyle {
    { Sid::beamNoSlope,                        Pid::BEAM_NO_SLOPE },
};

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(System* parent)
    : EngravingItem(ElementType::BEAM, parent)
{
    initElementStyle(&beamStyle);
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(const Beam& b)
    : EngravingItem(b)
{
    _elements     = b._elements;
    _id           = b._id;
    for (const BeamSegment* bs : b._beamSegments) {
        _beamSegments.push_back(new BeamSegment(*bs));
    }
    _direction       = b._direction;
    _up              = b._up;
    _userModified[0] = b._userModified[0];
    _userModified[1] = b._userModified[1];
    _grow1           = b._grow1;
    _grow2           = b._grow2;
    _beamDist        = b._beamDist;
    for (const BeamFragment* f : b.fragments) {
        fragments.push_back(new BeamFragment(*f));
    }
    _minMove          = b._minMove;
    _maxMove          = b._maxMove;
    _isGrace          = b._isGrace;
    _cross            = b._cross;
    _slope            = b._slope;
    _layoutInfo       = b._layoutInfo;
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
{
    //
    // delete all references from chords
    //
    for (ChordRest* cr : _elements) {
        cr->setBeam(0);
    }
    DeleteAll(_beamSegments);
    DeleteAll(fragments);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Beam::pagePos() const
{
    System* s = system();
    if (s == 0) {
        return pos();
    }
    double yp = y() + s->staff(staffIdx())->y() + s->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF Beam::canvasPos() const
{
    PointF p(pagePos());
    if (system() && system()->explicitParent()) {
        p += system()->parentItem()->pos();
    }
    return p;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Beam::add(EngravingItem* e)
{
    if (e->isChordRest()) {
        addChordRest(toChordRest(e));
        e->added();
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(EngravingItem* e)
{
    if (e->isChordRest()) {
        removeChordRest(toChordRest(e));
        e->removed();
    }
}

//---------------------------------------------------------
//   addChordRest
//---------------------------------------------------------

void Beam::addChordRest(ChordRest* a)
{
    a->setBeam(this);
    if (!mu::contains(_elements, a)) {
        //
        // insert element in same order as it appears
        // in the score
        //
        if (a->segment() && !_elements.empty()) {
            for (size_t i = 0; i < _elements.size(); ++i) {
                Segment* s = _elements[i]->segment();
                if ((s->tick() > a->segment()->tick())
                    || ((s->tick() == a->segment()->tick()) && (a->segment()->next(SegmentType::ChordRest) == s))
                    ) {
                    _elements.insert(_elements.begin() + i, a);
                    return;
                }
            }
        }
        _elements.push_back(a);
    }
}

//---------------------------------------------------------
//   removeChordRest
//---------------------------------------------------------

void Beam::removeChordRest(ChordRest* a)
{
    if (!mu::remove(_elements, a)) {
        LOGD("Beam::remove(): cannot find ChordRest");
    }
    a->setBeam(0);
}

const Chord* Beam::findChordWithCustomStemDirection() const
{
    for (const ChordRest* rest: elements()) {
        if (!rest->isChord()) {
            continue;
        }

        const Chord* chord = toChord(rest);
        if (chord->stemDirection() != DirectionV::AUTO) {
            return chord;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (_beamSegments.empty()) {
        return;
    }
    painter->setBrush(mu::draw::Brush(curColor()));
    painter->setNoPen();

    // make beam thickness independent of slant
    // (expression can be simplified?)

    const LineF bs = _beamSegments.front()->line;
    double d  = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
    if (_beamSegments.size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww = (_beamWidth / 2.0) / sin(M_PI_2 - atan(d));

    for (const BeamSegment* bs1 : _beamSegments) {
        painter->drawPolygon(
            PolygonF({
                PointF(bs1->line.x1(), bs1->line.y1() - ww),
                PointF(bs1->line.x2(), bs1->line.y2() - ww),
                PointF(bs1->line.x2(), bs1->line.y2() + ww),
                PointF(bs1->line.x1(), bs1->line.y1() + ww),
            }),
            draw::FillRule::OddEvenFill);
    }
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(const PointF& offset)
{
    EngravingItem::move(offset);
    for (BeamSegment* bs : _beamSegments) {
        bs->line.translate(offset);
    }
}

//---------------------------------------------------------
//   layout1
//    - remove beam segments
//    - detach from system
//    - calculate stem direction and set chord
//---------------------------------------------------------

void Beam::layout1()
{
    resetExplicitParent();  // parent is System

    const StaffType* staffType = this->staffType();
    _tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    _isBesideTabStaff = _tab && !_tab->stemless() && !_tab->stemThrough();

    // TAB's with stem beside staves have special layout
    if (_isBesideTabStaff) {
        _up = !_tab->stemsDown();
        _slope = 0.0;
        _cross = false;
        _minMove = 0;
        _maxMove = 0;
        for (ChordRest* cr : _elements) {
            if (cr->isChord()) {
                _up = cr->up();
                break;
            }
        }
        return;
    }

    if (staff()->isDrumStaff(Fraction(0, 1))) {
        if (_direction != DirectionV::AUTO) {
            _up = _direction == DirectionV::UP;
        } else if (_isGrace) {
            _up = true;
        } else {
            // search through the beam for the first chord with explicit stem direction and use that.
            // if there is no explicit stem direction, default to the direction of the first stem.
            bool firstUp = false;
            bool firstChord = true;
            for (ChordRest* cr :_elements) {
                if (cr->isChord()) {
                    DirectionV crDirection = toChord(cr)->stemDirection();
                    if (crDirection != DirectionV::AUTO) {
                        _up = crDirection == DirectionV::UP;
                        break;
                    } else if (firstChord) {
                        firstUp = cr->up();
                        firstChord = false;
                    }
                }
                _up = firstUp;
            }
        }
        for (ChordRest* cr : _elements) {
            cr->computeUp();
            if (cr->isChord()) {
                toChord(cr)->layoutStem();
            }
        }
        return;
    }

    _minMove = std::numeric_limits<int>::max();
    _maxMove = std::numeric_limits<int>::min();
    double mag = 0.0;

    _notes.clear();
    staff_idx_t staffIdx = mu::nidx;
    for (ChordRest* cr : _elements) {
        double m = cr->isSmall() ? score()->styleD(Sid::smallNoteMag) : 1.0;
        mag = std::max(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            staffIdx = chord->vStaffIdx();
            int i = chord->staffMove();
            _minMove = std::min(_minMove, i);
            _maxMove = std::max(_maxMove, i);

            for (int distance : chord->noteDistances()) {
                _notes.push_back(distance);
            }
        }
    }

    std::sort(_notes.begin(), _notes.end());
    setMag(mag);

    //
    // determine beam stem direction
    //
    if (_elements.empty()) {
        return;
    }
    ChordRest* firstNote = _elements.front();
    Measure* measure = firstNote->measure();
    bool hasMultipleVoices = measure->hasVoices(firstNote->staffIdx(), tick(), ticks());
    if (_direction != DirectionV::AUTO) {
        _up = _direction == DirectionV::UP;
    } else if (_maxMove > 0) {
        _up = false;
    } else if (_minMove < 0) {
        _up = true;
    } else if (_isGrace) {
        if (hasMultipleVoices) {
            _up = firstNote->track() % 2 == 0;
        } else {
            _up = true;
        }
    } else if (_notes.size()) {
        if (hasMultipleVoices) {
            _up = firstNote->track() % 2 == 0;
        } else {
            if (const Chord* chord = findChordWithCustomStemDirection()) {
                _up = chord->stemDirection() == DirectionV::UP;
            } else {
                std::set<int> noteSet(_notes.begin(), _notes.end());
                std::vector<int> notes(noteSet.begin(), noteSet.end());
                _up = Chord::computeAutoStemDirection(notes) > 0;
            }
        }
    } else {
        _up = true;
    }

    int middleStaffLine = firstNote->staffType()->middleLine();
    for (size_t i = 0; i < _notes.size(); i++) {
        _notes[i] += middleStaffLine;
    }

    _cross = _minMove != _maxMove;
    bool isEntirelyMoved = false;
    if (_minMove == _maxMove && _minMove != 0) {
        isEntirelyMoved = true;
        setStaffIdx(staffIdx);
        if (_direction == DirectionV::AUTO) {
            _up = _maxMove > 0;
        }
    } else if (_elements.size()) {
        setStaffIdx(_elements.at(0)->staffIdx());
    }

    _slope = 0.0;

    for (ChordRest* cr : _elements) {
        const bool staffMove = cr->isChord() ? toChord(cr)->staffMove() : false;
        if (!_cross || !staffMove) {
            if (cr->up() != _up) {
                cr->setUp(isEntirelyMoved ? _up : (_up != staffMove));
                if (cr->isChord()) {
                    toChord(cr)->layoutStem();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layout
//   TODO - document what this function does
//---------------------------------------------------------

void Beam::layout()
{
    // all of the beam layout code depends on _elements being in order by tick
    // this may not be the case if two cr's were recently swapped.
    std::sort(_elements.begin(), _elements.end(),
              [](const ChordRest* a, const ChordRest* b) -> bool {
        return a->tick() < b->tick();
    });
    System* system = _elements.front()->measure()->system();
    setParent(system);

    std::vector<ChordRest*> crl;

    size_t n = 0;
    for (ChordRest* cr : _elements) {
        auto newSystem = cr->measure()->system();
        if (newSystem && newSystem != system) {
            SpannerSegmentType st;
            if (n == 0) {
                st = SpannerSegmentType::BEGIN;
            } else {
                st = SpannerSegmentType::MIDDLE;
            }
            ++n;
            if (fragments.size() < n) {
                fragments.push_back(new BeamFragment);
            }
            layout2(crl, st, static_cast<int>(n) - 1);
            crl.clear();
            system = cr->measure()->system();
        }
        crl.push_back(cr);
    }
    setbbox(RectF());
    if (!crl.empty()) {
        SpannerSegmentType st;
        if (n == 0) {
            st = SpannerSegmentType::SINGLE;
        } else {
            st = SpannerSegmentType::END;
        }
        if (fragments.size() < (n + 1)) {
            fragments.push_back(new BeamFragment);
        }
        layout2(crl, st, static_cast<int>(n));

        double lw2 = _beamWidth / 2.0;

        for (const BeamSegment* bs : _beamSegments) {
            PolygonF a(4);
            a[0] = PointF(bs->line.x1(), bs->line.y1());
            a[1] = PointF(bs->line.x2(), bs->line.y2());
            a[2] = PointF(bs->line.x2(), bs->line.y2());
            a[3] = PointF(bs->line.x1(), bs->line.y1());
            RectF r(a.boundingRect().adjusted(0.0, -lw2, 0.0, lw2));
            addbbox(r);
        }
    }
}

//---------------------------------------------------------
//   layoutIfNeed
//   check to see if the layout info is valid, and if not, layout
//---------------------------------------------------------

void Beam::layoutIfNeed()
{
    if (!_layoutInfo.isValid()) {
        layout();
    }
}

PointF Beam::chordBeamAnchor(const ChordRest* chord, BeamTremoloLayout::ChordBeamAnchorType anchorType) const
{
    return _layoutInfo.chordBeamAnchor(chord, anchorType);
}

double Beam::chordBeamAnchorY(const ChordRest* chord) const
{
    return _layoutInfo.chordBeamAnchorY(chord);
}

void Beam::setTremAnchors()
{
    _tremAnchors.clear();
    for (ChordRest* cr : _elements) {
        if (!cr || !cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        Tremolo* t = c ? c->tremolo() : nullptr;
        if (t && t->twoNotes() && t->chord1() == c && t->chord2()->beam() == this) {
            // there is an inset tremolo here!
            // figure out up / down
            bool tremUp = t->up();
            int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            if (_userModified[fragmentIndex]) {
                tremUp = c->up();
            } else if (_cross && t->chord1()->staffMove() == t->chord2()->staffMove()) {
                tremUp = t->chord1()->staffMove() == _maxMove;
            }
            TremAnchor tremAnchor;
            tremAnchor.chord1 = c;
            int regularBeams = c->beams(); // non-tremolo strokes

            // find the left-side anchor
            double width = _endAnchor.x() - _startAnchor.x();
            double height = _endAnchor.y() - _startAnchor.y();
            double x = chordBeamAnchor(c, BeamTremoloLayout::ChordBeamAnchorType::Middle).x();
            double proportionAlongX = (x - _startAnchor.x()) / width;
            double y = _startAnchor.y() + (proportionAlongX * height);
            y += regularBeams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) * spatium() * (tremUp ? 1. : -1.);
            tremAnchor.y1 = y;
            // find the right-side anchor
            x = chordBeamAnchor(t->chord2(), BeamTremoloLayout::ChordBeamAnchorType::Middle).x();
            proportionAlongX = (x - _startAnchor.x()) / width;
            y = _startAnchor.y() + (proportionAlongX * height);
            y += regularBeams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) * spatium() * (tremUp ? 1. : -1.);
            tremAnchor.y2 = y;
            _tremAnchors.push_back(tremAnchor);
        }
    }
}

void Beam::createBeamSegment(ChordRest* startCr, ChordRest* endCr, int level)
{
    const bool isFirstSubgroup = startCr == _elements.front();
    const bool isLastSubgroup = endCr == _elements.back();
    const bool firstUp = startCr->up();
    const bool lastUp = endCr->up();
    bool overallUp = _up;
    if (isFirstSubgroup == isLastSubgroup) {
        // this subgroup is either the only one in the beam, or in the middle
        if (firstUp == lastUp) {
            // the "outside notes" of this subgroup go the same direction so use them
            // to determine the side of the beams
            overallUp = firstUp;
        } else {
            // no perfect way to solve this problem, for now we'll base it on the number of
            // up and down stemmed notes in this subgroup
            int upStems, downStems;
            upStems = downStems = 0;
            for (ChordRest* cr : _elements) {
                if (!cr->isChord() || cr->tick() < startCr->tick()) {
                    continue;
                }
                if (cr->tick() > endCr->tick()) {
                    break;
                }

                ++(toChord(cr)->up() ? upStems : downStems);

                if (cr == endCr) {
                    break;
                }
            }
            if (upStems == downStems) {
                // we are officially bamboozled. for now we can just use the default
                // direction based on the staff we're on I guess
                overallUp = _up;
            } else {
                // use the direction with the most stems
                overallUp = upStems > downStems;
            }
        }
    } else if (isFirstSubgroup) {
        overallUp = lastUp;
    } else if (isLastSubgroup) {
        overallUp = firstUp;
    }

    const double startX = _layoutInfo.chordBeamAnchorX(startCr, BeamTremoloLayout::ChordBeamAnchorType::Start);
    const double endX = _layoutInfo.chordBeamAnchorX(endCr, BeamTremoloLayout::ChordBeamAnchorType::End);

    double startY = _slope * (startX - _startAnchor.x()) + _startAnchor.y() - pagePos().y();
    double endY = _slope * (endX - _startAnchor.x()) + _startAnchor.y() - pagePos().y();

    int beamsBelow = 0; // how many beams below level 0?
    int beamsAbove = 0; // how many beams above level 0?

    // avoid adjusting for beams on opposite side of level 0
    if (level != 0) {
        for (const BeamSegment* beam : _beamSegments) {
            if (beam->level == 0 || beam->endTick < startCr->tick() || beam->startTick > endCr->tick()) {
                continue;
            }
            ++(beam->above ? beamsAbove : beamsBelow);
        }

        const int upValue = overallUp ? -1 : 1;
        const int extraBeamAdjust = overallUp ? beamsAbove : beamsBelow;
        const double verticalOffset = _beamDist * (level - extraBeamAdjust) * upValue;

        if (RealIsEqual(_grow1, _grow2)) {
            startY -= verticalOffset * _grow1;
            endY -= verticalOffset * _grow1;
        } else {
            // Feathered beams
            double startProportionAlongX = (startX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());
            double endProportionAlongX = (endX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());

            double grow1 = startProportionAlongX * (_grow2 - _grow1) + _grow1;
            double grow2 = endProportionAlongX * (_grow2 - _grow1) + _grow1;

            startY -= verticalOffset * grow1;
            endY -= verticalOffset * grow2;
        }
    }

    BeamSegment* b = new BeamSegment(this);
    b->above = !overallUp;
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
    b->startTick = startCr->tick();
    b->endTick = endCr->tick();
    _beamSegments.push_back(b);

    if (level > 0) {
        ++(b->above ? beamsAbove : beamsBelow);
    }

    // extend stems properly
    for (ChordRest* cr : _elements) {
        if (!cr->isChord() || cr->tick() < startCr->tick()) {
            continue;
        }
        if (cr->tick() > endCr->tick()) {
            break;
        }

        Chord* chord = toChord(cr);
        double addition = 0.0;

        if (level > 0) {
            double grow = _grow1;
            if (!RealIsEqual(_grow1, _grow2)) {
                double anchorX = _layoutInfo.chordBeamAnchorX(chord, BeamTremoloLayout::ChordBeamAnchorType::Middle);
                double proportionAlongX = (anchorX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());
                grow = proportionAlongX * (_grow2 - _grow1) + _grow1;
            }

            int extraBeamAdjust = cr->up() ? beamsBelow : beamsAbove;
            addition = grow * (level - extraBeamAdjust) * _beamDist;
        }

        if (level == 0 || !RealIsEqual(addition, 0.0)) {
            _layoutInfo.extendStem(chord, addition);
        }

        if (chord == endCr) {
            break;
        }
    }
}

bool Beam::calcIsBeamletBefore(Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break) const
{
    // if first or last chord in beam group
    if (i == 0) {
        return false;
    } else if (i == static_cast<int>(_elements.size()) - 1) {
        return true;
    }
    // if first or last chord in tuplet
    Tuplet* tuplet = chord->tuplet();
    if (tuplet && chord == tuplet->elements().front()) {
        return false;
    } else if (tuplet && chord == tuplet->elements().back()) {
        return true;
    }

    // next note has a beam break
    ChordRest* nextChordRest = _elements[i + 1];
    ChordRest* currChordRest = _elements[i];
    ChordRest* prevChordRest = _elements[i - 1];
    if (nextChordRest->isChord()) {
        bool nextBreak32 = false;
        bool nextBreak64 = false;
        bool currBreak32 = false;
        bool currBreak64 = false;
        calcBeamBreaks(currChordRest, prevChordRest, prevChordRest->beams(), currBreak32, currBreak64);
        calcBeamBreaks(nextChordRest, currChordRest, level, nextBreak32, nextBreak64);
        if ((nextBreak32 && level >= 1) || (!currBreak32 && nextBreak64 && level >= 2)) {
            return true;
        }
    }

    // if previous or next chord has more beams, point in that direction
    int previousChordLevel = -1;
    int nextChordLevel = -1;
    int previousOffset = 1;
    while (i - previousOffset >= 0) {
        ChordRest* previous = _elements[i - previousOffset];
        if (previous->isChord()) {
            previousChordLevel = toChord(previous)->beams();
            if (isAfter32Break) {
                previousChordLevel = std::min(previousChordLevel, 1);
            } else if (isAfter64Break) {
                previousChordLevel = std::min(previousChordLevel, 2);
            }
            break;
        }
        ++previousOffset;
    }

    int nextOffset = 1;
    while (i + nextOffset < static_cast<int>(_elements.size())) {
        ChordRest* next = _elements[i + nextOffset];
        if (next->isChord()) {
            nextChordLevel = toChord(next)->beams();
            break;
        }
        ++nextOffset;
    }
    int chordLevelDifference = nextChordLevel - previousChordLevel;
    if (chordLevelDifference != 0) {
        return chordLevelDifference < 0;
    }

    // if the chord ends a subdivision of the beat
    Fraction baseTick = tuplet ? tuplet->tick() : chord->measure()->tick();
    Fraction tickNext = nextChordRest->tick() - baseTick;
    if (tuplet) {
        // for tuplets with odd ratios, apply ratio
        // for tuplets with even ratios, use actual beat
        Fraction ratio = tuplet->ratio();
        if (ratio.numerator() & 1) {
            tickNext *= ratio;
        }
    }

    static const int BEAM_TUPLET_TOLERANCE = 6;
    int tickLargeSize  = chord->ticks().ticks() * 2;
    int remainder = tickNext.ticks() % tickLargeSize;
    if (remainder <= BEAM_TUPLET_TOLERANCE || (tickLargeSize - remainder) <= BEAM_TUPLET_TOLERANCE) {
        return true;
    }

    // default case
    return false;
}

void Beam::createBeamletSegment(ChordRest* cr, bool isBefore, int level)
{
    const double startX = _layoutInfo.chordBeamAnchorX(cr,
                                                       isBefore ? BeamTremoloLayout::ChordBeamAnchorType::End : BeamTremoloLayout::ChordBeamAnchorType::Start);

    const double beamletLength = score()->styleMM(Sid::beamMinLen).val()
                                 * cr->mag();

    const double endX = startX + (isBefore ? -beamletLength : beamletLength);

    double startY = _slope * (startX - _startAnchor.x()) + _startAnchor.y() - pagePos().y();
    double endY = _slope * (endX - startX) + startY;

    // how many beams past level 0 (i.e. beams on the other side of level 0 for this subgroup)
    int extraBeamAdjust = 0;

    // avoid adjusting for beams on opposite side of level 0
    for (const BeamSegment* beam : _beamSegments) {
        if (beam->level == 0 || beam->line.x2() < startX || beam->line.x1() > endX) {
            continue;
        }

        if (cr->up() == beam->above) {
            extraBeamAdjust++;
        }
    }

    const int upValue = cr->up() ? -1 : 1;
    const double verticalOffset = _beamDist * (level - extraBeamAdjust) * upValue;

    if (RealIsEqual(_grow1, _grow2)) {
        startY -= verticalOffset * _grow1;
        endY -= verticalOffset * _grow1;
    } else {
        // Feathered beams
        double startProportionAlongX = (startX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());
        double endProportionAlongX = (endX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());

        double grow1 = startProportionAlongX * (_grow2 - _grow1) + _grow1;
        double grow2 = endProportionAlongX * (_grow2 - _grow1) + _grow1;

        startY -= verticalOffset * grow1;
        endY -= verticalOffset * grow2;
    }

    BeamSegment* b = new BeamSegment(this);
    b->above = !cr->up();
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
    b->isBeamlet = true;
    b->isBefore = isBefore;
    cr->setBeamlet(b);
    _beamSegments.push_back(b);
}

void Beam::calcBeamBreaks(const ChordRest* chord, const ChordRest* prevChord, int level, bool& isBroken32, bool& isBroken64) const
{
    BeamMode beamMode = chord->beamMode();

    // get default beam mode -- based on time signature preferences
    const Groups& group = chord->staff()->group(chord->measure()->tick());
    BeamMode defaultBeamMode = group.endBeam(chord, prevChord);

    bool isManuallyBroken32 = level >= 1 && beamMode == BeamMode::BEGIN32;
    bool isManuallyBroken64 = level >= 2 && beamMode == BeamMode::BEGIN64;
    bool isDefaultBroken32 = beamMode == BeamMode::AUTO && level >= 1 && defaultBeamMode == BeamMode::BEGIN32;
    bool isDefaultBroken64 = beamMode == BeamMode::AUTO && level >= 2 && defaultBeamMode == BeamMode::BEGIN64;

    isBroken32 = isManuallyBroken32 || isDefaultBroken32;
    isBroken64 = isManuallyBroken64 || isDefaultBroken64;

    // deal with beam-embedded triplets by breaking beams as if they are their underlying durations
    // note that we use max(hooks, 1) here because otherwise we'd end up breaking the main (level 0) beam for
    // tuplets that take up non-beamed amounts of space (eg. 16th note quintuplets)
    if (level > 0 && prevChord && chord->beamMode() == BeamMode::AUTO) {
        if (chord->tuplet() && chord->tuplet() != prevChord->tuplet()) {
            // this cr starts a tuplet
            int beams = std::max(TDuration(chord->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken32 = level == 1;
                isBroken64 = level >= 2;
            }
        } else if (prevChord->tuplet() && prevChord->tuplet() != chord->tuplet()) {
            // this is a non-tuplet cr that is first after a tuplet
            int beams = std::max(TDuration(prevChord->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken32 = level == 1;
                isBroken64 = level >= 2;
            }
        }
    }
}

void Beam::createBeamSegments(const std::vector<ChordRest*>& chordRests)
{
    DeleteAll(_beamSegments);
    _beamSegments.clear();

    bool levelHasBeam = false;
    int level = 0;
    do {
        levelHasBeam = false;
        ChordRest* startCr = nullptr;
        ChordRest* endCr = nullptr;
        bool breakBeam = false;
        bool previousBreak32 = false;
        bool previousBreak64 = false;

        for (size_t i = 0; i < chordRests.size(); i++) {
            ChordRest* chordRest = chordRests[i];
            ChordRest* prevChordRest = i < 1 ? nullptr : chordRests[i - 1];

            if (level < chordRest->beams()) {
                levelHasBeam = true;
            }
            bool isBroken32 = false;
            bool isBroken64 = false;
            // updates isBroken32 and isBroken64
            calcBeamBreaks(chordRest, prevChordRest, level, isBroken32, isBroken64);
            breakBeam = isBroken32 || isBroken64;

            if (level < chordRest->beams() && !breakBeam) {
                endCr = chordRest;
                if (!startCr) {
                    startCr = chordRest;
                }
            } else {
                if (startCr && endCr) {
                    if (startCr == endCr && startCr->isChord()) {
                        bool isBeamletBefore = calcIsBeamletBefore(toChord(
                                                                       startCr), static_cast<int>(i) - 1, level, previousBreak32,
                                                                   previousBreak64);
                        createBeamletSegment(toChord(startCr), isBeamletBefore, level);
                    } else {
                        createBeamSegment(startCr, endCr, level);
                    }
                }
                bool setCr = chordRest && chordRest->isChord() && breakBeam && level < chordRest->beams();
                startCr = setCr ? chordRest : nullptr;
                endCr = setCr ? chordRest : nullptr;
            }
            previousBreak32 = isBroken32;
            previousBreak64 = isBroken64;
        }

        // if the beam ends on the last chord
        if (startCr && (endCr || breakBeam)) {
            if ((startCr == endCr || !endCr) && startCr->isChord()) {
                // this chord is either the last chord, or the first (followed by a rest)
                bool isBefore = !(startCr == chordRests.front());
                createBeamletSegment(toChord(startCr), isBefore, level);
            } else {
                createBeamSegment(startCr, endCr, level);
            }
        }
        level++;
    } while (levelHasBeam);
}

//---------------------------------------------------------
//   layout2
//---------------------------------------------------------

void Beam::layout2(const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag)
{
    _layoutInfo = BeamTremoloLayout(this);
    Chord* startChord = nullptr;
    Chord* endChord = nullptr;
    if (chordRests.empty()) {
        return;
    }
    for (auto chordRest : chordRests) {
        if (chordRest->isChord()) {
            if (!startChord) {
                startChord = toChord(chordRest);
                endChord = startChord;
            } else {
                endChord = toChord(chordRest);
            }
            toChord(chordRest)->layoutStem();
        }
    }
    if (!startChord) {
        // we were passed a vector of only rests. we don't support beams across only rests
        // this beam will be deleted in LayoutBeams
        return;
    }

    _beamSpacing = score()->styleB(Sid::useWideBeams) ? 4 : 3;
    _beamDist = (_beamSpacing / 4.0) * spatium() * mag();
    _beamWidth = point(score()->styleS(Sid::beamWidth)) * mag();

    _startAnchor = _layoutInfo.chordBeamAnchor(startChord, BeamTremoloLayout::ChordBeamAnchorType::Start);
    _endAnchor = _layoutInfo.chordBeamAnchor(endChord, BeamTremoloLayout::ChordBeamAnchorType::End);

    if (_isGrace) {
        _beamDist *= score()->styleD(Sid::graceNoteMag);
        _beamWidth *= score()->styleD(Sid::graceNoteMag);
    }

    int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    if (_userModified[fragmentIndex]) {
        _layoutInfo = BeamTremoloLayout(this);
        double startY = fragments[frag]->py1[fragmentIndex];
        double endY = fragments[frag]->py2[fragmentIndex];
        if (score()->styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        startY += pagePos().y();
        endY += pagePos().y();
        _startAnchor.setY(startY);
        _endAnchor.setY(endY);
        _layoutInfo.setAnchors(_startAnchor, _endAnchor);
        _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
        createBeamSegments(chordRests);
        setTremAnchors();
        return;
    }

    // anchor represents the middle of the beam, not the tip of the stem
    // location depends on _isBesideTabStaff

    if (!_isBesideTabStaff) {
        _layoutInfo = BeamTremoloLayout(this);
        _layoutInfo.calculateAnchors(chordRests, _notes);
        _startAnchor = _layoutInfo.startAnchor();
        _endAnchor = _layoutInfo.endAnchor();
        _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
        _beamDist = _layoutInfo.beamDist();
    } else {
        _slope = 0;
        Chord* startChord = nullptr;
        for (ChordRest* cr : chordRests) {
            if (cr->isChord()) {
                startChord = toChord(cr);
                break;
            }
        }
        _layoutInfo = BeamTremoloLayout(this);
        double x1 = _layoutInfo.chordBeamAnchorX(chordRests.front(), BeamTremoloLayout::ChordBeamAnchorType::Start);
        double x2 = _layoutInfo.chordBeamAnchorX(chordRests.back(), BeamTremoloLayout::ChordBeamAnchorType::End);
        double y = _layoutInfo.chordBeamAnchorY(startChord);
        _startAnchor = PointF(x1, y);
        _endAnchor = PointF(x2, y);
        _layoutInfo.setAnchors(_startAnchor, _endAnchor);
        _beamWidth = _layoutInfo.beamWidth();
    }

    fragments[frag]->py1[fragmentIndex] = _startAnchor.y() - pagePos().y();
    fragments[frag]->py2[fragmentIndex] = _endAnchor.y() - pagePos().y();

    createBeamSegments(chordRests);
    setTremAnchors();
}

bool Beam::layout2Cross(const std::vector<ChordRest*>& chordRests, int frag)
{
    int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    ChordRest* startCr = _elements.front();
    ChordRest* endCr = _elements.back();

    const double quarterSpace = spatium() / 4;
    // imagine a line of beamed notes all in a row on the same staff. the first and last of those
    // are the 'outside' notes, and the slant of the beam is going to be affected by the 'middle' notes
    // between them.
    // we have to keep track of this for both staves.
    Chord* topFirst = nullptr;
    Chord* topLast = nullptr;
    Chord* bottomFirst = nullptr;
    Chord* bottomLast = nullptr;
    int maxMiddleTopLine = std::numeric_limits<int>::min(); // lowest note in the top staff
    int minMiddleBottomLine = std::numeric_limits<int>::max(); // highest note in the bottom staff
    int prevTopLine = maxMiddleTopLine; // previous note's line position (top)
    int prevBottomLine = minMiddleBottomLine; // previous note's line position (bottom)
    // if the immediate neighbor of one of the two 'outside' notes on either the top or bottom
    // are the same as that outside note, we need to record it so that we can add a 1/4 space slant.
    bool secondTopIsSame = false;
    bool secondBottomIsSame = false;
    bool penultimateTopIsSame = false;
    bool penultimateBottomIsSame = false;
    double maxY = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::min();
    int otherStaff = 0;
    // recompute _minMove and _maxMove as they may have shifted since last layout
    _minMove = std::numeric_limits<int>::max();
    _maxMove = std::numeric_limits<int>::min();
    for (ChordRest* c : chordRests) {
        IF_ASSERT_FAILED(c) {
            continue;
        }
        int staffMove = c->staffMove();
        _minMove = std::min(_minMove, staffMove);
        _maxMove = std::max(_maxMove, staffMove);

        if (staffMove != 0) {
            otherStaff = staffMove;
        }
    }
    if (otherStaff == 0 || _minMove == _maxMove) {
        return false;
    }
    // Find the notes on the top and bottom of staves
    //
    bool checkNextTop = false;
    bool checkNextBottom = false;
    for (ChordRest* cr : chordRests) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        if ((c->staffMove() == otherStaff && otherStaff > 0) || (c->staffMove() != otherStaff && otherStaff < 0)) {
            // this chord is on the bottom staff
            if (penultimateBottomIsSame) {
                // the chord we took as the penultimate bottom note wasn't.
                // so treat it properly as a middle note
                minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                penultimateBottomIsSame = false;
            }
            checkNextTop = false; // we are no longer looking for the second note in the top
                                  // staff being the same as the first--this note is on the bottom.
            if (!bottomFirst) {
                bottomFirst = c;
                checkNextBottom = true; // this was the first bottom note, so check for second next time
            } else {
                penultimateBottomIsSame = prevBottomLine == c->line();
                if (!penultimateBottomIsSame) {
                    minMiddleBottomLine = std::min(minMiddleBottomLine, prevBottomLine);
                }
                if (checkNextBottom) {
                    // this is the second bottom note, so we should see if this one is same line as first
                    secondBottomIsSame = c->line() == bottomFirst->line();
                    checkNextBottom = false;
                } else {
                    prevBottomLine = c->line();
                }
                bottomLast = c;
            }
            maxY = std::min(maxY, _layoutInfo.chordBeamAnchorY(toChord(c)));
        } else {
            // this chord is on the top staff
            if (penultimateTopIsSame) {
                // the chord we took as the penultimate top note wasn't.
                // so treat it properly as a middle note
                maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                penultimateTopIsSame = false;
            }
            checkNextBottom = false; // no longer looking for a bottom second note since this is on top
            if (!topFirst) {
                topFirst = c;
                checkNextTop = true;
            } else {
                penultimateTopIsSame = prevTopLine == c->line();
                if (!penultimateTopIsSame) {
                    maxMiddleTopLine = std::max(maxMiddleTopLine, prevTopLine);
                }
                if (checkNextTop) {
                    secondTopIsSame = c->line() == topFirst->line();
                    checkNextTop = false;
                } else {
                    prevTopLine = c->line();
                }
                topLast = c;
            }
            minY = std::max(minY, _layoutInfo.chordBeamAnchorY(toChord(c)));
        }
    }
    _startAnchor.ry() = (maxY + minY) / 2;
    _endAnchor.ry() = (maxY + minY) / 2;
    _slope = 0;

    if (!noSlope()) {
        int topFirstLine = topFirst ? topFirst->downNote()->line() : 0;
        int topLastLine = topLast ? topLast->downNote()->line() : 0;
        int bottomFirstLine = bottomFirst ? bottomFirst->upNote()->line() : 0;
        int bottomLastLine = bottomLast ? bottomLast->upNote()->line() : 0;
        bool constrainTopToQuarter = false;
        bool constrainBottomToQuarter = false;
        if ((topFirstLine > topLastLine && secondTopIsSame)
            || (topFirstLine < topLastLine && penultimateTopIsSame)) {
            constrainTopToQuarter = true;
        }
        if ((bottomFirstLine < bottomLastLine && secondBottomIsSame)
            || (bottomFirstLine > bottomLastLine && penultimateBottomIsSame)) {
            constrainBottomToQuarter = true;
        }
        if (!topLast && !bottomLast && topFirst && bottomFirst) {
            // if there are only two notes, one on each staff, special case
            // take max slope into account
            double yFirst, yLast;
            if (topFirst->tick() < bottomFirst->tick()) {
                yFirst = topFirst->stemPos().y();
                yLast = bottomFirst->stemPos().y();
            } else {
                yFirst = bottomFirst->stemPos().y();
                yLast = topFirst->stemPos().y();
            }
            int desiredSlant = round((yFirst - yLast) / spatium());
            int slant = std::min(std::abs(desiredSlant), _layoutInfo.getMaxSlope());
            slant *= (desiredSlant < 0) ? -quarterSpace : quarterSpace;
            _startAnchor.ry() += (slant / 2);
            _endAnchor.ry() -= (slant / 2);
        } else if (!topLast || !bottomLast) {
            // otherwise, if there is only one note on one of the staves, use slope from other staff
            int startNote = 0;
            int endNote = 0;
            bool forceHoriz = false;
            if (!topLast) {
                startNote = bottomFirstLine;
                endNote = bottomLastLine;
                if (minMiddleBottomLine <= std::min(startNote, endNote)) {
                    // there is a note closer to the beam than the start and end notes
                    // we force horizontal beam here.
                    forceHoriz = true;
                }
            } else if (!bottomLast) {
                startNote = topFirstLine;
                endNote = topLastLine;
                if (maxMiddleTopLine >= std::max(startNote, endNote)) {
                    // same as above, for the top staff
                    // force horizontal.
                    forceHoriz = true;
                }
            }

            if (!forceHoriz) {
                int slant = startNote - endNote;
                slant = std::min(std::abs(slant), _layoutInfo.getMaxSlope());
                if ((!bottomLast && constrainTopToQuarter) || (!topLast && constrainBottomToQuarter)) {
                    slant = 1;
                }
                double slope = slant * (startNote > endNote ? quarterSpace : -quarterSpace);
                _startAnchor.ry() += (slope / 2);
                _endAnchor.ry() -= (slope / 2);
            } // otherwise, do nothing, beam is already horizontal.
        } else {
            // otherwise, there are at least two notes on each staff
            // (that is, topLast and bottomLast are both set)
            bool forceHoriz = false;
            if (topFirstLine == topLastLine || bottomFirstLine == bottomLastLine) {
                // if outside notes on top or bottom staff are on the same staff line, slope = 0
                // no further adjustment needed, the beam is already well-placed and horizontal
                forceHoriz = true;
            }
            // otherwise, we have to compare the slopes from the top staff and bottom staff.
            int topSlant = topFirstLine - topLastLine;
            if (constrainTopToQuarter && topSlant != 0) {
                topSlant = topFirstLine < topLastLine ? -1 : 1;
            }
            int bottomSlant = bottomFirstLine - bottomLastLine;
            if (constrainBottomToQuarter && bottomSlant != 0) {
                bottomSlant = bottomFirstLine < bottomLastLine ? -1 : 1;
            }
            if ((maxMiddleTopLine >= std::max(topFirstLine, topLastLine)
                 || (minMiddleBottomLine <= std::min(bottomFirstLine, bottomLastLine)))) {
                forceHoriz = true;
            }
            if (topSlant == 0 || bottomSlant == 0 || forceHoriz) {
                // if one of the slants is 0, the whole slant is zero
            } else if ((topSlant < 0 && bottomSlant < 0) || (topSlant > 0 && bottomSlant > 0)) {
                int slant = (abs(topSlant) < abs(bottomSlant)) ? topSlant : bottomSlant;
                slant = std::min(std::abs(slant), _layoutInfo.getMaxSlope());
                double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                _startAnchor.ry() += (slope / 2);
                _endAnchor.ry() -= (slope / 2);
            } else {
                // if the two slopes are in opposite directions, flat!
                // nothing needs to be done, the beam is already horizontal and placed nicely
            }
        }
        _startAnchor.setX(_layoutInfo.chordBeamAnchorX(startCr, BeamTremoloLayout::ChordBeamAnchorType::Start));
        _endAnchor.setX(_layoutInfo.chordBeamAnchorX(endCr, BeamTremoloLayout::ChordBeamAnchorType::End));
        _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
    }
    fragments[frag]->py1[fragmentIndex] = _startAnchor.y() - pagePos().y();
    fragments[frag]->py2[fragmentIndex] = _endAnchor.y() - pagePos().y();
    createBeamSegments(chordRests);
    return true;
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Beam::spatiumChanged(double oldValue, double newValue)
{
    int idx = (!_up) ? 0 : 1;
    if (_userModified[idx]) {
        double diff = newValue / oldValue;
        for (BeamFragment* f : fragments) {
            f->py1[idx] = f->py1[idx] * diff;
            f->py2[idx] = f->py2[idx] * diff;
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Beam::write(XmlWriter& xml) const
{
    if (_elements.empty()) {
        return;
    }
    xml.startElement(this);
    EngravingItem::writeProperties(xml);

    writeProperty(xml, Pid::STEM_DIRECTION);
    writeProperty(xml, Pid::BEAM_NO_SLOPE);
    writeProperty(xml, Pid::GROW_LEFT);
    writeProperty(xml, Pid::GROW_RIGHT);

    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    if (_userModified[idx]) {
        double _spatium = spatium();
        for (BeamFragment* f : fragments) {
            xml.startElement("Fragment");
            xml.tag("y1", f->py1[idx] / _spatium);
            xml.tag("y2", f->py2[idx] / _spatium);
            xml.endElement();
        }
    }

    // this info is used for regression testing
    // l1/l2 is the beam position of the layout engine
    if (MScore::testMode) {
        double spatium8 = spatium() * .125;
        for (BeamFragment* f : fragments) {
            xml.tag("l1", int(lrint(f->py1[idx] / spatium8)));
            xml.tag("l2", int(lrint(f->py2[idx] / spatium8)));
        }
    }

    xml.endElement();
}

//---------------------------------------------------------
//   BeamEditData
//---------------------------------------------------------

class BeamEditData : public ElementEditData
{
    OBJECT_ALLOCATOR(engraving, BeamEditData)
public:
    int editFragment;
    virtual EditDataType type() override { return EditDataType::BeamEditData; }
};

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Beam::editDrag(EditData& ed)
{
    int idx  = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double dy = ed.delta.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = fragments[bed->editFragment];
    double y1 = f->py1[idx];
    double y2 = f->py2[idx];

    if (ed.curGrip == Grip::MIDDLE || noSlope()) {
        y1 += dy;
        y2 += dy;
    } else if (ed.curGrip == Grip::START) {
        y1 += dy;
    } else if (ed.curGrip == Grip::END) {
        y2 += dy;
    }

    double _spatium = spatium();
    // Because of the logic in Beam::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Beam::gripsPositions(const EditData& ed) const
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = fragments[bed->editFragment];

    ChordRest* c1 = nullptr;
    ChordRest* c2 = nullptr;
    size_t n = _elements.size();

    if (n == 0) {
        return std::vector<PointF>();
    }

    for (size_t i = 0; i < n; ++i) {
        if (_elements[i]->isChordRest()) {
            c1 = toChordRest(_elements[i]);
            break;
        }
    }
    if (!c1) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }
    for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
        if (_elements[i]->isChordRest()) {
            c2 = toChordRest(_elements[i]);
            break;
        }
    }
    if (!c2) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }

    int y = pagePos().y();
    double beamStartX = _startAnchor.x() + (system() ? system()->x() : 0);
    double beamEndX = _endAnchor.x() + (system() ? system()->x() : 0);
    double middleX = (beamStartX + beamEndX) / 2;
    double middleY = (f->py1[idx] + y + f->py2[idx] + y) / 2;

    return {
        PointF(beamStartX, f->py1[idx] + y),
        PointF(beamEndX, f->py2[idx] + y),
        PointF(middleX, middleY)
    };
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setBeamDirection(DirectionV d)
{
    if (_direction == d || _cross) {
        return;
    }

    _direction = d;

    if (d != DirectionV::AUTO) {
        _up = d == DirectionV::UP;
    }

    for (ChordRest* e : elements()) {
        if (e->isChord()) {
            toChord(e)->setStemDirection(d);
        }
    }
}

void Beam::setAsFeathered(const bool slower)
{
    if (slower) {
        undoChangeProperty(Pid::GROW_LEFT, 1.0);
        undoChangeProperty(Pid::GROW_RIGHT, 0.0);
    } else {
        undoChangeProperty(Pid::GROW_LEFT, 0.0);
        undoChangeProperty(Pid::GROW_RIGHT, 1.0);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Beam::reset()
{
    if (growLeft() != 1.0) {
        undoChangeProperty(Pid::GROW_LEFT, 1.0);
    }
    if (growRight() != 1.0) {
        undoChangeProperty(Pid::GROW_RIGHT, 1.0);
    }
    if (userModified()) {
        undoChangeProperty(Pid::BEAM_POS, PropertyValue::fromValue(beamPos()));
        undoChangeProperty(Pid::USER_MODIFIED, false);
    }
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    resetProperty(Pid::BEAM_NO_SLOPE);
    setGenerated(true);
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Beam::startEdit(EditData& ed)
{
    initBeamEditData(ed);
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Beam::endEdit(EditData& ed)
{
    EngravingItem::endEdit(ed);
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Beam::triggerLayout() const
{
    if (!_elements.empty()) {
        _elements.front()->triggerLayout();
        _elements.back()->triggerLayout();
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Beam::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;

    if (e->isActionIcon()) {
        ActionIconType type = toActionIcon(e)->actionType();
        return type == ActionIconType::BEAM_FEATHERED_DECELERATE
               || type == ActionIconType::BEAM_FEATHERED_ACCELERATE;
    }

    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Beam::drop(EditData& data)
{
    if (!data.dropElement->isActionIcon()) {
        return nullptr;
    }

    ActionIcon* e = toActionIcon(data.dropElement);

    if (e->actionType() == ActionIconType::BEAM_FEATHERED_DECELERATE) {
        setAsFeathered(true /*slower*/);
    } else if (e->actionType() == ActionIconType::BEAM_FEATHERED_ACCELERATE) {
        setAsFeathered(false /*slower*/);
    }

    return nullptr;
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF Beam::beamPos() const
{
    if (fragments.empty()) {
        return PairF(0.0, 0.0);
    }
    BeamFragment* f = fragments.back();
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double _spatium = spatium();
    return PairF(f->py1[idx] / _spatium, f->py2[idx] / _spatium);
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Beam::setBeamPos(const PairF& bp)
{
    if (fragments.empty()) {
        fragments.push_back(new BeamFragment);
    }
    BeamFragment* f = fragments.back();
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _userModified[idx] = true;
    setGenerated(false);

    double _spatium = spatium();
    if (noSlope()) {
        f->py1[idx] = f->py2[idx] = (bp.first + bp.second) * 0.5 * _spatium;
    } else {
        f->py1[idx] = bp.first * _spatium;
        f->py2[idx] = bp.second * _spatium;
    }
}

void Beam::setNoSlope(bool b)
{
    _noSlope = b;

    // Make flat if usermodified
    if (_noSlope) {
        int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
        if (_userModified[idx]) {
            BeamFragment* f = fragments.back();
            f->py1[idx] = f->py2[idx] = (f->py1[idx] + f->py2[idx]) * 0.5;
        }
    }
}

//---------------------------------------------------------
//   userModified
//---------------------------------------------------------

bool Beam::userModified() const
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    return _userModified[idx];
}

//---------------------------------------------------------
//   setUserModified
//---------------------------------------------------------

void Beam::setUserModified(bool val)
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _userModified[idx] = val;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Beam::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::STEM_DIRECTION: return beamDirection();
    case Pid::GROW_LEFT:      return growLeft();
    case Pid::GROW_RIGHT:     return growRight();
    case Pid::USER_MODIFIED:  return userModified();
    case Pid::BEAM_POS:       return PropertyValue::fromValue(beamPos());
    case Pid::BEAM_NO_SLOPE:  return noSlope();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Beam::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::STEM_DIRECTION:
        setBeamDirection(v.value<DirectionV>());
        break;
    case Pid::GROW_LEFT:
        setGrowLeft(v.toDouble());
        break;
    case Pid::GROW_RIGHT:
        setGrowRight(v.toDouble());
        break;
    case Pid::USER_MODIFIED:
        setUserModified(v.toBool());
        break;
    case Pid::BEAM_POS:
        if (userModified()) {
            setBeamPos(v.value<PairF>());
        }
        break;
    case Pid::BEAM_NO_SLOPE:
        setNoSlope(v.toBool());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Beam::propertyDefault(Pid id) const
{
    switch (id) {
//            case Pid::SUB_STYLE:      return int(TextStyleName::BEAM);
    case Pid::STEM_DIRECTION: return DirectionV::AUTO;
    case Pid::GROW_LEFT:      return 1.0;
    case Pid::GROW_RIGHT:     return 1.0;
    case Pid::USER_MODIFIED:  return false;
    case Pid::BEAM_POS:       return PropertyValue::fromValue(beamPos());
    default:                  return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   addSkyline
//    add beam shape to skyline
//---------------------------------------------------------

void Beam::addSkyline(Skyline& sk)
{
    if (_beamSegments.empty() || !addToSkyline()) {
        return;
    }
    double lw2 = point(score()->styleS(Sid::beamWidth)) * .5 * mag();
    const LineF bs = _beamSegments.front()->line;
    double d  = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
    if (_beamSegments.size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww      = lw2 / sin(M_PI_2 - atan(d));
    double _spatium = spatium();

    for (const BeamSegment* beamSegment : _beamSegments) {
        double x = beamSegment->line.x1();
        double y = beamSegment->line.y1();
        double w = beamSegment->line.x2() - x;
        int n   = (d < 0.01) ? 1 : int(ceil(w / _spatium));

        double s = (beamSegment->line.y2() - y) / w;
        w /= n;
        for (int i = 1; i <= n; ++i) {
            double y2 = y + w * s;
            double yn, ys;
            if (y2 > y) {
                yn = y;
                ys = y2;
            } else {
                yn = y2;
                ys = y;
            }
            sk.north().add(x, yn - ww, w);
            sk.south().add(x, ys + ww, w);
            x += w;
            y = y2;
        }
    }
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Beam::tick() const
{
    return _elements.empty() ? Fraction(0, 1) : _elements.front()->segment()->tick();
}

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction Beam::rtick() const
{
    return _elements.empty() ? Fraction(0, 1) : _elements.front()->segment()->rtick();
}

//---------------------------------------------------------
//   ticks
//    calculate the ticks of all chords and rests connected by the beam
//---------------------------------------------------------

Fraction Beam::ticks() const
{
    Fraction ticks = Fraction(0, 1);
    for (ChordRest* cr : _elements) {
        ticks += cr->actualTicks();
    }
    return ticks;
}

//---------------------------------------------------------
//   actionIconTypeForBeamMode
//---------------------------------------------------------

ActionIconType Beam::actionIconTypeForBeamMode(BeamMode mode)
{
    switch (mode) {
    case BeamMode::AUTO:
        return ActionIconType::BEAM_AUTO;
    case BeamMode::NONE:
        return ActionIconType::BEAM_NONE;
    case BeamMode::BEGIN:
        return ActionIconType::BEAM_BREAK_LEFT;
    case BeamMode::BEGIN32:
        return ActionIconType::BEAM_BREAK_INNER_8TH;
    case BeamMode::BEGIN64:
        return ActionIconType::BEAM_BREAK_INNER_16TH;
    case BeamMode::MID:
        return ActionIconType::BEAM_JOIN;
    default:
        break;
    }
    return ActionIconType::UNDEFINED;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Beam::drag(EditData& ed)
{
    int idx  = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double dy = ed.pos.y() - ed.lastPos.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = fragments[bed->editFragment];

    double y1 = f->py1[idx];
    double y2 = f->py2[idx];

    y1 += dy;
    y2 += dy;

    double _spatium = spatium();
    // Because of the logic in Beam::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();

    return canvasBoundingRect();
}

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------
bool Beam::isMovable() const
{
    return true;
}

//---------------------------------------------------------
//   initBeamEditData
//---------------------------------------------------------
void Beam::initBeamEditData(EditData& ed)
{
    std::shared_ptr<BeamEditData> bed = std::make_shared<BeamEditData>();
    bed->e    = this;
    bed->editFragment = 0;
    ed.addData(bed);

    PointF pt(ed.normalizedStartMove - pagePos());
    double ydiff = 100000000.0;
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    int i = 0;
    for (BeamFragment* f : fragments) {
        double d = fabs(f->py1[idx] - pt.y());
        if (d < ydiff) {
            ydiff = d;
            bed->editFragment = i;
        }
        ++i;
    }
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------
void Beam::startDrag(EditData& editData)
{
    initBeamEditData(editData);
}
}

//---------------------------------------------------------
//   containsChord
//---------------------------------------------------------
bool Beam::hasAllRests()
{
    for (ChordRest* cr : _elements) {
        if (cr && cr->isChord()) {
            return false;
        }
    }
    return true;
}

Shape Beam::shape() const
{
    Shape shape;
    for (BeamSegment* beamSegment : _beamSegments) {
        shape.add(beamSegment->shape());
    }
    return shape;
}

//-------------------------------------------------------
// BEAM SEGMENT CLASS
//-------------------------------------------------------

Shape BeamSegment::shape() const
{
    Shape shape;
    PointF startPoint = line.p1();
    PointF endPoint = line.p2();
    double _beamWidth = parentElement->isBeam() ? toBeam(parentElement)->_beamWidth : toTremolo(parentElement)->beamWidth();
    // This is the case of right-beamlets
    if (startPoint.x() > endPoint.x()) {
        std::swap(startPoint, endPoint);
    }
    double beamHorizontalLength = endPoint.x() - startPoint.x();
    // If beam is horizontal, one rectangle is enough
    if (RealIsEqual(startPoint.y(), endPoint.y())) {
        RectF rect(startPoint.x(), startPoint.y(), beamHorizontalLength, _beamWidth / 2);
        rect.adjust(0.0, -_beamWidth / 2, 0.0, 0.0);
        shape.add(rect, parentElement);
        return shape;
    }
    // If not, break the beam shape into multiple rectangles
    double beamHeightDiff = endPoint.y() - startPoint.y();
    int subBoxesCount = floor(beamHorizontalLength / parentElement->spatium());
    double horizontalStep = beamHorizontalLength / subBoxesCount;
    double verticalStep = beamHeightDiff / subBoxesCount;
    std::vector<PointF> pointsOnBeamLine;
    pointsOnBeamLine.push_back(startPoint);
    for (int i = 0; i < subBoxesCount - 1; ++i) {
        PointF nextPoint = pointsOnBeamLine.back() + PointF(horizontalStep, verticalStep);
        pointsOnBeamLine.push_back(nextPoint);
    }
    for (PointF point : pointsOnBeamLine) {
        RectF rect(point.x(), point.y(), horizontalStep, _beamWidth / 2);
        rect.adjust(0.0, -_beamWidth / 2, 0.0, 0.0);
        shape.add(rect, parentElement);
    }
    return shape;
}
