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

#include "translation.h"
#include "draw/transform.h"
#include "draw/pen.h"
#include "draw/brush.h"
#include "style/style.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "layout/layouttremolo.h"

#include "score.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "segment.h"
#include "stem.h"

using namespace mu;
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

static const char* tremoloName[] = {
    QT_TRANSLATE_NOOP("Tremolo", "Eighth through stem"),
    QT_TRANSLATE_NOOP("Tremolo", "16th through stem"),
    QT_TRANSLATE_NOOP("Tremolo", "32nd through stem"),
    QT_TRANSLATE_NOOP("Tremolo", "64th through stem"),
    QT_TRANSLATE_NOOP("Tremolo", "Buzz roll"),
    QT_TRANSLATE_NOOP("Tremolo", "Eighth between notes"),
    QT_TRANSLATE_NOOP("Tremolo", "16th between notes"),
    QT_TRANSLATE_NOOP("Tremolo", "32nd between notes"),
    QT_TRANSLATE_NOOP("Tremolo", "64th between notes")
};

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

qreal Tremolo::chordMag() const
{
    return explicitParent() ? toChord(explicitParent())->chordMag() : 1.0;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Tremolo::mag() const
{
    return parentItem() ? parentItem()->mag() : 1.0;
}

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

qreal Tremolo::minHeight() const
{
    const qreal sw = score()->styleS(Sid::tremoloStrokeWidth).val() * chordMag();
    const qreal td = score()->styleS(Sid::tremoloDistance).val() * chordMag();
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
        qreal x = 0.0;     // bbox().width() * .25;
        Pen pen(curColor(), point(score()->styleS(Sid::stemWidth)));
        painter->setPen(pen);
        const qreal sp = spatium();
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

void Tremolo::spatiumChanged(qreal oldValue, qreal newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::localSpatiumChanged(qreal oldValue, qreal newValue)
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

PainterPath Tremolo::basePath() const
{
    if (isBuzzRoll()) {
        return PainterPath();
    }

    const qreal sp = spatium() * chordMag();

    // overall width of two-note tremolos should not be changed if chordMag() isn't 1.0
    qreal w2  = sp * score()->styleS(Sid::tremoloWidth).val() * .5 / (twoNotes() ? chordMag() : 1.0);
    qreal nw2 = w2 * score()->styleD(Sid::tremoloStrokeLengthMultiplier);
    qreal lw  = sp * score()->styleS(Sid::tremoloStrokeWidth).val();
    qreal td  = sp * score()->styleS(Sid::tremoloDistance).val();

    PainterPath ppath;

    // first line
    if (explicitParent() && twoNotes() && (_style == TremoloStyle::DEFAULT)) {
        ppath.addRect(-nw2, 0.0, 2.0 * nw2, lw);
    } else {
        ppath.addRect(-w2, 0.0, 2.0 * w2, lw);
    }

    qreal ty = td;

    // other lines
    for (int i = 1; i < _lines; i++) {
        if (explicitParent() && twoNotes() && (_style != TremoloStyle::TRADITIONAL)) {
            ppath.addRect(-nw2, ty, 2.0 * nw2, lw);
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

void Tremolo::layoutOneNoteTremolo(qreal x, qreal y, qreal h, qreal spatium)
{
    Q_ASSERT(!twoNotes());

    bool up = chord()->up();
    int upValue = up ? -1 : 1;

    qreal yOffset = h - score()->styleMM(Sid::tremoloOutSidePadding).val();

    int beams = chord()->beams();
    if (chord()->hook()) {
        yOffset -= up ? 1.5 * spatium : 1 * spatium;
        yOffset -= beams >= 2 ? 0.5 * spatium : 0.0;
    } else if (beams) {
        yOffset -= beams * (score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) * spatium;
        yOffset += beams == 0 ? 0.0 : 0.25 * spatium;
    }
    yOffset -= isBuzzRoll() && up ? 0.5 * spatium : 0.0;
    yOffset *= upValue;
    yOffset -= up ? 0.0 : minHeight() * spatium;

    y += yOffset;

    if (up) {
        qreal height = isBuzzRoll() ? 0 : minHeight();
        y = qMin(y, ((staff()->lines(tick()) - 1) - height) * spatium);
    } else {
        y = qMax(y, 0.0);
    }
    setPos(x, y);
}

//---------------------------------------------------------
//   layoutTwoNotesTremolo
//---------------------------------------------------------

void Tremolo::layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal spatium)
{
    const bool defaultStyle = (!customStyleApplicable()) || (_style == TremoloStyle::DEFAULT);
    const bool isTraditionalAlternate = (_style == TremoloStyle::TRADITIONAL_ALTERNATE);

    //---------------------------------------------------
    //   Step 1: Calculate the position of the tremolo (x, y)
    //---------------------------------------------------

    y += (h - bbox().height()) * .5;

    Stem* stem1 = _chord1->stem();
    Stem* stem2 = _chord2->stem();

    // compute the y coordinates of the tips of the stems
    qreal y1, y2;
    qreal firstChordStaffY;

    if (stem2 && stem1) {
        // stemPageYOffset variable is used for the case when the first
        // chord is cross-staff
        firstChordStaffY = stem1->pagePos().y() - stem1->y();      // y coordinate of the staff of the first chord
        y1 = stem1->y() + stem1->p2().y();
        y2 = stem2->pagePos().y() - firstChordStaffY + stem2->p2().y();      // ->p2().y() is better than ->stemLen()
    } else {
        firstChordStaffY = _chord1->pagePos().y() - _chord1->y();      // y coordinate of the staff of the first chord
        const std::pair<qreal, qreal> extendedLen
            = LayoutTremolo::extendedStemLenWithTwoNoteTremolo(this, _chord1->defaultStemLength(), _chord2->defaultStemLength());
        y1 = _chord1->stemPos().y() - firstChordStaffY + extendedLen.first;
        y2 = _chord2->stemPos().y() - firstChordStaffY + extendedLen.second;
    }

    qreal lw = spatium * score()->styleS(Sid::tremoloStrokeWidth).val();
    if (_chord1->beams() == 0 && _chord2->beams() == 0) {
        // improve the case when one stem is up and another is down
        if (defaultStyle && _chord1->up() != _chord2->up() && !crossStaffBeamBetween()) {
            qreal meanNote1Y = .5
                               * (_chord1->upNote()->pagePos().y() - firstChordStaffY + _chord1->downNote()->pagePos().y()
                                  - firstChordStaffY);
            qreal meanNote2Y = .5
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
    qreal x2 = _chord2->stemPosBeam().x();
    if (stem2) {
        if (defaultStyle && _chord2->up()) {
            x2 -= stem2->lineWidthMag();
        } else if (!defaultStyle && !_chord2->up()) {
            x2 += stem2->lineWidthMag();
        }
    }
    qreal x1 = _chord1->stemPosBeam().x();
    if (stem1) {
        if (defaultStyle && !_chord1->up()) {
            x1 += stem1->lineWidthMag();
        } else if (!defaultStyle && _chord1->up()) {
            x1 -= stem1->lineWidthMag();
        }
    }

    x = (x1 + x2) * .5 - _chord1->pagePos().x();

    //---------------------------------------------------
    //   Step 2: Stretch the tremolo strokes horizontally
    //    from the form of a one-note tremolo (done in basePath())
    //    to that of a two-note tremolo according to the distance between the two chords
    //---------------------------------------------------

    Transform xScaleTransform;
    const qreal H_MULTIPLIER = score()->styleD(Sid::tremoloStrokeLengthMultiplier);
    // TODO const qreal MAX_H_LENGTH = spatium * score()->styleS(Sid::tremoloBeamLengthMultiplier).val();
    const qreal MAX_H_LENGTH = spatium * 12.0;

    const qreal defaultLength = qMin(H_MULTIPLIER * (x2 - x1), MAX_H_LENGTH);
    qreal xScaleFactor = defaultStyle ? defaultLength / H_MULTIPLIER : (x2 - x1);
    const qreal w2 = spatium * score()->styleS(Sid::tremoloWidth).val() * .5;
    xScaleFactor /= (2.0 * w2);

    xScaleTransform.scale(xScaleFactor, 1.0);
    path = xScaleTransform.map(path);

    //---------------------------------------------------
    //   Step 3: Calculate the adjustment of the position of the tremolo
    //    if the chords are connected by a beam so as not to collide with it
    //---------------------------------------------------

    qreal beamYOffset = 0.0;

    if (_chord1->beams() == _chord2->beams() && _chord1->beam()) {
        int beams = _chord1->beams();
        qreal beamHalfLineWidth = point(score()->styleS(Sid::beamWidth)) * .5 * chordMag();
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
    qreal dy = y2 - y1;
    qreal dx = x2 - x1;
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
        dy = qMin(qMax(dy, -1.0 * spatium / defaultLength * dx), 1.0 * spatium / defaultLength * dx);
    }
    qreal ds = dy / dx;
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
    qreal x, y, h;
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
        h = score()->styleMM(Sid::tremoloNoteSidePadding).val() + bbox().height();
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

String Tremolo::subtypeName() const
{
    return mtrc("engraving", tremoloName[subtype()]);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Tremolo::accessibleInfo() const
{
    return String("%1: %2").arg(EngravingItem::accessibleInfo(), subtypeName());
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
