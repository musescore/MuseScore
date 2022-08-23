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

#include "chordline.h"

#include "rw/xml.h"
#include "types/typesconv.h"

#include "chord.h"
#include "measure.h"
#include "system.h"
#include "note.h"
#include "score.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Chord* parent)
    : EngravingItem(ElementType::CHORDLINE, parent, ElementFlag::MOVABLE)
{
    modified = false;
    _chordLineType = ChordLineType::NOTYPE;
    _straight = false;
    _lengthX = 0.0;
    _lengthY = 0.0;
    _note = nullptr;
}

ChordLine::ChordLine(const ChordLine& cl)
    : EngravingItem(cl)
{
    path     = cl.path;
    modified = cl.modified;
    _chordLineType = cl._chordLineType;
    _straight = cl._straight;
    _lengthX = cl._lengthX;
    _lengthY = cl._lengthY;
    _note = cl._note;
}

//---------------------------------------------------------
//   setChordLineType
//---------------------------------------------------------

void ChordLine::setChordLineType(ChordLineType st)
{
    _chordLineType = st;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ChordLine::layout()
{
    if (!modified) {
        double x2 = 0;
        double y2 = 0;
        double horBaseLength = 1.2 * _baseLength; // let the symbols extend a bit more horizontally
        x2 += isToTheLeft() ? -horBaseLength : horBaseLength;
        y2 += isBelow() ? _baseLength : -_baseLength;
        if (_chordLineType != ChordLineType::NOTYPE) {
            path = PainterPath();
            if (!isToTheLeft()) {
                if (_straight) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(x2 / 2, 0.0, x2, y2 / 2, x2, y2);
                }
            } else {
                if (_straight) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(0.0, y2 / 2, x2 / 2, y2, x2, y2);
                }
            }
        }
    }

    double _spatium = spatium();
    if (explicitParent()) {
        Note* note = nullptr;

        if (_note) {
            note = chord()->findNote(_note->pitch());
        }

        if (!note) {
            note = chord()->upNote();
        }

        double x = 0.0;
        double y = note->pos().y();
        double horOffset = 0.33 * spatium(); // one third of a space away from the note
        double vertOffset = 0.25 * spatium(); // one quarter of a space from the center line
        // Get chord shape
        Shape chordShape = chord()->shape();
        // ...but remove chordLines, otherwise we are spacing chordLines against themselves
        auto iter = chordShape.begin();
        while (iter != chordShape.end()) {
            if (iter->toItem && iter->toItem->isChordLine()) {
                iter = chordShape.erase(iter);
            } else {
                ++iter;
            }
        }
        x += isToTheLeft() ? -chordShape.left() - horOffset : chordShape.right() + horOffset;
        y += isBelow() ? vertOffset : -vertOffset;
        setPos(x, y);
    } else {
        setPos(0.0, 0.0);
    }
    RectF r = path.boundingRect();
    int x1, y1, width, height = 0;

    x1 = r.x() * _spatium;
    y1 = r.y() * _spatium;
    width = r.width() * _spatium;
    height = r.height() * _spatium;
    bbox().setRect(x1, y1, width, height);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordLine::read(XmlReader& e)
{
    path = PainterPath();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Path") {
            path = PainterPath();
            PointF curveTo;
            PointF p1;
            int state = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView nextTag(e.name());
                if (nextTag == "Element") {
                    int type = e.intAttribute("type");
                    double x  = e.doubleAttribute("x");
                    double y  = e.doubleAttribute("y");
                    switch (PainterPath::ElementType(type)) {
                    case PainterPath::ElementType::MoveToElement:
                        path.moveTo(x, y);
                        break;
                    case PainterPath::ElementType::LineToElement:
                        path.lineTo(x, y);
                        break;
                    case PainterPath::ElementType::CurveToElement:
                        curveTo.rx() = x;
                        curveTo.ry() = y;
                        state = 1;
                        break;
                    case PainterPath::ElementType::CurveToDataElement:
                        if (state == 1) {
                            p1.rx() = x;
                            p1.ry() = y;
                            state = 2;
                        } else if (state == 2) {
                            path.cubicTo(curveTo, p1, PointF(x, y));
                            state = 0;
                        }
                        break;
                    }
                    e.skipCurrentElement();           //needed to go to next EngravingItem in Path
                } else {
                    e.unknown();
                }
            }
            modified = true;
        } else if (tag == "subtype") {
            setChordLineType(TConv::fromXml(e.readAsciiText(), ChordLineType::NOTYPE));
        } else if (tag == "straight") {
            setStraight(e.readInt());
        } else if (tag == "lengthX") {
            setLengthX(e.readInt());
        } else if (tag == "lengthY") {
            setLengthY(e.readInt());
        } else if (tag == "offset" && score()->mscVersion() < 400) { // default positions has changed in 4.0 so ignore previous offset
            e.skipCurrentElement();
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordLine::write(XmlWriter& xml) const
{
    xml.startElement(this);
    writeProperty(xml, Pid::CHORD_LINE_TYPE);
    writeProperty(xml, Pid::CHORD_LINE_STRAIGHT);
    xml.tag("lengthX", _lengthX, 0.0);
    xml.tag("lengthY", _lengthY, 0.0);
    EngravingItem::writeProperties(xml);
    if (modified) {
        size_t n = path.elementCount();
        xml.startElement("Path");
        for (size_t i = 0; i < n; ++i) {
            const PainterPath::Element& e = path.elementAt(i);
            xml.tag("Element", { { "type", int(e.type) }, { "x", e.x }, { "y", e.y } });
        }
        xml.endElement();
    }
    xml.endElement();
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void ChordLine::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    double _spatium = spatium();
    painter->scale(_spatium, _spatium);
    painter->setPen(Pen(curColor(), .15, PenStyle::SolidLine));
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(path);
    painter->scale(1.0 / _spatium, 1.0 / _spatium);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void ChordLine::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);

    eed->pushProperty(Pid::PATH);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void ChordLine::editDrag(EditData& ed)
{
    auto n = path.elementCount();
    PainterPath p;
    double sp = spatium();
    _lengthX += ed.delta.x();
    _lengthY += ed.delta.y();

    // used to limit how grips can affect the slide, stops the user from being able to turn one kind of slide into another
    int slideBoundary = 5;
    if ((_chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::FALL) && _lengthY < -slideBoundary) {
        _lengthY = -slideBoundary;
    } else if ((_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT) && _lengthX < -slideBoundary) {
        _lengthX = -slideBoundary;
    } else if ((_chordLineType == ChordLineType::DOIT || _chordLineType == ChordLineType::SCOOP) && _lengthY > slideBoundary) {
        _lengthY = slideBoundary;
    } else if ((_chordLineType == ChordLineType::SCOOP || _chordLineType == ChordLineType::PLOP) && _lengthX > slideBoundary) {
        _lengthX = slideBoundary;
    }

    double dx = ed.delta.x() / sp;
    double dy = ed.delta.y() / sp;
    for (size_t i = 0; i < n; ++i) {
        const PainterPath::Element& e = (_straight ? path.elementAt(1) : path.elementAt(i));
        if (_straight) {
            if (i > 0) {
                break;
            }
            // check the gradient of the line
            const PainterPath::Element& startPoint = path.elementAt(0);
            if ((_chordLineType == ChordLineType::FALL && (e.x + dx < startPoint.x || e.y + dy < startPoint.y))
                || (_chordLineType == ChordLineType::DOIT && (e.x + dx < startPoint.x || e.y + dy > startPoint.y))
                || (_chordLineType == ChordLineType::SCOOP && (e.x + dx > startPoint.x || e.y + dy < startPoint.y))
                || (_chordLineType == ChordLineType::PLOP && (e.x + dx > startPoint.x || e.y + dy > startPoint.y))) {
                return;
            }
        }

        double x = e.x;
        double y = e.y;
        if (ed.curGrip == Grip(i)) {
            x += dx;
            y += dy;
        }
        switch (e.type) {
        case PainterPath::ElementType::CurveToDataElement:
            break;
        case PainterPath::ElementType::MoveToElement:
            p.moveTo(x, y);
            break;
        case PainterPath::ElementType::LineToElement:
            p.lineTo(x, y);
            break;
        case PainterPath::ElementType::CurveToElement:
        {
            double x2 = path.elementAt(i + 1).x;
            double y2 = path.elementAt(i + 1).y;
            double x3 = path.elementAt(i + 2).x;
            double y3 = path.elementAt(i + 2).y;
            if (Grip(i + 1) == ed.curGrip) {
                x2 += dx;
                y2 += dy;
            } else if (Grip(i + 2) == ed.curGrip) {
                x3 += dx;
                y3 += dy;
            }
            p.cubicTo(x, y, x2, y2, x3, y3);
            i += 2;
        }
        break;
        }
    }
    path = p;
    modified = true;
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> ChordLine::gripsPositions(const EditData&) const
{
    double sp = spatium();
    auto n   = path.elementCount();
    PointF cp(pagePos());
    if (_straight) {
        // limit the number of grips to one
        double offset = 0.5 * sp;
        PointF p;

        if (_chordLineType == ChordLineType::FALL) {
            p = PointF(offset, -offset);
        } else if (_chordLineType == ChordLineType::DOIT) {
            p = PointF(offset, offset);
        } else if (_chordLineType == ChordLineType::SCOOP) {
            p = PointF(-offset, offset);
        } else if (_chordLineType == ChordLineType::PLOP) {
            p = PointF(-offset, -offset);
        }

        // translate on the length and height - stops the grips from going past boundaries of slide
        p += (cp + PointF(path.elementAt(1).x * sp, path.elementAt(1).y * sp));
        return { p };
    } else {
        std::vector<PointF> grips(n);
        for (size_t i = 0; i < n; ++i) {
            grips[i] = cp + PointF(path.elementAt(i).x * sp, path.elementAt(i).y * sp);
        }
        return grips;
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String ChordLine::accessibleInfo() const
{
    String rez = EngravingItem::accessibleInfo();
    if (chordLineType() != ChordLineType::NOTYPE) {
        rez = String(u"%1: %2").arg(rez, TConv::translatedUserName(chordLineType()));
    }
    return rez;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue ChordLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PATH:
        return PropertyValue::fromValue(path);
    case Pid::CHORD_LINE_TYPE:
        return int(_chordLineType);
    case Pid::CHORD_LINE_STRAIGHT:
        return _straight;
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool ChordLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PATH:
        path = val.value<PainterPath>();
        break;
    case Pid::CHORD_LINE_TYPE:
        setChordLineType(ChordLineType(val.toInt()));
        break;
    case Pid::CHORD_LINE_STRAIGHT:
        setStraight(val.toBool());
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

PropertyValue ChordLine::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::CHORD_LINE_STRAIGHT:
        return false;
    default:
        break;
    }
    return EngravingItem::propertyDefault(pid);
}
}
