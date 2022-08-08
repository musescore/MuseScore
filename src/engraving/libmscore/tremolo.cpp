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

#include "draw/types/transform.h"
#include "draw/types/pen.h"
#include "draw/types/brush.h"
#include "style/style.h"
#include "rw/xml.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "layout/layouttremolo.h"

#include "score.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "segment.h"
#include "stem.h"

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
    return explicitParent() ? toChord(explicitParent())->chordMag() : 1.0;
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

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (isBuzzRoll()) {
        painter->setPen(curColor());
        drawSymbol(SymId::buzzRoll, painter);
    } else {
        painter->setBrush(Brush(curColor()));
        painter->setNoPen();
        painter->drawPath(path);
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
//   layoutOneNoteTremolo
//---------------------------------------------------------

void Tremolo::layoutOneNoteTremolo(double x, double y, double h, double spatium)
{
    assert(!twoNotes());

    bool up = chord()->up();
    int upValue = up ? -1 : 1;
    double mag = chord()->relativeMag();
    spatium *= mag;

    double yOffset = h - score()->styleMM(Sid::tremoloOutSidePadding).val() * mag;

    int beams = chord()->beams();
    if (chord()->hook()) {
        yOffset -= up ? 1.75 * spatium : 1.25 * spatium;
        yOffset -= beams >= 2 ? 0.5 * spatium : 0.0;
    } else if (beams) {
        yOffset -= (beams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) - 0.25) * spatium;
    }
    yOffset -= isBuzzRoll() && up ? 0.5 * spatium : 0.0;
    yOffset -= up ? 0.0 : minHeight() * spatium / mag;
    yOffset *= upValue;

    y += yOffset;

    if (up) {
        double height = isBuzzRoll() ? 0 : minHeight();
        y = std::min(y, ((staff()->lines(tick()) - 1) - height) * spatium);
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
    UNUSED(h);
    const bool defaultStyle = (!customStyleApplicable()) || (_style == TremoloStyle::DEFAULT);
    const bool isTraditionalAlternate = (_style == TremoloStyle::TRADITIONAL_ALTERNATE);

    // TODO: This should be a style setting, to replace tremoloStrokeLengthMultiplier
    static constexpr double stemGapSp = 0.65;

    // make sure both stems are in the same direction
    int up = 0;
    bool isUp = _up;
    if (_chord1->beam() && _chord1->beam() == _chord2->beam()) {
        Beam* beam = _chord1->beam();
        _up = beam->up();
        _direction = beam->beamDirection();
        // stem stuff is already taken care of by the beams
    } else {
        if (_chord1->stemDirection() == DirectionV::AUTO && _chord2->stemDirection() == DirectionV::AUTO
            && _chord1->staffMove() == _chord2->staffMove()) {
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
        } else if (_chord1->staffMove() > 0 || _chord2->staffMove() > 0) {
            isUp = false;
        } else if (_chord1->staffMove() < 0 || _chord2->staffMove() < 0) {
            isUp = true;
        } else if (_chord1->measure()->hasVoices(_chord1->staffIdx(), _chord1->tick(), _chord2->rtick() - _chord1->tick())) {
            isUp = _chord1->track() % 2 == 0;
        } else if (_chord1->stemDirection() != DirectionV::AUTO) {
            isUp = _chord1->stemDirection() == DirectionV::UP;
        } else if (_chord2->stemDirection() != DirectionV::AUTO) {
            isUp = _chord2->stemDirection() == DirectionV::UP;
        }
        _up = isUp;
        _chord1->setUp(_chord1->staffMove() == 0 ? isUp : !isUp); // if on a different staff, flip stem dir
        _chord2->setUp(_chord2->staffMove() == 0 ? isUp : !isUp);
        _chord1->layoutStem();
        _chord2->layoutStem();
    }
    //---------------------------------------------------
    //   Step 1: Calculate the position of the tremolo (x, y)
    //---------------------------------------------------
    Stem* stem1 = _chord1->stem();
    Stem* stem2 = _chord2->stem();

    // compute the y coordinates of the tips of the stems
    double y1, y2;
    double firstChordStaffY;

    if (stem2 && stem1) {
        // stemPageYOffset variable is used for the case when the first
        // chord is cross-staff
        firstChordStaffY = stem1->pagePos().y() - stem1->y();      // y coordinate of the staff of the first chord
        y1 = stem1->y() + stem1->p2().y();
        y2 = stem2->pagePos().y() - firstChordStaffY + stem2->p2().y();      // ->p2().y() is better than ->stemLen()
    } else {
        Note* note1 = _up ? _chord1->downNote() : _chord1->upNote();

        firstChordStaffY = note1->pagePos().y() - note1->y();      // y coordinate of the staff of the first chord
        const std::pair<double, double> extendedLen
            = LayoutTremolo::extendedStemLenWithTwoNoteTremolo(this, _chord1->defaultStemLength(), _chord2->defaultStemLength());
        y1 = _chord1->stemPos().y() - firstChordStaffY + (extendedLen.first * (_up ? -1 : 1));
        y2 = _chord2->stemPos().y() - firstChordStaffY + (extendedLen.second * (_up ? -1 : 1));
    }

    double lw = spatium * score()->styleS(Sid::tremoloStrokeWidth).val();
    if (_chord1->beams() == 0 && _chord2->beams() == 0) {
        // improve the case when one stem is up and another is down
        if (defaultStyle && _chord1->up() != _chord2->up() && !crossStaffBeamBetween()) {
            double meanNote1Y = .5
                                * (_chord1->upNote()->pagePos().y() - firstChordStaffY + _chord1->downNote()->pagePos().y()
                                   - firstChordStaffY);
            double meanNote2Y = .5
                                * (_chord2->upNote()->pagePos().y() - firstChordStaffY + _chord2->downNote()->pagePos().y()
                                   - firstChordStaffY);
            y1 = .5 * (y1 + meanNote1Y);
            y2 = .5 * (y2 + meanNote2Y);
        }
        if (!defaultStyle && _chord1->up() == _chord2->up()) {
            y1 += _chord1->up() ? -lw / 2.0 : lw / 2.0;
            y2 += _chord1->up() ? -lw / 2.0 : lw / 2.0;
        }
    }

    y = (y1 + y2) * .5;
    if (!_chord1->up()) {
        y -= isTraditionalAlternate ? lw * .5 : path.boundingRect().height() * .5;
    }
    if (!_chord2->up()) {
        y -= isTraditionalAlternate ? lw * .5 : path.boundingRect().height() * .5;
    }

    // compute the x coordinates of
    // the inner edge of the stems (default beam style)
    // the outer edge of the stems (non-default beam style)
    double x2 = _chord2->stemPosBeam().x();
    if (!stem2 && _chord2->up()) {
        double nhw = score()->noteHeadWidth();
        if (_chord2->noteType() != NoteType::NORMAL) {
            nhw *= score()->styleD(Sid::graceNoteMag);
        }
        nhw *= _chord2->mag();
        x2 -= nhw;
    } else if (stem2) {
        if (defaultStyle && _chord2->up()) {
            x2 -= stem2->lineWidthMag();
        } else if (!defaultStyle && !_chord2->up()) {
            x2 += stem2->lineWidthMag();
        }
    }

    double x1 = _chord1->stemPosBeam().x();
    if (!stem1 && !_chord1->up()) {
        double nhw = score()->noteHeadWidth();
        if (_chord1->noteType() != NoteType::NORMAL) {
            nhw *= score()->styleD(Sid::graceNoteMag);
        }
        nhw *= _chord1->mag();
        x1 += nhw;
    } else if (stem1) {
        if (defaultStyle && !_chord1->up()) {
            x1 += stem1->lineWidthMag();
        } else if (!defaultStyle && _chord1->up()) {
            x1 -= stem1->lineWidthMag();
        }
    }
    x = (x2 + x1) * .5 - _chord1->pagePos().x();

    double slope = (y2 - y1) / (x2 - x1);
    // add offsets to the x endpoints
    double offset = defaultStyle ? stemGapSp * spatium : 0.; // offset from stems (or original position)
    x2 -= offset; // apply offset horizontally
    x1 += offset;
    // apply offset vertically to maintain the same slope
    y1 += offset * slope;
    y2 -= offset * slope;

    //---------------------------------------------------
    //   Step 2: Stretch the tremolo strokes horizontally
    //    from the form of a one-note tremolo (done in basePath())
    //    to that of a two-note tremolo according to the distance between the two chords
    //---------------------------------------------------

    Transform xScaleTransform;
    const double MAX_H_LENGTH = spatium * 15.0;

    const double defaultLength = std::min(x2 - x1, MAX_H_LENGTH);
    double xScaleFactor = defaultStyle ? defaultLength : (x2 - x1);
    const double origTremWidth = spatium * score()->styleS(Sid::tremoloWidth).val();
    xScaleFactor /= origTremWidth;
    if (_style == TremoloStyle::TRADITIONAL_ALTERNATE) {
        path = basePath(xScaleFactor);
    }

    xScaleTransform.scale(xScaleFactor, 1.0);
    path = xScaleTransform.map(path);

    //---------------------------------------------------
    //   Step 3: Calculate the adjustment of the position of the tremolo
    //    if the chords are connected by a beam so as not to collide with it
    //---------------------------------------------------

    double beamYOffset = 0.0;

    if (_chord1->beams() == _chord2->beams() && _chord1->beam()) {
        int beams = _chord1->beams();
        double beamHalfLineWidth = point(score()->styleS(Sid::beamWidth)) * .5 * chordMag();
        beamYOffset = beams * _chord1->beam()->beamDist() - beamHalfLineWidth;
        if (_chord1->up() != _chord2->up()) {      // cross-staff
            beamYOffset = 2 * beamYOffset + beamHalfLineWidth;
        } else if (!_chord1->up() && !_chord2->up()) {
            beamYOffset = -beamYOffset;
        }
    }

    //---------------------------------------------------
    //   Step 4: Tilt the tremolo strokes according to the stems of the chords
    //---------------------------------------------------

    Transform shearTransform;
    double dy = y2 - y1;
    double dx = x2 - x1;
    if (_chord1->beams() == 0 && _chord2->beams() == 0) {
        if (_chord1->up() && !_chord2->up()) {
            dy -= isTraditionalAlternate ? lw : path.boundingRect().height();
            if (!defaultStyle) {
                dy += lw;
            }
        } else if (!_chord1->up() && _chord2->up()) {
            dy += isTraditionalAlternate ? lw : path.boundingRect().height();
            if (!defaultStyle) {
                dy -= lw;
            }
        }
    }
    // Make tremolo strokes less steep if two chords have the opposite stem directions,
    // except for two cases:
    // 1. The tremolo doesn't have the default style.
    // In this case tremolo strokes should attach to the ends of both stems, so no adjustment needed;
    // 2. The chords are on different staves and the tremolo is between them.
    // The layout should be improved by extending both stems, so changes are not needed here.
    if (_chord1->up() != _chord2->up() && defaultStyle && !crossStaffBeamBetween()) {
        dy = std::min(std::max(dy, -1.0 * spatium / defaultLength * dx), 1.0 * spatium / defaultLength * dx);
    }
    double ds = dy / dx;
    shearTransform.shear(0.0, ds);
    path = shearTransform.map(path);

    //---------------------------------------------------
    //   Step 5: Flip the tremolo strokes if necessary
    //    By default, a TRADITIONAL_ALTERNATE tremolo has its attached-to-stem stroke be above other strokes,
    //    see basePath().
    //    But if both chords have stems facing down,
    //    the tremolo should be flipped to have the attached-to-stem stroke be below other strokes.
    //---------------------------------------------------

    if (isTraditionalAlternate && !_chord1->up() && !_chord2->up()) {
        Transform rotateTransform;
        rotateTransform.translate(0.0, lw * .5);
        rotateTransform.rotate(180);
        rotateTransform.translate(0.0, -lw * .5);
        path = rotateTransform.map(path);
    }

    setbbox(path.boundingRect());
    setPos(x, y + beamYOffset);
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

    _chord1->setStemDirection(d);
    _chord2->setStemDirection(d);
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
        h = (score()->styleMM(Sid::tremoloNoteSidePadding).val() + bbox().height()) * _chord1->relativeMag();
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

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);
    writeProperty(xml, Pid::TREMOLO_TYPE);
    writeProperty(xml, Pid::TREMOLO_STYLE);
    EngravingItem::writeProperties(xml);
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tremolo::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            setTremoloType(TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO));
        }
        // Style needs special handling other than readStyledProperty()
        // to avoid calling customStyleApplicable() in setProperty(),
        // which cannot be called now because durationType() isn't defined yet.
        else if (tag == "strokeStyle") {
            setStyle(TremoloStyle(e.readInt()));
            setPropertyFlags(Pid::TREMOLO_STYLE, PropertyFlags::UNSTYLED);
        } else if (readStyledProperty(e, tag)) {
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
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
