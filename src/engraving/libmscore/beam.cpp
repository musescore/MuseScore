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

#include "draw/brush.h"
#include "style/style.h"
#include "rw/xml.h"

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

namespace Ms {
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
    qreal py1[2];
    qreal py2[2];
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
    for (const LineF* bs : b._beamSegments) {
        _beamSegments.append(new LineF(*bs));
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
        fragments.append(new BeamFragment(*f));
    }
    _minMove          = b._minMove;
    _maxMove          = b._maxMove;
    _isGrace         = b._isGrace;
    _cross           = b._cross;
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
    for (ChordRest* cr : qAsConst(_elements)) {
        cr->setBeam(0);
    }
    qDeleteAll(_beamSegments);
    qDeleteAll(fragments);
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
    qreal yp = y() + s->staff(staffIdx())->y() + s->y();
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
    if (!_elements.contains(a)) {
        //
        // insert element in same order as it appears
        // in the score
        //
        if (a->segment() && !_elements.empty()) {
            for (int i = 0; i < _elements.size(); ++i) {
                Segment* s = _elements[i]->segment();
                if ((s->tick() > a->segment()->tick())
                    || ((s->tick() == a->segment()->tick()) && (a->segment()->next(SegmentType::ChordRest) == s))
                    ) {
                    _elements.insert(i, a);
                    return;
                }
            }
        }
        _elements.append(a);
    }
}

//---------------------------------------------------------
//   removeChordRest
//---------------------------------------------------------

