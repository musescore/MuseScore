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

#include "draw/brush.h"
#include "style/style.h"
#include "rw/xml.h"
#include "realfn.h"

#include "segment.h"
#include "score.h"
#include "chord.h"
#include "sig.h"
#include "note.h"
#include "tuplet.h"
#include "system.h"
#include "tremolo.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "hook.h"
#include "mscore.h"
#include "actionicon.h"
#include "stemslash.h"
#include "groups.h"
#include "spanner.h"

#include "layout/layoutbeams.h"
#include "layout/layoutchords.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle beamStyle {
    { Sid::beamNoSlope,                        Pid::BEAM_NO_SLOPE },
};

//---------------------------------------------------------
//   BeamFragment
//    position of primary beam
//    idx 0 - DirectionV::AUTO or DirectionV::DOWN
//        1 - DirectionV::UP
//---------------------------------------------------------

struct BeamFragment {
    double py1[2];
    double py2[2];
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
    _distribute      = b._distribute;
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
    _maxDuration      = b._maxDuration;
    _slope            = b._slope;
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
    TRACE_OBJ_DRAW;
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

    _maxDuration.setType(DurationType::V_INVALID);

    // TAB's with stem beside staves have special layout
    bool isTabStaff = staff()->isTabStaff(Fraction(0, 1)) && !staff()->staffType(Fraction(0, 1))->stemThrough();
    if (isTabStaff) {
        _up = !staff()->staffType(Fraction(0, 1))->stemsDown();
        _slope = 0.0;
        _cross = false;
        _minMove = 0;
        _maxMove = 0;
        for (ChordRest* cr : _elements) {
            if (cr->isChord()) {
                if (!_maxDuration.isValid() || (_maxDuration < cr->durationType())) {
                    _maxDuration = cr->durationType();
                }
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
            for (ChordRest* cr :_elements) {
                if (cr->isChord()) {
                    _up = toChord(cr)->up();
                    break;
                }
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
        if (!_maxDuration.isValid() || (_maxDuration < cr->durationType())) {
            _maxDuration = cr->durationType();
        }
    }

    std::sort(_notes.begin(), _notes.end());
    setMag(mag);

    //
    // determine beam stem direction
    //
    if (_direction != DirectionV::AUTO) {
        _up = _direction == DirectionV::UP;
    } else if (_maxMove > 0) {
        _up = false;
    } else if (_minMove < 0) {
        _up = true;
    } else if (_isGrace) {
        _up = true;
    } else if (_notes.size()) {
        ChordRest* firstNote = _elements.front();
        Measure* measure = firstNote->measure();
        bool hasMultipleVoices = measure->hasVoices(firstNote->staffIdx(), tick(), ticks());
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

    ChordRest* firstNote = _elements.front();
    int middleStaffLine = firstNote->staffType()->middleLine();
    for (uint i = 0; i < _notes.size(); i++) {
        _notes[i] += middleStaffLine;
    }

    _cross = _minMove != _maxMove;
    if (_minMove == _maxMove && _minMove != 0) {
        setStaffIdx(staffIdx);
        _up = _maxMove > 0;
    } else if (_elements.size()) {
        setStaffIdx(_elements.at(0)->staffIdx());
    }

    _slope = 0.0;

    for (ChordRest* cr : _elements) {
        const bool staffMove = cr->isChord() ? toChord(cr)->staffMove() : false;
        if (!_cross || !staffMove) {
            if (cr->up() != _up) {
                if (cr->isChord()) {
                    Chord* c = toChord(cr);
                    c->setUp(_up != staffMove);
                    c->layoutStem();
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
    System* system = _elements.front()->measure()->system();
    setParent(system);

    std::vector<ChordRest*> crl;

    size_t n = 0;
    for (ChordRest* cr : _elements) {
        if (cr->measure()->system() != system) {
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

int Beam::getMiddleStaffLine(ChordRest* startChord, ChordRest* endChord, int staffLines) const
{
    bool useWideBeams = score()->styleB(Sid::useWideBeams);
    int startMiddleLine = Chord::minStaffOverlap(_up, staffLines, startChord->beams(), false, _beamSpacing / 4.0, useWideBeams, !_isGrace);
    int endMiddleLine = Chord::minStaffOverlap(_up, staffLines, endChord->beams(), false, _beamSpacing / 4.0, useWideBeams, !_isGrace);

    // offset middle line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (_up) {
        return std::min(startMiddleLine, endMiddleLine) + 1;
    }
    return std::max(startMiddleLine, endMiddleLine) - 1;
}

int Beam::computeDesiredSlant(int startNote, int endNote, int middleLine, int dictator, int pointer) const
{
    if (noSlope()) {
        return 0;
    }
    if (dictator == middleLine && pointer == middleLine) {
        return 0;
    }
    if (startNote == endNote) {
        return 0;
    }
    // if a note is more extreme than the endpoints, slope is 0
    // p.s. _notes is a sorted vector
    if (_notes.size() > 2) {
        if (_up) {
            int higherEnd = std::min(startNote, endNote);
            if (higherEnd > _notes[0]) {
                return 0;
            }
            if (higherEnd == _notes[0] && higherEnd >= _notes[1]) {
                if (higherEnd > _notes[1]) {
                    return 0;
                }
                size_t chordCount = _elements.size();
                if (chordCount >= 3 && _notes.size() >= 3) {
                    bool middleNoteHigherThanHigherEnd = higherEnd >= _notes[2];
                    if (middleNoteHigherThanHigherEnd) {
                        return 0;
                    }
                    bool secondNoteSameHeightAsHigherEnd = startNote < endNote && _elements[1]->isChord()
                                                           && toChord(_elements[1])->upLine() == higherEnd;
                    bool secondToLastNoteSameHeightAsHigherEnd = endNote < startNote && _elements[chordCount - 2]->isChord() && toChord(
                        _elements[chordCount - 2])->upLine() == higherEnd;
                    if (!(secondNoteSameHeightAsHigherEnd || secondToLastNoteSameHeightAsHigherEnd)) {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
        } else {
            int lowerEnd = std::max(startNote, endNote);
            if (lowerEnd < _notes[_notes.size() - 1]) {
                return 0;
            }
            if (lowerEnd == _notes[_notes.size() - 1] && lowerEnd <= _notes[_notes.size() - 2]) {
                if (lowerEnd < _notes[_notes.size() - 2]) {
                    return 0;
                }
                size_t chordCount = _elements.size();
                if (chordCount >= 3 && _notes.size() >= 3) {
                    bool middleNoteLowerThanLowerEnd = lowerEnd <= _notes[_notes.size() - 3];
                    if (middleNoteLowerThanLowerEnd) {
                        return 0;
                    }
                    bool secondNoteSameHeightAsLowerEnd = startNote > endNote && _elements[1]->isChord()
                                                          && toChord(_elements[1])->downLine() == lowerEnd;
                    bool secondToLastNoteSameHeightAsLowerEnd = endNote > startNote && _elements[chordCount - 2]->isChord() && toChord(
                        _elements[chordCount - 2])->downLine() == lowerEnd;
                    if (!(secondNoteSameHeightAsLowerEnd || secondToLastNoteSameHeightAsLowerEnd)) {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
        }
    }

    // calculate max slope based on distance between first and last chords
    int maxSlope = getMaxSlope();

    // calculate max slope based on note interval
    int interval = std::min(std::abs(endNote - startNote), (int)_maxSlopes.size() - 1);
    return std::min(maxSlope, _maxSlopes[interval]) * (_up ? 1 : -1);
}

int Beam::getMaxSlope() const
{
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i

    // calculate max slope based on distance between first and last chords
    double beamWidth = _elements[_elements.size() - 1]->stemPos().x() - _elements[0]->stemPos().x();
    beamWidth /= spatium();
    int maxSlope = _maxSlopes.back();
    if (beamWidth < 3.0) {
        maxSlope = _maxSlopes[0];
    } else if (beamWidth < 5.0) {
        maxSlope = _maxSlopes[1];
    } else if (beamWidth < 8.0) {
        maxSlope = _maxSlopes[2];
    } else if (beamWidth < 13.0) {
        maxSlope = _maxSlopes[3];
    } else if (beamWidth < 21.0) {
        maxSlope = _maxSlopes[4];
    } else if (beamWidth < 34.0) {
        maxSlope = _maxSlopes[5];
    } else {
        maxSlope = _maxSlopes[6];
    }

    return maxSlope;
}

int Beam::getBeamCount(const std::vector<ChordRest*> chordRests) const
{
    int beamCount = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && toChord(chordRest)->beams() > beamCount) {
            beamCount = toChord(chordRest)->beams();
        }
    }
    return beamCount;
}

double Beam::chordBeamAnchorX(const Chord* chord, ChordBeamAnchorType anchorType) const
{
    double stemPosX = chord->stemPosX() + chord->pagePos().x() - pagePos().x();

    if (!chord->stem()) {
        return stemPosX;
    }

    double stemWidth = chord->stem()->lineWidth().val() * chord->mag();

    switch (anchorType) {
    case ChordBeamAnchorType::Start:
        if (_tab) {
            return stemPosX - 0.5 * stemWidth;
        }

        if (chord->up()) {
            return stemPosX - stemWidth;
        }

        break;
    case ChordBeamAnchorType::Middle:
        if (_tab) {
            return stemPosX;
        }

        return chord->up() ? stemPosX - 0.5 * stemWidth : stemPosX + 0.5 * stemWidth;

    case ChordBeamAnchorType::End:
        if (_tab) {
            return stemPosX + 0.5 * stemWidth;
        }

        if (!chord->up()) {
            return stemPosX + stemWidth;
        }

        break;
    }

    return stemPosX;
}

double Beam::chordBeamAnchorY(const Chord* chord) const
{
    Note* note = chord->up() ? chord->downNote() : chord->upNote();
    PointF position = note->pagePos();

    int upValue = chord->up() ? -1 : 1;
    double beamOffset = _beamWidth / 2 * upValue;

    if (_isBesideTabStaff) {
        double stemLength = _tab->chordStemLength(chord);
        double y = _tab->chordRestStemPosY(chord) + stemLength;
        y *= spatium();
        y -= beamOffset;
        return y + chord->pagePos().y();
    }

    return position.y() + (chord->defaultStemLength() * upValue) - beamOffset;
}

PointF Beam::chordBeamAnchor(const Chord* chord, ChordBeamAnchorType anchorType) const
{
    return PointF(chordBeamAnchorX(chord, anchorType), chordBeamAnchorY(chord));
}

void Beam::createBeamSegment(Chord* startChord, Chord* endChord, int level)
{
    const bool isFirstSubgroup = startChord == toChord(_elements.front());
    const bool isLastSubgroup = endChord == toChord(_elements.back());
    const bool firstUp = startChord->up();
    const bool lastUp = endChord->up();
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
                if (!cr->isChord() || cr->tick() < startChord->tick()) {
                    continue;
                }
                if (cr->tick() > endChord->tick()) {
                    break;
                }

                ++(toChord(cr)->up() ? upStems : downStems);

                if (cr == endChord) {
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

    const double startX = chordBeamAnchorX(startChord, ChordBeamAnchorType::Start);
    const double endX = chordBeamAnchorX(endChord, ChordBeamAnchorType::End);

    double startY = _slope * (startX - _startAnchor.x()) + _startAnchor.y() - pagePos().y();
    double endY = _slope * (endX - _startAnchor.x()) + _startAnchor.y() - pagePos().y();

    int beamsBelow = 0; // how many beams below level 0?
    int beamsAbove = 0; // how many beams above level 0?

    // avoid adjusting for beams on opposite side of level 0
    if (level != 0) {
        for (const BeamSegment* beam : _beamSegments) {
            if (beam->level == 0 || beam->line.x2() < startX || beam->line.x1() > endX) {
                continue;
            }

            ++(beam->above ? beamsAbove : beamsBelow);
        }

        const int upValue = overallUp ? -1 : 1;
        const int extraBeamAdjust = overallUp ? beamsAbove : beamsBelow;
        const double verticalOffset = _beamDist * (level - extraBeamAdjust) * upValue;

        if (qFuzzyCompare(_grow1, _grow2)) {
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

    BeamSegment* b = new BeamSegment();
    b->above = !overallUp;
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
    _beamSegments.push_back(b);

    if (level > 0) {
        ++(b->above ? beamsAbove : beamsBelow);
    }

    // extend stems properly
    for (ChordRest* cr : _elements) {
        if (!cr->isChord() || cr->tick() < startChord->tick()) {
            continue;
        }
        if (cr->tick() > endChord->tick()) {
            break;
        }

        Chord* chord = toChord(cr);
        double addition = 0.0;

        if (level > 0) {
            double grow = _grow1;
            if (!qFuzzyCompare(_grow1, _grow2)) {
                double anchorX = chordBeamAnchorX(chord, ChordBeamAnchorType::Middle);
                double proportionAlongX = (anchorX - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());
                grow = proportionAlongX * (_grow2 - _grow1) + _grow1;
            }

            int extraBeamAdjust = cr->up() ? beamsBelow : beamsAbove;
            addition = grow * (level - extraBeamAdjust) * _beamDist;
        }

        extendStem(chord, addition);

        if (chord == endChord) {
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
    if (nextChordRest->isChord()) {
        bool nextBreak32 = false;
        bool nextBreak64 = false;
        calcBeamBreaks(nextChordRest, currChordRest, level, nextBreak32, nextBreak64);
        if ((nextBreak32 && level >= 1) || (nextBreak64 && level >= 2)) {
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

void Beam::createBeamletSegment(Chord* chord, bool isBefore, int level)
{
    const double startX = chordBeamAnchorX(chord, isBefore ? ChordBeamAnchorType::End : ChordBeamAnchorType::Start);

    const double beamletLength = score()->styleMM(Sid::beamMinLen).val()
                                 * mag()
                                 * chord->staff()->staffMag(chord);

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

        if (chord->up() == beam->above) {
            extraBeamAdjust++;
        }
    }

    const int upValue = chord->up() ? -1 : 1;
    const double verticalOffset = _beamDist * (level - extraBeamAdjust) * upValue;

    if (qFuzzyCompare(_grow1, _grow2)) {
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

    BeamSegment* b = new BeamSegment();
    b->above = !chord->up();
    b->level = level;
    b->line = LineF(startX, startY, endX, endY);
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
}

void Beam::createBeamSegments(const std::vector<ChordRest*>& chordRests)
{
    DeleteAll(_beamSegments);
    _beamSegments.clear();

    bool levelHasBeam = false;
    int level = 0;
    do {
        levelHasBeam = false;
        Chord* startChord = nullptr;
        Chord* endChord = nullptr;
        bool breakBeam = false;
        bool previousBreak32 = false;
        bool previousBreak64 = false;
        int prevRests = 0;

        for (uint i = 0; i < chordRests.size(); i++) {
            ChordRest* chordRest = chordRests[i];
            ChordRest* prevChordRest = i < 1 ? nullptr : chordRests[i - 1];
            if (!chordRest->isChord()) {
                prevRests++;
                continue;
            }
            Chord* chord = toChord(chordRest);
            if (level < chord->beams()) {
                levelHasBeam = true;
            }
            bool isBroken32 = false;
            bool isBroken64 = false;
            // updates isBroken32 and isBroken64
            calcBeamBreaks(chordRest, prevChordRest, level, isBroken32, isBroken64);
            breakBeam = isBroken32 || isBroken64;
            if (level < chord->beams() && !breakBeam) {
                endChord = chord;
                if (!startChord) {
                    startChord = chord;
                }
            } else {
                if (startChord && endChord) {
                    if (startChord == endChord) {
                        bool isBeamletBefore = calcIsBeamletBefore(startChord, i - 1 - prevRests, level, previousBreak32, previousBreak64);
                        createBeamletSegment(startChord, isBeamletBefore, level);
                    } else {
                        createBeamSegment(startChord, endChord, level);
                    }
                }
                startChord = breakBeam && level < chord->beams() ? chord : nullptr;
                endChord = breakBeam && level < chord->beams() ? chord : nullptr;
            }
            previousBreak32 = isBroken32;
            previousBreak64 = isBroken64;
            prevRests = 0;
        }

        // if the beam ends on the last chord
        if (startChord && (endChord || breakBeam)) {
            if (startChord == endChord || !endChord) {
                // since it's the last chord, beamlet always goes before
                createBeamletSegment(startChord, true, level);
            } else {
                createBeamSegment(startChord, endChord, level);
            }
        }
        level++;
    } while (levelHasBeam);
}

void Beam::offsetBeamToRemoveCollisions(const std::vector<ChordRest*> chordRests, int& dictator, int& pointer,
                                        const double startX, const double endX,
                                        bool isFlat, bool isStartDictator) const
{
    if (_cross) {
        return;
    }

    if (endX == startX) {
        // zero-length beams?
        return;
    }

    // tolerance eliminates all possibilities of floating point rounding errors
    const double tolerance = _beamWidth * 0.25 * (_up ? -1 : 1);

    double startY = (isStartDictator ? dictator : pointer) * spatium() / 4 + tolerance;
    double endY = (isStartDictator ? pointer : dictator) * spatium() / 4 + tolerance;

    // stems in the middle of the beam can be shortened to a minimum of.....
    // 1 beam 2 beams etc
    constexpr double middleStemMinLength[5] = { 2.75, 3.25, 3.75, 4.5, 5.25 };

    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord()) {
            continue;
        }

        Chord* chord = toChord(chordRest);
        PointF anchor = chordBeamAnchor(chord, ChordBeamAnchorType::Middle) - pagePos();
        double reduction = 0;
        if (chordRest != _elements.back() && chordRest != _elements.front()) {
            double minLength = chord->beams() < 6
                               ? middleStemMinLength[chord->beams() - 1]
                               : middleStemMinLength[4] + ((chord->beams() - 5) * 0.75);
            minLength *= spatium();
            reduction = chord->stem()->length() - minLength;
        }

        // avoid division by zero for zero-length beams (can exist as a pre-layout state used
        // for horizontal spacing computations)
        const double proportionAlongX = (anchor.x() - startX) / (endX - startX);

        while (true) {
            double desiredY = proportionAlongX * (endY - startY) + startY;
            bool beamClearsAnchor = (_up && RealIsEqualOrLess(desiredY, anchor.y() + reduction))
                                    || (!_up && RealIsEqualOrMore(desiredY, anchor.y() - reduction));
            if (beamClearsAnchor) {
                break;
            }

            if (isFlat) {
                dictator += _up ? -1 : 1;
                pointer += _up ? -1 : 1;
            } else if (std::abs(dictator - pointer) == 1) {
                dictator += _up ? -1 : 1;
            } else {
                pointer += _up ? -1 : 1;
            }

            startY = (isStartDictator ? dictator : pointer) * spatium() / 4 + tolerance;
            endY = (isStartDictator ? pointer : dictator) * spatium() / 4 + tolerance;
        }
    }
}

void Beam::extendStem(Chord* chord, double addition)
{
    PointF anchor = chordBeamAnchor(chord, ChordBeamAnchorType::Middle);
    double desiredY;
    if (_endAnchor.x() > _startAnchor.x()) {
        double proportionAlongX = (anchor.x() - _startAnchor.x()) / (_endAnchor.x() - _startAnchor.x());
        desiredY = proportionAlongX * (_endAnchor.y() - _startAnchor.y()) + _startAnchor.y();
    } else {
        desiredY = std::max(_endAnchor.y(), _startAnchor.y());
    }

    if (chord->up()) {
        chord->setBeamExtension(anchor.y() - desiredY + addition);
    } else {
        chord->setBeamExtension(desiredY - anchor.y() + addition);
    }
    if (chord->tremolo()) {
        chord->tremolo()->layout();
    }
}

bool Beam::isBeamInsideStaff(int yPos, int staffLines) const
{
    return yPos > -3 && yPos < staffLines * 4 - 1;
}

int Beam::getOuterBeamPosOffset(int innerBeam, int beamCount, int staffLines) const
{
    int spacing = (_up ? -_beamSpacing : _beamSpacing);
    int offset = (beamCount - 1) * spacing;
    while (offset != 0 && !isBeamInsideStaff(innerBeam + offset, staffLines)) {
        offset -= spacing;
    }
    return offset;
}

bool Beam::isValidBeamPosition(int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, int beamCount) const
{
    // outside the staff
    if (!isBeamInsideStaff(yPos, staffLines)) {
        return true;
    }
    // removes modulo weirdness with negative numbers (i.e., right above staff)
    yPos += 8;
    // is floater (doesn't apply to 3+ beams)
    if (beamCount < 3 && yPos % 4 == 2) {
        return false;
    }
    if (isFlat) {
        return true;
    }
    // is on line
    if (yPos % 4 == 0) {
        return true;
    }
    bool isSitting = yPos % 4 == 3;
    if (isSitting) {
        return !((isAscending && isStart) || (!isAscending && !isStart));
    }
    // hanging
    return !((!isAscending && isStart) || (isAscending && !isStart));
}

bool Beam::is64thBeamPositionException(int& yPos, int staffLines) const
{
    if (_beamSpacing == 4) {
        return false;
    }
    return yPos == 2 || yPos == staffLines * 4 - 2 || yPos == staffLines * 4 - 6 || yPos == -2;
}

int Beam::findValidBeamOffset(int outer, int beamCount, int staffLines, bool isStart, bool isAscending,
                              bool isFlat) const
{
    bool isBeamValid = false;
    int offset = 0;
    int innerBeam = outer + (beamCount - 1) * (_up ? _beamSpacing : -_beamSpacing);
    while (!isBeamValid) {
        while (!isValidBeamPosition(innerBeam + offset, isStart, isAscending, isFlat, staffLines, beamCount)) {
            offset += _up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(outerMostBeam, isStart, isAscending, isFlat,
                                staffLines, beamCount)
            || (beamCount == 4 && is64thBeamPositionException(outerMostBeam, staffLines))) {
            isBeamValid = true;
        } else {
            offset += _up ? -1 : 1;
        }
    }
    return offset;
}

void Beam::setValidBeamPositions(int& dictator, int& pointer, int beamCount, int staffLines, bool isStartDictator,
                                 bool isFlat, bool isAscending)
{
    if (_cross) {
        return;
    }
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCount >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && _beamSpacing != 4) {
        int dictatorInner = dictator + (beamCount - 1) * (_up ? _beamSpacing : -_beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(dictatorInner, beamCount, staffLines);
        if (std::abs(outerDictatorOffset) <= _beamSpacing) {
            has3BeamsInsideStaff = false;
            break;
        }
        // use dictator for both to simulate flat beams
        int offset = findValidBeamOffset(dictator, beamCount, staffLines, isStartDictator, false, true);
        dictator += offset;
        pointer = dictator;
        if (offset == 0) {
            areBeamsValid = true;
        }
    }
    while (!areBeamsValid) {
        int dictatorOffset = findValidBeamOffset(dictator, beamCount, staffLines, isStartDictator, isAscending, isFlat);
        dictator += dictatorOffset;
        pointer += dictatorOffset;
        if (isFlat) {
            pointer = dictator;
            int pointerOffset
                = findValidBeamOffset(pointer, beamCount, staffLines, !isStartDictator, isAscending, isFlat);
            if (pointerOffset == 0) {
                areBeamsValid = true;
            } else {
                dictator += pointerOffset;
                pointer += pointerOffset;
            }
        } else {
            pointer += findValidBeamOffset(pointer, beamCount, staffLines, !isStartDictator, isAscending, isFlat);
            if ((_up && pointer <= dictator) || (!_up && pointer >= dictator)) {
                dictator = pointer + (_up ? -1 : 1);
            } else {
                areBeamsValid = true;
            }
        }
    }
}

void Beam::addMiddleLineSlant(int& dictator, int& pointer, int beamCount, int middleLine, int interval)
{
    if (interval == 0 || (beamCount > 2 && _beamSpacing != 4) || noSlope()) {
        return;
    }
    bool isOnMiddleLine = pointer == middleLine && (std::abs(pointer - dictator) < 2);
    if (isOnMiddleLine) {
        if (interval == 1 || (beamCount == 2 && _beamSpacing != 4)) {
            dictator = middleLine + (_up ? -1 : 1);
        } else {
            dictator = middleLine + (_up ? -2 : 2);
        }
    }
}

void Beam::add8thSpaceSlant(PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                            int interval, int middleLine, bool isFlat)
{
    if (beamCount != 3 || noSlope() || _beamSpacing != 3) {
        return;
    }
    if ((isFlat && dictator != middleLine) || (dictator != pointer) || interval == 0) {
        return;
    }
    if ((_up && (dictator + 4) % 4 == 3) || (!_up && (dictator + 4) % 4 == 1)) {
        return;
    }
    dictatorAnchor.setY(dictatorAnchor.y() + (_up ? -0.125 * spatium() : 0.125 * spatium()));
    _beamDist += 0.0625 * spatium();
}

//---------------------------------------------------------
//   layout2
//---------------------------------------------------------

void Beam::layout2(const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag)
{
    if (chordRests.empty()) {
        return;
    }
    for (auto chordRest : chordRests) {
        if (chordRest->isChord()) {
            toChord(chordRest)->layoutStem();
        }
    }
    if (_distribute) {
        // fix horizontal spacing of stems
        LayoutBeams::respace(chordRests);
    }

    if (!chordRests.front()->isChord() || !chordRests.back()->isChord()) {
        NOT_IMPLEMENTED;
        return;
    }

    // todo: add edge case for when a beam starts or ends on a rest

    _beamSpacing = score()->styleB(Sid::useWideBeams) ? 4 : 3;
    _beamDist = (_beamSpacing / 4.0) * spatium() * mag();
    _beamWidth = point(score()->styleS(Sid::beamWidth)) * mag();

    Chord* startChord = toChord(chordRests.front());
    Chord* endChord = toChord(chordRests.back());
    _startAnchor = chordBeamAnchor(startChord, ChordBeamAnchorType::Start);
    _endAnchor = chordBeamAnchor(endChord, ChordBeamAnchorType::End);

    if (_isGrace) {
        _beamDist *= score()->styleD(Sid::graceNoteMag);
        _beamWidth *= score()->styleD(Sid::graceNoteMag);
    }

    const Staff* staffItem = staff();
    const StaffType* staffType = staffItem ? staffItem->staffTypeForElement(this) : nullptr;
    _tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    _isBesideTabStaff = _tab && !_tab->stemless() && !_tab->stemThrough();

    int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    if (_userModified[fragmentIndex]) {
        double startY = fragments[frag]->py1[fragmentIndex] + pagePos().y();
        double endY = fragments[frag]->py2[fragmentIndex] + pagePos().y();
        if (score()->styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        _startAnchor.setY(startY);
        _endAnchor.setY(endY);
        _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
        createBeamSegments(chordRests);
        return;
    }

    if (_cross) {
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
        for (ChordRest* c : chordRests) {
            if (c && (otherStaff = c->staffMove())) {
                break;
            }
        }
        if (otherStaff != 0 && _minMove != _maxMove) {
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
                    maxY = std::min(maxY, chordBeamAnchorY(toChord(c)));
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
                    minY = std::max(minY, chordBeamAnchorY(toChord(c)));
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
                if (chordRests.size() == 2 && chordRests[0]->staffMove() != chordRests[1]->staffMove()) {
                    // if there are only two notes, one on each staff, special case
                    // take max slope into account
                    int desiredSlant = round((chordRests[0]->stemPos().y() - chordRests[1]->stemPos().y()) / spatium());
                    int slant = std::min(std::abs(desiredSlant), getMaxSlope());
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
                        slant = std::min(std::abs(slant), getMaxSlope());
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
                        slant = std::min(std::abs(slant), getMaxSlope());
                        double slope = slant * ((topSlant < 0) ? -quarterSpace : quarterSpace);
                        _startAnchor.ry() += (slope / 2);
                        _endAnchor.ry() -= (slope / 2);
                    } else {
                        // if the two slopes are in opposite directions, flat!
                        // nothing needs to be done, the beam is already horizontal and placed nicely
                    }
                }
                _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
            }
            fragments[frag]->py1[fragmentIndex] = _startAnchor.y() - pagePos().y();
            fragments[frag]->py2[fragmentIndex] = _endAnchor.y() - pagePos().y();
            createBeamSegments(chordRests);
            return;
        } else {
            _cross = false;
        }
    }

    // anchor represents the middle of the beam, not the tip of the stem
    // location depends on _isBesideTabStaff

    if (!_isBesideTabStaff) {
        int startNote = _up ? startChord->upNote()->line() : startChord->downNote()->line();
        int endNote = _up ? endChord->upNote()->line() : endChord->downNote()->line();
        if (_tab) {
            startNote = _up ? startChord->upString() : startChord->downString();
            endNote = _up ? endChord->upString() : endChord->downString();
        }
        const int interval = std::abs(startNote - endNote);
        const bool isStartDictator = _up ? startNote < endNote : startNote > endNote;
        const double quarterSpace = spatium() / 4;
        PointF startAnchor = _startAnchor - pagePos();
        PointF endAnchor = _endAnchor - pagePos();
        int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
        int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

        const int staffLines = startChord->staff()->lines(tick());
        const int middleLine = getMiddleStaffLine(startChord, endChord, staffLines);

        int slant = computeDesiredSlant(startNote, endNote, middleLine, dictator, pointer);
        pointer = dictator + slant;
        bool isFlat = slant == 0;
        bool isAscending = startNote > endNote;
        int beamCount = getBeamCount(chordRests);

        if (endAnchor.x() > startAnchor.x()) {
            /* When beam layout is called before horizontal spacing (see LayoutMeasure::getNextMeasure() to
             * know why) the x positions aren't yet determined and may be all zero, which would cause the
             * following function to get stuck in a loop. The if() condition avoids that case. */
            offsetBeamToRemoveCollisions(chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
        }
        if (!_tab) {
            if (!_isGrace) {
                setValidBeamPositions(dictator, pointer, beamCount, staffLines, isStartDictator, isFlat, isAscending);
            }
            addMiddleLineSlant(dictator, pointer, beamCount, middleLine, interval);
        }

        _startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer) + pagePos().y());
        _endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator) + pagePos().y());

        bool add8th = true;
        for (bool modified : _userModified) {
            if (modified) {
                add8th = false;
                break;
            }
        }
        if (!_tab && add8th) {
            add8thSpaceSlant(isStartDictator ? _startAnchor : _endAnchor, dictator, pointer, beamCount, interval, middleLine, isFlat);
        }
        _slope = (_endAnchor.y() - _startAnchor.y()) / (_endAnchor.x() - _startAnchor.x());
    } else {
        _slope = 0;
    }

    fragments[frag]->py1[fragmentIndex] = _startAnchor.y() - pagePos().y();
    fragments[frag]->py2[fragmentIndex] = _endAnchor.y() - pagePos().y();

    createBeamSegments(chordRests);
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
    writeProperty(xml, Pid::DISTRIBUTE);
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
//   read
//---------------------------------------------------------

void Beam::read(XmlReader& e)
{
    double _spatium = spatium();
    if (score()->mscVersion() < 301) {
        _id = e.intAttribute("id");
    }
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "StemDirection") {
            readProperty(e, Pid::STEM_DIRECTION);
        } else if (tag == "distribute") {
            setDistribute(e.readInt());
        } else if (readStyledProperty(e, tag)) {
        } else if (tag == "growLeft") {
            setGrowLeft(e.readDouble());
        } else if (tag == "growRight") {
            setGrowRight(e.readDouble());
        } else if (tag == "y1") {
            if (fragments.empty()) {
                fragments.push_back(new BeamFragment);
            }
            BeamFragment* f = fragments.back();
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            f->py1[idx] = e.readDouble() * _spatium;
        } else if (tag == "y2") {
            if (fragments.empty()) {
                fragments.push_back(new BeamFragment);
            }
            BeamFragment* f = fragments.back();
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            f->py2[idx] = e.readDouble() * _spatium;
        } else if (tag == "Fragment") {
            BeamFragment* f = new BeamFragment;
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            double _spatium1 = spatium();

            while (e.readNextStartElement()) {
                const AsciiStringView tag1(e.name());
                if (tag1 == "y1") {
                    f->py1[idx] = e.readDouble() * _spatium1;
                } else if (tag1 == "y2") {
                    f->py2[idx] = e.readDouble() * _spatium1;
                } else {
                    e.unknown();
                }
            }
            fragments.push_back(f);
        } else if (tag == "l1" || tag == "l2") {      // ignore
            e.skipCurrentElement();
        } else if (tag == "subtype") {          // obsolete
            e.skipCurrentElement();
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   BeamEditData
//---------------------------------------------------------

class BeamEditData : public ElementEditData
{
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

    double middleX = (c1->stemPosX() + c1->pageX() + c2->stemPosX() + c2->pageX()) / 2;
    double middleY = (f->py1[idx] + y + f->py2[idx] + y) / 2;

    return {
        PointF(c1->stemPosX() + c1->pageX(), f->py1[idx] + y),
        PointF(c2->stemPosX() + c2->pageX(), f->py2[idx] + y),
        PointF(middleX, middleY)
    };
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setBeamDirection(DirectionV d)
{
    if (_direction == d) {
        return;
    }

    _direction = d;

    if (d != DirectionV::AUTO) {
        _up = d == DirectionV::UP;
    }

    for (ChordRest* rest : elements()) {
        if (Chord* chord = toChord(rest)) {
            chord->setStemDirection(d);
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
    if (distribute()) {
        undoChangeProperty(Pid::DISTRIBUTE, false);
    }
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
    case Pid::DISTRIBUTE:     return distribute();
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
    case Pid::DISTRIBUTE:
        setDistribute(v.toBool());
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
    case Pid::DISTRIBUTE:     return false;
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
