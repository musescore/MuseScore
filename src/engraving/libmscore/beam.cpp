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

#include "actionicon.h"
#include "chord.h"
#include "groups.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "tremolo.h"
#include "tuplet.h"

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

PointF Beam::chordBeamAnchor(const ChordRest* chord, layout::v0::BeamTremoloLayout::ChordBeamAnchorType anchorType) const
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
            double x = chordBeamAnchor(c, layout::v0::BeamTremoloLayout::ChordBeamAnchorType::Middle).x();
            double proportionAlongX = (x - _startAnchor.x()) / width;
            double y = _startAnchor.y() + (proportionAlongX * height);
            y += regularBeams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) * spatium() * (tremUp ? 1. : -1.);
            tremAnchor.y1 = y;
            // find the right-side anchor
            x = chordBeamAnchor(t->chord2(), layout::v0::BeamTremoloLayout::ChordBeamAnchorType::Middle).x();
            proportionAlongX = (x - _startAnchor.x()) / width;
            y = _startAnchor.y() + (proportionAlongX * height);
            y += regularBeams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) * spatium() * (tremUp ? 1. : -1.);
            tremAnchor.y2 = y;
            _tremAnchors.push_back(tremAnchor);
        }
    }
}

void Beam::calcBeamBreaks(const ChordRest* cr, const ChordRest* prevCr, int level, bool& isBroken32, bool& isBroken64) const
{
    BeamMode beamMode = cr->beamMode();

    // get default beam mode -- based on time signature preferences
    const Groups& group = cr->staff()->group(cr->measure()->tick());
    BeamMode defaultBeamMode = group.endBeam(cr, prevCr);

    bool isManuallyBroken32 = level >= 1 && beamMode == BeamMode::BEGIN32;
    bool isManuallyBroken64 = level >= 2 && beamMode == BeamMode::BEGIN64;
    bool isDefaultBroken32 = beamMode == BeamMode::AUTO && level >= 1 && defaultBeamMode == BeamMode::BEGIN32;
    bool isDefaultBroken64 = beamMode == BeamMode::AUTO && level >= 2 && defaultBeamMode == BeamMode::BEGIN64;

    isBroken32 = isManuallyBroken32 || isDefaultBroken32;
    isBroken64 = isManuallyBroken64 || isDefaultBroken64;

    // deal with beam-embedded triplets by breaking beams as if they are their underlying durations
    // note that we use max(hooks, 1) here because otherwise we'd end up breaking the main (level 0) beam for
    // tuplets that take up non-beamed amounts of space (eg. 16th note quintuplets)
    if (level > 0 && prevCr && cr->beamMode() == BeamMode::AUTO) {
        if (cr->tuplet() && cr->tuplet() != prevCr->tuplet()) {
            // this cr starts a tuplet
            int beams = std::max(TDuration(cr->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken32 = level == 1;
                isBroken64 = level >= 2;
            }
        } else if (prevCr->tuplet() && prevCr->tuplet() != cr->tuplet()) {
            // this is a non-tuplet cr that is first after a tuplet
            int beams = std::max(TDuration(prevCr->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken32 = level == 1;
                isBroken64 = level >= 2;
            }
        }
    }
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
    subBoxesCount = std::max(subBoxesCount, 1); // at least one rectangle, of course (avoid division by zero)
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