void Beam::removeChordRest(ChordRest* a)
{
    if (!_elements.removeOne(a)) {
        qDebug("Beam::remove(): cannot find ChordRest");
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
    qreal lw2 = point(score()->styleS(Sid::beamWidth)) * .5 * mag();

    // make beam thickness independent of slant
    // (expression can be simplified?)

    const LineF* bs = _beamSegments.front();
    double d  = (qAbs(bs->y2() - bs->y1())) / (bs->x2() - bs->x1());
    if (_beamSegments.size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww = lw2 / sin(M_PI_2 - atan(d));

    for (const LineF* bs1 : _beamSegments) {
        painter->drawPolygon(
            PolygonF({
                PointF(bs1->x1(), bs1->y1() - ww),
                PointF(bs1->x2(), bs1->y2() - ww),
                PointF(bs1->x2(), bs1->y2() + ww),
                PointF(bs1->x1(), bs1->y1() + ww),
            }),
            Qt::OddEvenFill);
    }
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(const PointF& offset)
{
    EngravingItem::move(offset);
    for (mu::LineF* bs : qAsConst(_beamSegments)) {
        bs->translate(offset);
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
        for (ChordRest* cr : qAsConst(_elements)) {
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
        } else {
            for (ChordRest* cr :qAsConst(_elements)) {
                if (cr->isChord()) {
                    _up = toChord(cr)->up();
                    break;
                }
            }
        }
        for (ChordRest* cr : qAsConst(_elements)) {
            cr->computeUp();
            if (cr->isChord()) {
                toChord(cr)->layoutStem();
            }
        }
        return;
    }

    _minMove = std::numeric_limits<int>::max();
    _maxMove = std::numeric_limits<int>::min();
    _isGrace = false;
    qreal mag = 0.0;

    _notes.clear();
    int staffIdx = -1;
    for (ChordRest* cr : qAsConst(_elements)) {
        qreal m = cr->isSmall() ? score()->styleD(Sid::smallNoteMag) : 1.0;
        mag = qMax(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            staffIdx = chord->vStaffIdx();
            int i = chord->staffMove();
            _minMove = qMin(_minMove, i);
            _maxMove = qMax(_maxMove, i);

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
        _up = true;
    } else if (_minMove < 0) {
        _up = false;
    } else if (_notes.size()) {
        ChordRest* firstNote = _elements.first();
        Measure* measure = firstNote->measure();
        bool hasMultipleVoices = measure->hasVoices(firstNote->staffIdx(), tick(), ticks());
        if (hasMultipleVoices) {
            _up = firstNote->track() % 2 == 0;
        } else {
            if (const Chord* chord = findChordWithCustomStemDirection()) {
                _up = chord->stemDirection() == DirectionV::UP;
            } else {
                std::vector<int> notes;
                std::set<int> noteSet(_notes.begin(), _notes.end());
                notes.assign(noteSet.begin(), noteSet.end());
                _up = Chord::computeAutoStemDirection(&notes) > 0;
            }
        }
    } else {
        _up = true;
    }

    ChordRest* firstNote = _elements.first();
    int middleStaffLine = firstNote->staffType()->middleLine();
    for (uint i = 0; i < _notes.size(); i++) {
        _notes[i] += middleStaffLine;
    }

    _cross = _minMove < _maxMove;
    if (_minMove == 1 && _maxMove == 1) {
        setStaffIdx(staffIdx);
    } else if (_elements.size()) {
        setStaffIdx(_elements.at(0)->staffIdx());
    }

    _slope = 0.0;

    for (ChordRest* cr : qAsConst(_elements)) {
        const bool staffMove = cr->isChord() ? toChord(cr)->staffMove() : false;
        if (!_cross || !staffMove) {
            if (cr->up() != _up) {
                cr->computeUp();
                if (cr->isChord()) {
                    toChord(cr)->layoutStem();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutGraceNotes
//---------------------------------------------------------

void Beam::layoutGraceNotes()
{
    _maxDuration.setType(DurationType::V_INVALID);
    Chord* c1 = 0;
    Chord* c2 = 0;

    _minMove = std::numeric_limits<int>::max();
    _maxMove = std::numeric_limits<int>::min();
    _isGrace = true;
    setMag(score()->styleD(Sid::graceNoteMag));

    for (ChordRest* cr : qAsConst(_elements)) {
        c2 = toChord(cr);
        if (c1 == 0) {
            c1 = c2;
        }
        int i = c2->staffMove();
        _minMove = qMin(_minMove, i);
        _maxMove = qMax(_maxMove, i);
        if (!_maxDuration.isValid() || (_maxDuration < cr->durationType())) {
            _maxDuration = cr->durationType();
        }
    }
    //
    // determine beam stem direction
    //
    if (staff()->isTabStaff(Fraction(0, 1))) {
        // direction determined only by tab direction
        _up = !staff()->staffType(Fraction(0, 1))->stemsDown();
    } else {
        if (_direction != DirectionV::AUTO) {
            _up = _direction == DirectionV::UP;
        } else {
            ChordRest* cr = _elements[0];
            Measure* m = cr->measure();
            bool hasMultipleVoices = m->hasVoices(cr->staffIdx(), tick(), ticks());
            _up = hasMultipleVoices ? cr->track() % 2 == 0 : true;
        }
    }

    _slope = 0.0;

    for (ChordRest* cr : qAsConst(_elements)) {
        cr->computeUp();
        if (cr->isChord()) {
            toChord(cr)->layoutStem();
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

    int n = 0;
    for (ChordRest* cr : qAsConst(_elements)) {
        if (cr->measure()->system() != system) {
            SpannerSegmentType st;
            if (n == 0) {
                st = SpannerSegmentType::BEGIN;
            } else {
                st = SpannerSegmentType::MIDDLE;
            }
            ++n;
            if (fragments.size() < n) {
                fragments.append(new BeamFragment);
            }
            layout2(crl, st, n - 1);
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
            fragments.append(new BeamFragment);
        }
        layout2(crl, st, n);

        qreal lw2 = point(score()->styleS(Sid::beamWidth)) * .5 * mag();

        for (const LineF* bs : qAsConst(_beamSegments)) {
            PolygonF a(4);
            a[0] = PointF(bs->x1(), bs->y1());
            a[1] = PointF(bs->x2(), bs->y2());
            a[2] = PointF(bs->x2(), bs->y2());
            a[3] = PointF(bs->x1(), bs->y1());
            RectF r(a.boundingRect().adjusted(0.0, -lw2, 0.0, lw2));
            addbbox(r);
        }
    }
}

int Beam::getMiddleStaffLine(ChordRest* startChord, ChordRest* endChord, int staffLines) const
{
    bool useWideBeams = score()->styleB(Sid::useWideBeams);
    int startMiddleLine = Chord::minStaffOverlap(_up, staffLines, startChord->beams(), false, _beamSpacing / 4.0, useWideBeams);
    int endMiddleLine = Chord::minStaffOverlap(_up, staffLines, endChord->beams(), false, _beamSpacing / 4.0, useWideBeams);

    // offset middle line by 1 or -1 since the anchor is at the middle of the beam,
    // not at the tip of the stem
    if (_up) {
        return qMin(startMiddleLine, endMiddleLine) + 1;
    }
    return qMax(startMiddleLine, endMiddleLine) - 1;
}

int Beam::computeDesiredSlant(int startNote, int endNote, int middleLine, int dictator, int pointer) const
{
    if (score()->styleB(Sid::beamNoSlope)) {
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
            int higherEnd = qMin(startNote, endNote);
            if (higherEnd > _notes[0]) {
                return 0;
            }
            if (higherEnd == _notes[0] && higherEnd >= _notes[1]) {
                if (higherEnd > _notes[1]) {
                    return 0;
                }
                int chordCount = _elements.size();
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
            int lowerEnd = qMax(startNote, endNote);
            if (lowerEnd < _notes[_notes.size() - 1]) {
                return 0;
            }
            if (lowerEnd == _notes[_notes.size() - 1] && lowerEnd <= _notes[_notes.size() - 2]) {
                if (lowerEnd < _notes[_notes.size() - 2]) {
                    return 0;
                }
                int chordCount = _elements.size();
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
    // for 2-indexed interval i (seconds, thirds, etc.)
    // maxSlopes[i] = max slope of beam for notes with interval i
    static constexpr std::array maxSlopes = { 1, 2, 3, 4, 5, 6, 7 };

    // calculate max slope based on distance between first and last chords
    qreal beamWidth = _elements[_elements.size() - 1]->stemPos().x() - _elements[0]->stemPos().x();
    beamWidth /= spatium();
    int maxSlope = maxSlopes.back();
    if (beamWidth < 3.0) {
        maxSlope = maxSlopes[0];
    } else if (beamWidth < 5.0) {
        maxSlope = maxSlopes[1];
    } else if (beamWidth < 8.0) {
        maxSlope = maxSlopes[2];
    } else if (beamWidth < 13.0) {
        maxSlope = maxSlopes[3];
    } else if (beamWidth < 21.0) {
        maxSlope = maxSlopes[4];
    } else if (beamWidth < 34.0) {
        maxSlope = maxSlopes[5];
    } else {
        maxSlope = maxSlopes[6];
    }

    // calculate max slope based on note interval
    int interval = qMin(qAbs(endNote - startNote), (int)maxSlopes.size() - 1);
    return qMin(maxSlope, maxSlopes[interval]) * (_up ? 1 : -1);
}

int Beam::getBeamCount(std::vector<ChordRest*> chordRests) const
{
    int beamCount = 0;
    for (ChordRest* chordRest : chordRests) {
        if (chordRest->isChord() && toChord(chordRest)->beams() > beamCount) {
            beamCount = toChord(chordRest)->beams();
        }
    }
    return beamCount;
}

PointF Beam::chordBeamAnchor(Chord* chord) const
{
    Note* note = _up ? chord->downNote() : chord->upNote();
    PointF position = note->pos() + chord->segment()->pos() + chord->measure()->pos();

    int upValue = _up ? -1 : 1;
    qreal beamWidth = score()->styleMM(Sid::beamWidth).val() * chord->mag();
    qreal beamOffset = beamWidth / 2 * upValue;

    qreal x = chord->stemPosX() + chord->pagePos().x() - pagePos().x();
    qreal y = position.y() + chord->defaultStemLength() * upValue - beamOffset;
    if (_isBesideTabStaff) {
        StaffType const* staffType = chord->staff()->staffType(chord->tick());
        qreal stemLength = staffType->chordStemLength(chord);
        y = _tab->chordRestStemPosY(chord) + stemLength;
        y *= spatium();
        y -= beamOffset;
    }
    return PointF(x, y);
}

void Beam::createBeamSegment(Chord* startChord, Chord* endChord, int level)
{
    PointF posStart = chordBeamAnchor(startChord);
    PointF posEnd = chordBeamAnchor(endChord);
    qreal y2 = _slope * (posEnd.x() - posStart.x()) + posStart.y();

    int upValue = _up ? -1 : 1;
    qreal verticalOffset = _beamDist * level * upValue;
    qreal stemWidth = score()->styleMM(Sid::stemWidth).val() * startChord->mag();

    qreal startOffsetX = _up && !_tab ? -stemWidth : 0.0;
    qreal endOffsetX = _up || _tab ? 0.0 : stemWidth;

    _beamSegments.push_back(
        new LineF(
            posStart.x() + startOffsetX,
            posStart.y() - verticalOffset,
            posEnd.x() + endOffsetX,
            y2 - verticalOffset
            )
        );
}

bool Beam::calcIsBeamletBefore(Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break) const
{
    // if first or last chord in beam group
    if (i == 0) {
        return false;
    } else if (i == _elements.size() - 1) {
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
    if (nextChordRest->isChord()) {
        bool nextBreak32 = false;
        bool nextBreak64 = false;
        calcBeamBreaks(toChord(nextChordRest), level, nextBreak32, nextBreak64);
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
                previousChordLevel = qMin(previousChordLevel, 1);
            } else if (isAfter64Break) {
                previousChordLevel = qMin(previousChordLevel, 2);
            }
            break;
        }
        ++previousOffset;
    }

    int nextOffset = 1;
    while (i + nextOffset < _elements.size()) {
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
    PointF chordPos = chordBeamAnchor(chord);
    qreal beamletLength = score()->styleMM(Sid::beamMinLen).val()
                          * mag()
                          * chord->staff()->staffMag(chord);
    qreal x2 = chordPos.x() + (isBefore ? -beamletLength : beamletLength);
    qreal y2 = chordPos.y() + _slope * (x2 - chordPos.x());

    int upValue = _up ? -1 : 1;
    qreal verticalOffset = _beamDist * level * upValue;
    qreal stemWidth = score()->styleMM(Sid::stemWidth).val() * chord->mag();
    qreal startOffsetX = 0;
    if (!_tab) {
        if (isBefore && !_up) {
            startOffsetX = stemWidth;
        } else if (!isBefore && _up) {
            startOffsetX = -stemWidth;
        }
    }

    _beamSegments.push_back(
        new LineF(
            chordPos.x() + startOffsetX,
            chordPos.y() - verticalOffset,
            x2,
            y2 - verticalOffset
            )
        );
}

void Beam::calcBeamBreaks(const Chord* chord, int level, bool& isBroken32, bool& isBroken64) const
{
    BeamMode beamMode = chord->beamMode();

    // get default beam mode -- based on time signature preferences
    const Groups& group = chord->staff()->group(chord->measure()->tick());
    Fraction stretch = chord->staff()->timeStretch(chord->measure()->tick());
    int currentTick = (chord->rtick() * stretch).ticks();
    TDuration currentDuration = chord->durationType();
    BeamMode defaultBeamMode = group.beamMode(currentTick, currentDuration.type());

    bool isManuallyBroken32 = level >= 1 && beamMode == BeamMode::BEGIN32;
    bool isManuallyBroken64 = level >= 2 && beamMode == BeamMode::BEGIN64;
    bool isDefaultBroken32 = beamMode == BeamMode::AUTO && level >= 1 && defaultBeamMode == BeamMode::BEGIN32;
    bool isDefaultBroken64 = beamMode == BeamMode::AUTO && level >= 2 && defaultBeamMode == BeamMode::BEGIN64;

    isBroken32 = isManuallyBroken32 || isDefaultBroken32;
    isBroken64 = isManuallyBroken64 || isDefaultBroken64;
}

void Beam::createBeamSegments(std::vector<ChordRest*> chordRests)
{
    qDeleteAll(_beamSegments);
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

        for (uint i = 0; i < chordRests.size(); i++) {
            ChordRest* chordRest = chordRests[i];
            if (!chordRest->isChord()) {
                continue;
            }
            Chord* chord = toChord(chordRest);

            bool isBroken32 = false;
            bool isBroken64 = false;
            // updates isBroken32 and isBroken64
            calcBeamBreaks(chord, level, isBroken32, isBroken64);
            breakBeam = isBroken32 || isBroken64;

            if (level < chord->beams() && !breakBeam) {
                endChord = chord;
                if (!startChord) {
                    startChord = chord;
                }
                levelHasBeam = true;
            } else {
                if (startChord && endChord) {
                    if (startChord == endChord) {
                        bool isBeamletBefore = calcIsBeamletBefore(startChord, i - 1, level, previousBreak32, previousBreak64);
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

void Beam::offsetBeamToRemoveCollisions(std::vector<ChordRest*> chordRests, int& dictator, int& pointer, qreal startX, qreal endX,
                                        bool isFlat, bool isStartDictator) const
{
    // tolerance eliminates all possibilities of floating point rounding errors
    qreal tolerance = score()->styleMM(Sid::beamWidth).val() * mag() * 0.25 * (_up ? -1 : 1);
    qreal startY = (isStartDictator ? dictator : pointer) * spatium() / 4 + tolerance;
    qreal endY = (isStartDictator ? pointer : dictator) * spatium() / 4 + tolerance;
    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord()) {
            continue;
        }
        Chord* chord = toChord(chordRest);
        PointF anchor = chordBeamAnchor(chord);
        qreal proportionAlongX = (anchor.x() - startX) / (endX - startX);
        while (true) {
            qreal desiredY = proportionAlongX * (endY - startY) + startY;
            bool beamClearsAnchor = (_up && desiredY <= anchor.y()) || (!_up && desiredY >= anchor.y());
            if (beamClearsAnchor) {
                break;
            } else {
                if (isFlat) {
                    dictator += _up ? -1 : 1;
                    pointer += _up ? -1 : 1;
                } else if (qAbs(dictator - pointer) == 1) {
                    dictator += _up ? -1 : 1;
                } else {
                    pointer += _up ? -1 : 1;
                }
                startY = (isStartDictator ? dictator : pointer) * spatium() / 4 + tolerance;
                endY = (isStartDictator ? pointer : dictator) * spatium() / 4 + tolerance;
            }
        }
    }
}

void Beam::extendStems(std::vector<ChordRest*> chordRests, PointF start, PointF end)
{
    for (ChordRest* chordRest : chordRests) {
        if (!chordRest->isChord()) {
            continue;
        }
        Chord* chord = toChord(chordRest);
        PointF anchor = chordBeamAnchor(chord);
        qreal proportionAlongX = (anchor.x() - start.x()) / (end.x() - start.x());
        qreal desiredY = proportionAlongX * (end.y() - start.y()) + start.y();
        if (_up) {
            chord->setBeamExtension(anchor.y() - desiredY);
        } else {
            chord->setBeamExtension(desiredY - anchor.y());
        }
        if (chord->tremolo()) {
            chord->tremolo()->layout();
        }
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

bool Beam::isValidBeamPosition(int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines) const
{
    // outside the staff
    if (!isBeamInsideStaff(yPos, staffLines)) {
        return true;
    }
    // removes modulo weirdness with negative numbers (i.e., right above staff)
    yPos += 8;
    // is floater
    if (yPos % 4 == 2) {
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
        while (!isValidBeamPosition(innerBeam + offset, isStart, isAscending, isFlat, staffLines)) {
            offset += _up ? -1 : 1;
        }
        int outerMostBeam = innerBeam + offset + getOuterBeamPosOffset(innerBeam + offset, beamCount, staffLines);
        if (isValidBeamPosition(outerMostBeam, isStart, isAscending, isFlat,
                                staffLines)
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
    bool areBeamsValid = false;
    bool has3BeamsInsideStaff = beamCount >= 3;
    while (!areBeamsValid && has3BeamsInsideStaff && _beamSpacing != 4) {
        int dictatorInner = dictator + (beamCount - 1) * (_up ? _beamSpacing : -_beamSpacing);
        // use dictatorInner for both to simulate flat beams
        int outerDictatorOffset = getOuterBeamPosOffset(dictatorInner, beamCount, staffLines);
        if (qAbs(outerDictatorOffset) < _beamSpacing) {
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
    if (interval == 0 || (beamCount > 2 && _beamSpacing != 4) || score()->styleB(Sid::beamNoSlope)) {
        return;
    }
    bool isOnMiddleLine = pointer == middleLine && (qAbs(pointer - dictator) < 2);
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
    if (beamCount != 3 || score()->styleB(Sid::beamNoSlope) || _beamSpacing != 3) {
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

void Beam::layout2(std::vector<ChordRest*> chordRests, SpannerSegmentType, int frag)
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
        LayoutBeams::respace(&chordRests);
    }

    _beamSpacing = score()->styleB(Sid::useWideBeams) ? 4 : 3;
    _beamDist = (_beamSpacing / 4.0) * spatium() * mag();

    if (!chordRests.front()->isChord() || !chordRests.back()->isChord()) {
        NOT_IMPL_RETURN;
    }

    // todo: add edge case for when a beam starts or ends on a rest
    Chord* startChord = toChord(chordRests.front());
    Chord* endChord = toChord(chordRests.back());

    const Staff* staffItem = staff();
    const StaffType* staffType = staffItem ? staffItem->staffTypeForElement(this) : nullptr;
    _tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;
    _isBesideTabStaff = _tab && !_tab->stemless() && !_tab->stemThrough();

    // anchor represents the middle of the beam, not the tip of the stem
    // location depends on _isBesideTabStaff
    PointF startAnchor = chordBeamAnchor(startChord);
    PointF endAnchor = chordBeamAnchor(endChord);

    if (!_isBesideTabStaff) {
        int startNote = _up ? startChord->upNote()->line() : startChord->downNote()->line();
        int endNote = _up ? endChord->upNote()->line() : endChord->downNote()->line();
        if (_tab) {
            startNote = _up ? startChord->upString() : startChord->downString();
            endNote = _up ? endChord->upString() : endChord->downString();
        }
        const int interval = qAbs(startNote - endNote);
        const bool isStartDictator = _up ? startNote < endNote : startNote > endNote;
        const qreal quarterSpace = spatium() / 4;
        int dictator = round((isStartDictator ? startAnchor.y() : endAnchor.y()) / quarterSpace);
        int pointer = round((isStartDictator ? endAnchor.y() : startAnchor.y()) / quarterSpace);

        const int staffLines = startChord->staff()->lines(tick());
        const int middleLine = getMiddleStaffLine(startChord, endChord, staffLines);

        int slant = computeDesiredSlant(startNote, endNote, middleLine, dictator, pointer);
        pointer = dictator + slant;
        bool isFlat = slant == 0;
        bool isAscending = startNote > endNote;
        int beamCount = getBeamCount(chordRests);

        offsetBeamToRemoveCollisions(chordRests, dictator, pointer, startAnchor.x(), endAnchor.x(), isFlat, isStartDictator);
        if (!_tab) {
            setValidBeamPositions(dictator, pointer, beamCount, staffLines, isStartDictator, isFlat, isAscending);
            addMiddleLineSlant(dictator, pointer, beamCount, middleLine, interval);
        }

        startAnchor.setY(quarterSpace * (isStartDictator ? dictator : pointer));
        endAnchor.setY(quarterSpace * (isStartDictator ? pointer : dictator));

        if (!_tab) {
            add8thSpaceSlant(isStartDictator ? startAnchor : endAnchor, dictator, pointer, beamCount, interval, middleLine, isFlat);
        }
        _slope = (endAnchor.y() - startAnchor.y()) / (endAnchor.x() - startAnchor.x());
        extendStems(chordRests, startAnchor, endAnchor);
    } else {
        _slope = 0;
    }

    int fragmentIndex = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    fragments[frag]->py1[fragmentIndex] = startAnchor.y();
    fragments[frag]->py2[fragmentIndex] = endAnchor.y();

    createBeamSegments(chordRests);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Beam::spatiumChanged(qreal oldValue, qreal newValue)
{
    int idx = (!_up) ? 0 : 1;
    if (_userModified[idx]) {
        qreal diff = newValue / oldValue;
        for (BeamFragment* f : qAsConst(fragments)) {
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
    xml.startObject(this);
    EngravingItem::writeProperties(xml);

    writeProperty(xml, Pid::STEM_DIRECTION);
    writeProperty(xml, Pid::DISTRIBUTE);
    writeProperty(xml, Pid::BEAM_NO_SLOPE);
    writeProperty(xml, Pid::GROW_LEFT);
    writeProperty(xml, Pid::GROW_RIGHT);

    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    if (_userModified[idx]) {
        qreal _spatium = spatium();
        for (BeamFragment* f : fragments) {
            xml.startObject("Fragment");
            xml.tag("y1", f->py1[idx] / _spatium);
            xml.tag("y2", f->py2[idx] / _spatium);
            xml.endObject();
        }
    }

    // this info is used for regression testing
    // l1/l2 is the beam position of the layout engine
    if (MScore::testMode) {
        qreal spatium8 = spatium() * .125;
        for (BeamFragment* f : fragments) {
            xml.tag("l1", int(lrint(f->py1[idx] / spatium8)));
            xml.tag("l2", int(lrint(f->py2[idx] / spatium8)));
        }
    }

    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Beam::read(XmlReader& e)
{
    qreal _spatium = spatium();
    if (score()->mscVersion() < 301) {
        _id = e.intAttribute("id");
    }
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "StemDirection") {
            readProperty(e, Pid::STEM_DIRECTION);
            e.readNext();
        } else if (tag == "distribute") {
            setDistribute(e.readInt());
        } else if (readStyledProperty(e, tag)) {
        } else if (tag == "growLeft") {
            setGrowLeft(e.readDouble());
        } else if (tag == "growRight") {
            setGrowRight(e.readDouble());
        } else if (tag == "y1") {
            if (fragments.empty()) {
                fragments.append(new BeamFragment);
            }
            BeamFragment* f = fragments.back();
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            f->py1[idx] = e.readDouble() * _spatium;
        } else if (tag == "y2") {
            if (fragments.empty()) {
                fragments.append(new BeamFragment);
            }
            BeamFragment* f = fragments.back();
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            f->py2[idx] = e.readDouble() * _spatium;
        } else if (tag == "Fragment") {
            BeamFragment* f = new BeamFragment;
            int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
            _userModified[idx] = true;
            qreal _spatium1 = spatium();

            while (e.readNextStartElement()) {
                const QStringRef& tag1(e.name());
                if (tag1 == "y1") {
                    f->py1[idx] = e.readDouble() * _spatium1;
                } else if (tag1 == "y2") {
                    f->py2[idx] = e.readDouble() * _spatium1;
                } else {
                    e.unknown();
                }
            }
            fragments.append(f);
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
    qreal dy = ed.delta.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = fragments[bed->editFragment];
    qreal y1 = f->py1[idx];
    qreal y2 = f->py2[idx];

    if (ed.curGrip == Grip::START) {
        y1 += dy;
    } else if (ed.curGrip == Grip::END) {
        y2 += dy;
    } else if (ed.curGrip == Grip::MIDDLE) {
        y1 += dy;
        y2 += dy;
    }

    qreal _spatium = spatium();
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
    int n = _elements.size();

    if (n == 0) {
        return std::vector<PointF>();
    }

    for (int i = 0; i < n; ++i) {
        if (_elements[i]->isChordRest()) {
            c1 = toChordRest(_elements[i]);
            break;
        }
    }
    if (!c1) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }
    for (int i = n - 1; i >= 0; --i) {
        if (_elements[i]->isChordRest()) {
            c2 = toChordRest(_elements[i]);
            break;
        }
    }
    if (!c2) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }

    int y = pagePos().y();

    qreal middleX = (c1->stemPosX() + c1->pageX() + c2->stemPosX() + c2->pageX()) / 2;
    qreal middleY = (f->py1[idx] + y + f->py2[idx] + y) / 2;

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
        return type == ActionIconType::BEAM_FEATHERED_SLOWER
               || type == ActionIconType::BEAM_FEATHERED_FASTER;
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

    if (e->actionType() == ActionIconType::BEAM_FEATHERED_SLOWER) {
        setAsFeathered(true /*slower*/);
    } else if (e->actionType() == ActionIconType::BEAM_FEATHERED_FASTER) {
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
    qreal _spatium = spatium();
    return PairF(f->py1[idx] / _spatium, f->py2[idx] / _spatium);
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Beam::setBeamPos(const PairF& bp)
{
    if (fragments.empty()) {
        fragments.append(new BeamFragment);
    }
    BeamFragment* f = fragments.back();
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _userModified[idx] = true;
    setGenerated(false);
    qreal _spatium = spatium();
    f->py1[idx] = bp.first * _spatium;
    f->py2[idx] = bp.second * _spatium;
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
    case Pid::BEAM_NO_SLOPE:  return _slope == 0;
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
    qreal lw2 = point(score()->styleS(Sid::beamWidth)) * .5 * mag();
    const LineF* bs = _beamSegments.front();
    double d  = (qAbs(bs->y2() - bs->y1())) / (bs->x2() - bs->x1());
    if (_beamSegments.size() > 1 && d > M_PI / 6.0) {
        d = M_PI / 6.0;
    }
    double ww      = lw2 / sin(M_PI_2 - atan(d));
    qreal _spatium = spatium();

    for (const LineF* beamSegment : qAsConst(_beamSegments)) {
        qreal x = beamSegment->x1();
        qreal y = beamSegment->y1();
        qreal w = beamSegment->x2() - x;
        int n   = (d < 0.01) ? 1 : int(ceil(w / _spatium));

        qreal s = (beamSegment->y2() - y) / w;
        w /= n;
        for (int i = 1; i <= n; ++i) {
            qreal y2 = y + w * s;
            qreal yn, ys;
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
    case BeamMode::BEGIN:
        return ActionIconType::BEAM_START;
    case BeamMode::MID:
        return ActionIconType::BEAM_MID;
    case BeamMode::NONE:
        return ActionIconType::BEAM_NONE;
    case BeamMode::BEGIN32:
        return ActionIconType::BEAM_BEGIN_32;
    case BeamMode::BEGIN64:
        return ActionIconType::BEAM_BEGIN_64;
    case BeamMode::AUTO:
        return ActionIconType::BEAM_AUTO;
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
    qreal dy = ed.pos.y() - ed.lastPos.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = fragments[bed->editFragment];

    qreal y1 = f->py1[idx];
    qreal y2 = f->py2[idx];

    y1 += dy;
    y2 += dy;

    qreal _spatium = spatium();
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
    qreal ydiff = 100000000.0;
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    int i = 0;
    for (BeamFragment* f : qAsConst(fragments)) {
        qreal d = fabs(f->py1[idx] - pt.y());
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
