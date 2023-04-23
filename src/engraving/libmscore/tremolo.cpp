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

#include "tremolo.h"

#include "draw/types/brush.h"
#include "draw/types/pen.h"
#include "draw/types/transform.h"

#include "style/style.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "layout/layouttremolo.h"

#include "beam.h"
#include "chord.h"
#include "measure.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "stem.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   tremoloStyle
//---------------------------------------------------------

static const ElementStyle tremoloStyle {
    { Sid::tremoloStyle, Pid::TREMOLO_STYLE }
};

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

Tremolo::Tremolo(Chord* parent)
    : EngravingItem(ElementType::TREMOLO, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&tremoloStyle);
}

Tremolo::Tremolo(const Tremolo& t)
    : EngravingItem(t)
{
    setTremoloType(t.tremoloType());
    _chord1       = t.chord1();
    _chord2       = t.chord2();
    _durationType = t._durationType;
}

void Tremolo::setParent(Chord* ch)
{
    EngravingItem::setParent(ch);
}

//---------------------------------------------------------
//   chordMag
//---------------------------------------------------------

double Tremolo::chordMag() const
{
    return explicitParent() ? toChord(explicitParent())->intrinsicMag() : 1.0;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Tremolo::mag() const
{
    return parentItem() ? parentItem()->mag() : 1.0;
}

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

double Tremolo::minHeight() const
{
    const double sw = score()->styleS(Sid::tremoloStrokeWidth).val() * chordMag();
    const double td = score()->styleS(Sid::tremoloDistance).val() * chordMag();
    return (lines() - 1) * td + sw;
}

void Tremolo::createBeamSegments()
{
    // TODO: This should be a style setting, to replace tremoloStrokeLengthMultiplier
    static constexpr double stemGapSp = 1.0;
    const bool defaultStyle = (!customStyleApplicable()) || (_style == TremoloStyle::DEFAULT);

    DeleteAll(_beamSegments);
    _beamSegments.clear();
    if (!twoNotes()) {
        return;
    }
    bool _isGrace = _chord1->isGrace();
    PointF startAnchor = _layoutInfo.startAnchor() - PointF(0., pagePos().y());
    PointF endAnchor = _layoutInfo.endAnchor() - PointF(0., pagePos().y());

    // inset trem from stems for default style
    double slope = (endAnchor.y() - startAnchor.y()) / (endAnchor.x() - startAnchor.x());
    double gapSp = stemGapSp;
    if (defaultStyle) {
        // we can eat into the stemGapSp margin if the anchorpoints are sufficiently close together
        double widthSp = (endAnchor.x() - startAnchor.x()) / spatium() - (stemGapSp * 2);
        if (!RealIsEqualOrMore(widthSp, 0.6)) {
            // tremolo beam is too short; we can eat into the gap spacing a little
            gapSp = std::max(stemGapSp - ((0.6 - widthSp) * 0.5), 0.4);
        }
    } else {
        gapSp = 0.0;
    }
    double offset = gapSp * spatium();
    startAnchor.rx() += offset;
    endAnchor.rx() -= offset;
    startAnchor.ry() += offset * slope;
    endAnchor.ry() -= offset * slope;
    BeamSegment* mainStroke = new BeamSegment(this);
    mainStroke->level = 0;
    mainStroke->line = LineF(startAnchor, endAnchor);
    _beamSegments.push_back(mainStroke);
    double bboxTop = _up ? std::min(mainStroke->line.y1(), mainStroke->line.y2()) : std::max(mainStroke->line.y1(), mainStroke->line.y2());
    double halfWidth = score()->styleMM(Sid::beamWidth).val() / 2. * (_up ? -1. : 1.);
    RectF bbox = RectF(mainStroke->line.x1(), bboxTop + halfWidth, mainStroke->line.x2() - mainStroke->line.x1(),
                       std::abs(mainStroke->line.y2() - mainStroke->line.y1()) - halfWidth * 2.);
    PointF beamOffset = PointF(0., (_up ? 1 : -1) * spatium() * (score()->styleB(Sid::useWideBeams) ? 1. : 0.75));
    beamOffset.setY(beamOffset.y() * mag() * (_isGrace ? score()->styleD(Sid::graceNoteMag) : 1.));
    for (int i = 1; i < lines(); ++i) {
        BeamSegment* stroke = new BeamSegment(this);
        stroke->level = i;
        stroke->line = LineF(startAnchor + (beamOffset * (double)i), endAnchor + (beamOffset * (double)i));
        _beamSegments.push_back(stroke);
        bbox.unite(bbox.translated(0., beamOffset.y() * (double)i));
    }
    setbbox(bbox);

    // size stems properly
    if (_chord1->stem() && _chord2->stem() && !(_chord1->beam() && _chord1->beam() == _chord2->beam())) {
        // we don't need to do anything if these chords are part of the same beam--their stems are taken care of
        // by the beam layout
        int beamSpacing = score()->styleB(Sid::useWideBeams) ? 4 : 3;
        bool cross = chord1()->staffMove() != chord2()->staffMove();
        for (ChordRest* cr : { _chord1, _chord2 }) {
            Chord* chord = toChord(cr);
            double addition = 0.0;
            if (cross && cr->staffMove() != 0 && lines() > 1) {
                // need to adjust further for beams on the opposite side
                addition += (lines() - 1.) * beamSpacing / 4. * spatium() * mag();
            }
            // calling extendStem with addition 0.0 still sizes the stem to the manually adjusted height of the trem.
            _layoutInfo.extendStem(chord, addition);
        }
    }
}

//---------------------------------------------------------
//   chordBeamAnchor
//---------------------------------------------------------

PointF Tremolo::chordBeamAnchor(const ChordRest* chord, BeamTremoloLayout::ChordBeamAnchorType anchorType) const
{
    return _layoutInfo.chordBeamAnchor(chord, anchorType);
}

double Tremolo::beamWidth() const
{
    return _layoutInfo.beamWidth();
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Tremolo::drag(EditData& ed)
{
    if (!twoNotes()) {
        return EngravingItem::drag(ed);
    }
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double dy = ed.pos.y() - ed.lastPos.y();

    double y1 = _beamFragment.py1[idx];
    double y2 = _beamFragment.py2[idx];

    y1 += dy;
    y2 += dy;

    double _spatium = spatium();
    // Because of the logic in Tremolo::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();

    return canvasBoundingRect();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    if (isBuzzRoll()) {
        painter->setPen(curColor());
        drawSymbol(SymId::buzzRoll, painter);
    } else if (!twoNotes() || !explicitParent()) {
        painter->setBrush(Brush(curColor()));
        painter->setNoPen();
        painter->drawPath(path);
    } else if (twoNotes() && !_beamSegments.empty()) {
        // two-note trems act like beams

        // make beam thickness independent of slant
        // (expression can be simplified?)
        const LineF bs = _beamSegments.front()->line;
        double d = (std::abs(bs.y2() - bs.y1())) / (bs.x2() - bs.x1());
        if (_beamSegments.size() > 1 && d > M_PI / 6.0) {
            d = M_PI / 6.0;
        }
        double ww = (score()->styleMM(Sid::beamWidth).val() / 2.0) / sin(M_PI_2 - atan(d));
        painter->setBrush(Brush(curColor()));
        painter->setNoPen();
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
    // for palette
    if (!explicitParent() && !twoNotes()) {
        double x = 0.0;     // bbox().width() * .25;
        Pen pen(curColor(), point(score()->styleS(Sid::stemWidth)));
        painter->setPen(pen);
        const double sp = spatium();
        if (isBuzzRoll()) {
            painter->drawLine(LineF(x, -sp, x, bbox().bottom() + sp));
        } else {
            painter->drawLine(LineF(x, -sp * .5, x, path.boundingRect().height() + sp));
        }
    }
}

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(TremoloType t)
{
    _tremoloType = t;
    switch (tremoloType()) {
    case TremoloType::R16:
    case TremoloType::C16:
        _lines = 2;
        break;
    case TremoloType::R32:
    case TremoloType::C32:
        _lines = 3;
        break;
    case TremoloType::R64:
    case TremoloType::C64:
        _lines = 4;
        break;
    default:
        _lines = 1;
        break;
    }

    styleChanged();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Tremolo::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::localSpatiumChanged(double oldValue, double newValue)
{
    EngravingItem::localSpatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::styleChanged()
{
    EngravingItem::styleChanged();
    computeShape();
}

//---------------------------------------------------------
//   basePath
//---------------------------------------------------------

PainterPath Tremolo::basePath(double stretch) const
{
    if (isBuzzRoll()) {
        return PainterPath();
    }
    bool tradAlternate = twoNotes() && _style == TremoloStyle::TRADITIONAL_ALTERNATE;
    if (tradAlternate && RealIsEqual(stretch, 0.)) {
        // this shape will have to be constructed after the stretch
        // is known
        return PainterPath();
    }

    // TODO: This should be a style setting, to replace tremoloStrokeLengthMultiplier
    static constexpr double stemGapSp = 0.65;

    const double sp = spatium() * chordMag();

    // overall width of two-note tremolos should not be changed if chordMag() isn't 1.0
    double w2  = sp * score()->styleS(Sid::tremoloWidth).val() * .5 / (twoNotes() ? chordMag() : 1.0);
    double lw  = sp * score()->styleS(Sid::tremoloStrokeWidth).val();
    double td  = sp * score()->styleS(Sid::tremoloDistance).val();

    PainterPath ppath;

    // first line
    ppath.addRect(-w2, 0.0, 2.0 * w2, lw);
    double ty = td;

    // other lines
    for (int i = 1; i < _lines; i++) {
        if (tradAlternate) {
            double stemWidth1 = _chord1->stem()->lineWidthMag() / stretch;
            double stemWidth2 = _chord2->stem()->lineWidthMag() / stretch;
            double inset = (stemGapSp * spatium()) / stretch;

            ppath.addRect(-w2 + inset + stemWidth1, ty,
                          2.0 * w2 - (inset * 2.) - (stemWidth2 + stemWidth1), lw);
        } else {
            ppath.addRect(-w2, ty, 2.0 * w2, lw);
        }
        ty += td;
    }

    if (!explicitParent() || !twoNotes()) {
        // for the palette or for one-note tremolos
        Transform shearTransform;
        shearTransform.shear(0.0, -(lw / 2.0) / w2);
        ppath = shearTransform.map(ppath);
    }

    return ppath;
}

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void Tremolo::computeShape()
{
    if (explicitParent() && twoNotes()) {
        return;     // cannot compute shape here, should be done at layout stage
    }
    if (isBuzzRoll()) {
        setbbox(symBbox(SymId::buzzRoll));
    } else {
        path = basePath();
        setbbox(path.boundingRect());
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tremolo::reset()
{
    if (userModified()) {
        //undoChangeProperty(Pid::BEAM_POS, PropertyValue::fromValue(beamPos()));
        undoChangeProperty(Pid::USER_MODIFIED, false);
    }
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    resetProperty(Pid::BEAM_NO_SLOPE);
    setGenerated(true);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Tremolo::pagePos() const
{
    EngravingObject* e = explicitParent();
    while (e && (!e->isSystem() && e->explicitParent())) {
        e = e->explicitParent();
    }
    if (!e || !e->isSystem()) {
        return pos();
    }
    System* s = toSystem(e);
    double yp = y() + s->staff(staffIdx())->y() + s->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   layoutOneNoteTremolo
//---------------------------------------------------------

void Tremolo::layoutOneNoteTremolo(double x, double y, double h, double spatium)
{
    assert(!twoNotes());

    bool up = chord()->up();
    int upValue = up ? -1 : 1;
    double mag = chord()->intrinsicMag();
    spatium *= mag;

    double yOffset = h - score()->styleMM(Sid::tremoloOutSidePadding).val() * mag;

    int beams = chord()->beams();
    if (chord()->hook()) {
        // allow for space at the hook side of the stem (yOffset)
        // straight flags and traditional flags have different requirements because of their slopes
        // away from the stem. Straight flags have a shallower slope and a lot more space in general
        // so we can place the trem higher in that case
        bool straightFlags = score()->styleB(Sid::useStraightNoteFlags);
        if (straightFlags) {
            yOffset -= 0.75 * spatium;
        } else {
            // up-hooks and down-hooks are shaped differently
            yOffset -= up ? 1.5 * spatium : 1.0 * spatium;
        }
        // we need an additional offset for beams > 2 since those beams extend outwards and we don't want to adjust for that
        double beamOffset = straightFlags ? 0.75 : 0.5;
        yOffset -= beams >= 2 ? beamOffset * spatium : 0.0;
    } else if (beams) {
        yOffset -= (beams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) - 0.25) * spatium;
    }
    yOffset -= isBuzzRoll() && up ? 0.5 * spatium : 0.0;
    yOffset -= up ? 0.0 : minHeight() * spatium / mag;
    yOffset *= upValue;

    y += yOffset;

    if (up) {
        double height = isBuzzRoll() ? 0 : minHeight();
        y = std::min(y, ((staff()->lines(tick()) - 1) - height) * spatium / mag);
    } else {
        y = std::max(y, 0.0);
    }
    setPos(x, y);
}

//---------------------------------------------------------
//   layoutTwoNotesTremolo
//---------------------------------------------------------

void Tremolo::layoutTwoNotesTremolo(double x, double y, double h, double spatium)
{
    UNUSED(x);
    UNUSED(y);
    UNUSED(h);
    UNUSED(spatium);

    // make sure both stems are in the same direction
    int up = 0;
    bool isUp = _up;
    if (_chord1->beam() && _chord1->beam() == _chord2->beam()) {
        Beam* beam = _chord1->beam();
        _up = beam->up();
        _direction = beam->beamDirection();
        // stem stuff is already taken care of by the beams
    } else if (!userModified()) {
        // user modified trems will be dealt with later
        bool hasVoices = _chord1->measure()->hasVoices(_chord1->staffIdx(), _chord1->tick(), _chord2->tick() - _chord1->tick());
        if (_chord1->stemDirection() == DirectionV::AUTO && _chord2->stemDirection() == DirectionV::AUTO
            && _chord1->staffMove() == _chord2->staffMove() && !hasVoices) {
            std::vector<int> noteDistances;
            for (int distance : _chord1->noteDistances()) {
                noteDistances.push_back(distance);
            }
            for (int distance : _chord2->noteDistances()) {
                noteDistances.push_back(distance);
            }
            std::sort(noteDistances.begin(), noteDistances.end());
            up = Chord::computeAutoStemDirection(noteDistances);
            isUp = up > 0;
        } else if (_chord1->stemDirection() != DirectionV::AUTO) {
            isUp = _chord1->stemDirection() == DirectionV::UP;
        } else if (_chord2->stemDirection() != DirectionV::AUTO) {
            isUp = _chord2->stemDirection() == DirectionV::UP;
        } else if (_chord1->staffMove() > 0 || _chord2->staffMove() > 0) {
            isUp = false;
        } else if (_chord1->staffMove() < 0 || _chord2->staffMove() < 0) {
            isUp = true;
        } else if (hasVoices) {
            isUp = _chord1->track() % 2 == 0;
        }
        _up = isUp;
        _chord1->setUp(_chord1->staffMove() == 0 ? isUp : !isUp); // if on a different staff, flip stem dir
        _chord2->setUp(_chord2->staffMove() == 0 ? isUp : !isUp);
        _chord1->layoutStem();
        _chord2->layoutStem();
    }

    _layoutInfo = BeamTremoloLayout(this);
    _startAnchor = _layoutInfo.chordBeamAnchor(_chord1, BeamTremoloLayout::ChordBeamAnchorType::Start);
    _endAnchor = _layoutInfo.chordBeamAnchor(_chord2, BeamTremoloLayout::ChordBeamAnchorType::End);
    // deal with manual adjustments here and return
    PropertyValue val = getProperty(Pid::PLACEMENT);
    if (userModified()) {
        int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
        double startY = _beamFragment.py1[idx];
        double endY = _beamFragment.py2[idx];
        if (score()->styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = EngravingItem::spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        startY += pagePos().y();
        endY += pagePos().y();
        _startAnchor.setY(startY);
        _endAnchor.setY(endY);
        _layoutInfo.setAnchors(_startAnchor, _endAnchor);
        _chord1->layoutStem();
        _chord2->layoutStem();
        createBeamSegments();
        return;
    }
    setPosY(0.);
    std::vector<ChordRest*> chordRests{ chord1(), chord2() };
    std::vector<int> notes;
    double mag = 0.0;

    notes.clear();
    for (ChordRest* cr : chordRests) {
        double m = cr->isSmall() ? score()->styleD(Sid::smallNoteMag) : 1.0;
        mag = std::max(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            //int i = chord->staffMove();
            //_minMove = std::min(_minMove, i); todo: investigate this
            //_maxMove = std::max(_maxMove, i);

            for (int distance : chord->noteDistances()) {
                notes.push_back(distance);
            }
        }
    }

    std::sort(notes.begin(), notes.end());
    setMag(mag);
    _layoutInfo.calculateAnchors(chordRests, notes);
    _startAnchor = _layoutInfo.startAnchor();
    _endAnchor = _layoutInfo.endAnchor();
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _beamFragment.py1[idx] = _startAnchor.y() - pagePos().y();
    _beamFragment.py2[idx] = _endAnchor.y() - pagePos().y();
    createBeamSegments();
}

//---------------------------------------------------------
//   defaultStemLengthStart
//---------------------------------------------------------

double Tremolo::defaultStemLengthStart()
{
    return LayoutTremolo::extendedStemLenWithTwoNoteTremolo(this, _chord1->defaultStemLength(), _chord2->defaultStemLength()).first;
}

//---------------------------------------------------------
//   defaultStemLengthEnd
//---------------------------------------------------------

double Tremolo::defaultStemLengthEnd()
{
    return LayoutTremolo::extendedStemLenWithTwoNoteTremolo(this, _chord1->defaultStemLength(), _chord2->defaultStemLength()).second;
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Tremolo::setBeamDirection(DirectionV d)
{
    if (_direction == d) {
        return;
    }

    _direction = d;

    if (d != DirectionV::AUTO) {
        _up = d == DirectionV::UP;
    }
    if (twoNotes()) {
        if (_chord1) {
            _chord1->setStemDirection(d);
        }
        if (_chord2) {
            _chord2->setStemDirection(d);
        }
    } else {
        chord()->setStemDirection(d);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
{
    path = basePath();

    _chord1 = toChord(explicitParent());
    if (!_chord1) {
        // palette
        if (!isBuzzRoll()) {
            const RectF box = path.boundingRect();
            addbbox(RectF(box.x(), box.bottom(), box.width(), spatium()));
        }
        return;
    }

    Note* anchor1 = _chord1->up() ? _chord1->upNote() : _chord1->downNote();
    Stem* stem    = _chord1->stem();
    double x, y, h;
    if (stem) {
        x = stem->pos().x() + stem->width() / 2 * (_chord1->up() ? -1.0 : 1.0);
        y = stem->pos().y();
        h = stem->length();
    } else {
        // center tremolo above note
        x = anchor1->x() + anchor1->headWidth() * 0.5;
        if (!twoNotes()) {
            bool hasMirroredNote = false;
            for (Note* n : _chord1->notes()) {
                if (n->mirror()) {
                    hasMirroredNote = true;
                    break;
                }
            }
            if (hasMirroredNote) {
                x = _chord1->stemPosX();
            }
        }
        y = anchor1->y();
        h = (score()->styleMM(Sid::tremoloNoteSidePadding).val() + bbox().height()) * _chord1->intrinsicMag();
    }

    if (twoNotes()) {
        layoutTwoNotesTremolo(x, y, h, spatium());
    } else {
        layoutOneNoteTremolo(x, y, h, spatium());
    }
}

//---------------------------------------------------------
//   crossStaffBeamBetween
//    Return true if tremolo is two-note cross-staff and beams between staves
//---------------------------------------------------------

bool Tremolo::crossStaffBeamBetween() const
{
    if (!twoNotes()) {
        return false;
    }

    return ((_chord1->staffMove() > _chord2->staffMove()) && _chord1->up() && !_chord2->up())
           || ((_chord1->staffMove() < _chord2->staffMove()) && !_chord1->up() && _chord2->up());
}

void Tremolo::setUserModified(DirectionV d, bool val)
{
    switch (d) {
    case DirectionV::AUTO:
        _userModified[0] = val;
        break;
    case DirectionV::DOWN:
        _userModified[0] = val;
        break;
    case DirectionV::UP:
        _userModified[1] = val;
        break;
    }
}

TDuration Tremolo::durationType() const
{
    return _durationType;
}

void Tremolo::setDurationType(TDuration d)
{
    if (_durationType == d) {
        return;
    }

    _durationType = d;
    styleChanged();
}

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction Tremolo::tremoloLen() const
{
    Fraction f;
    switch (lines()) {
    case 1: f.set(1, 8);
        break;
    case 2: f.set(1, 16);
        break;
    case 3: f.set(1, 32);
        break;
    case 4: f.set(1, 64);
        break;
    }
    return f;
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Tremolo::setBeamPos(const PairF& bp)
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _userModified[idx] = true;
    setGenerated(false);

    double _spatium = spatium();
    _beamFragment.py1[idx] = bp.first * _spatium;
    _beamFragment.py2[idx] = bp.second * _spatium;
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF Tremolo::beamPos() const
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double _spatium = spatium();
    return PairF(_beamFragment.py1[idx] / _spatium, _beamFragment.py2[idx] / _spatium);
}

//---------------------------------------------------------
//   userModified
//---------------------------------------------------------

bool Tremolo::userModified() const
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    return _userModified[idx];
}

//---------------------------------------------------------
//   setUserModified
//---------------------------------------------------------

void Tremolo::setUserModified(bool val)
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    _userModified[idx] = val;
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Tremolo::triggerLayout() const
{
    if (twoNotes() && _chord1 && _chord2) {
        toChordRest(_chord1)->triggerLayout();
        toChordRest(_chord2)->triggerLayout();
    } else {
        EngravingItem::triggerLayout();
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Tremolo::gripsPositions(const EditData&) const
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;

    if (!twoNotes()) {
        return std::vector<PointF>();
    }

    int y = pagePos().y();
    double beamStartX = _startAnchor.x() + _chord1->pageX();
    double beamEndX = _endAnchor.x() + _chord1->pageX(); // intentional--chord1 is start x
    double middleX = (beamStartX + beamEndX) / 2;
    double middleY = (_beamFragment.py1[idx] + y + _beamFragment.py2[idx] + y) / 2;

    return {
        PointF(beamStartX, _beamFragment.py1[idx] + y),
        PointF(beamEndX, _beamFragment.py2[idx] + y),
        PointF(middleX, middleY)
    };
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Tremolo::endEdit(EditData& ed)
{
    EngravingItem::endEdit(ed);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tremolo::editDrag(EditData& ed)
{
    int idx = (_direction == DirectionV::AUTO || _direction == DirectionV::DOWN) ? 0 : 1;
    double dy = ed.delta.y();
    double y1 = _beamFragment.py1[idx];
    double y2 = _beamFragment.py2[idx];

    if (ed.curGrip == Grip::MIDDLE) {
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
//   subtypeName
//---------------------------------------------------------

TranslatableString Tremolo::subtypeUserName() const
{
    return TConv::userName(tremoloType());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Tremolo::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

//---------------------------------------------------------
//   customStyleApplicable
//---------------------------------------------------------

bool Tremolo::customStyleApplicable() const
{
    return twoNotes()
           && (durationType().type() == DurationType::V_HALF)
           && (staffType()->group() != StaffGroup::TAB);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Tremolo::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        return int(_tremoloType);
    case Pid::TREMOLO_STYLE:
        return int(_style);
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tremolo::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        setTremoloType(TremoloType(val.toInt()));
        break;
    case Pid::TREMOLO_STYLE:
        if (customStyleApplicable()) {
            setStyle(TremoloStyle(val.toInt()));
        }
        break;
    case Pid::STEM_DIRECTION:
        setBeamDirection(val.value<DirectionV>());
        break;
    case Pid::USER_MODIFIED:
        setUserModified(val.toBool());
        break;
    case Pid::BEAM_POS:
        if (userModified()) {
            setBeamPos(val.value<PairF>());
        }
        break;
    default:
        return EngravingItem::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Tremolo::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_STYLE:
        return score()->styleI(Sid::tremoloStyle);
    default:
        return EngravingItem::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tremolo::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (chord() && chord()->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
        return;
    }
    EngravingItem::scanElements(data, func, all);
}
}
