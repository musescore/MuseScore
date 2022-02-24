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
#include "chord.h"
#include "measure.h"
#include "system.h"
#include "note.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace Ms {
const char* scorelineNames[] = {
    QT_TRANSLATE_NOOP("Ms", "Fall"),
    QT_TRANSLATE_NOOP("Ms", "Doit"),
    QT_TRANSLATE_NOOP("Ms", "Plop"),
    QT_TRANSLATE_NOOP("Ms", "Scoop"),
};

//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Chord* parent, const ElementType& type)
    : EngravingItem(type, parent, ElementFlag::MOVABLE)
{
    modified = false;
    _chordLineType = ChordLineType::NOTYPE;
    _straight = false;
    _lengthX = 0.0;
    _lengthY = 0.0;
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
        qreal x2 = 0;
        qreal y2 = 0;
        switch (_chordLineType) {
        case ChordLineType::NOTYPE:
            break;
        case ChordLineType::FALL:
            x2 = _initialLength;
            y2 = _initialLength;
            break;
        case ChordLineType::PLOP:
            x2 = -_initialLength;
            y2 = -_initialLength;
            break;
        case ChordLineType::SCOOP:
            x2 = -_initialLength;
            y2 = _initialLength;
            break;
        default:
        case ChordLineType::DOIT:
            x2 = _initialLength;
            y2 = -_initialLength;
            break;
        }
        if (_chordLineType != ChordLineType::NOTYPE) {
            path = PainterPath();
            // chordlines to the right of the note
            if (_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT) {
                if (_straight) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(x2 / 2, 0.0, x2, y2 / 2, x2, y2);
                }
            }
            // chordlines to the left of the note
            else if (_chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::SCOOP) {
                if (_straight) {
                    path.lineTo(x2, y2);
                } else {
                    path.cubicTo(0.0, y2 / 2, x2 / 2, y2, x2, y2);
                }
            }
        }
    }

    qreal _spatium = spatium();
    if (explicitParent()) {
        Note* note = chord()->upNote();
        PointF p(note->pos());
        // chordlines to the right of the note
        if (_chordLineType == ChordLineType::FALL || _chordLineType == ChordLineType::DOIT) {
            setPos(p.x() + note->bboxRightPos() + _spatium * .2, p.y());
        }
        // chordlines to the left of the note
        if (_chordLineType == ChordLineType::PLOP) {
            setPos(p.x() + note->bboxRightPos() * .25, p.y() - note->headHeight() * .75);
        }
        if (_chordLineType == ChordLineType::SCOOP) {
            qreal x = p.x() + (chord()->up() ? note->bboxRightPos() * .25 : _spatium * -.2);
            setPos(x, p.y() + note->headHeight() * .75);
        }
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
        const QStringRef& tag(e.name());
        if (tag == "Path") {
            path = PainterPath();
            PointF curveTo;
            PointF p1;
            int state = 0;
            while (e.readNextStartElement()) {
                const QStringRef& nextTag(e.name());
                if (nextTag == "Element") {
                    int type = e.intAttribute("type");
                    qreal x  = e.doubleAttribute("x");
                    qreal y  = e.doubleAttribute("y");
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
            setChordLineType(ChordLineType(e.readInt()));
        } else if (tag == "straight") {
            setStraight(e.readInt());
        } else if (tag == "lengthX") {
            setLengthX(e.readInt());
        } else if (tag == "lengthY") {
            setLengthY(e.readInt());
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
    xml.startObject(this);
    writeProperty(xml, Pid::CHORD_LINE_TYPE);
    writeProperty(xml, Pid::CHORD_LINE_STRAIGHT);
    xml.tag("lengthX", _lengthX, 0.0);
    xml.tag("lengthY", _lengthY, 0.0);
    EngravingItem::writeProperties(xml);
    if (modified) {
        size_t n = path.elementCount();
        xml.startObject("Path");
        for (size_t i = 0; i < n; ++i) {
            const PainterPath::Element& e = path.elementAt(i);
            xml.tagE(QString("Element type=\"%1\" x=\"%2\" y=\"%3\"")
                     .arg(int(e.type)).arg(e.x).arg(e.y));
        }
        xml.endObject();
    }
    xml.endObject();
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void ChordLine::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    qreal _spatium = spatium();

    if (this->isStraight()) {
        painter->scale(_spatium, _spatium);
        painter->setPen(Pen(curColor(), .15, PenStyle::SolidLine));
        painter->setBrush(BrushStyle::NoBrush);

        PainterPath pathOffset = path;
        qreal offset = 0.5;

        if (_chordLineType == ChordLineType::FALL) {
            pathOffset.translate(offset, -offset);
        } else if (_chordLineType == ChordLineType::DOIT) {
            pathOffset.translate(offset, offset);
        } else if (_chordLineType == ChordLineType::SCOOP) {
            pathOffset.translate(-offset, offset);
        } else if (_chordLineType == ChordLineType::PLOP) {
            pathOffset.translate(-offset, -offset);
        }

        painter->drawPath(pathOffset);
        painter->scale(1.0 / _spatium, 1.0 / _spatium);
    } else {
        painter->scale(_spatium, _spatium);
        painter->setPen(Pen(curColor(), .15, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin));
        painter->setBrush(BrushStyle::NoBrush);
        painter->drawPath(path);
        painter->scale(1.0 / _spatium, 1.0 / _spatium);
    }
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
    qreal sp = spatium();
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

    qreal dx = ed.delta.x() / sp;
    qreal dy = ed.delta.y() / sp;
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

        qreal x = e.x;
        qreal y = e.y;
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
            qreal x2 = path.elementAt(i + 1).x;
            qreal y2 = path.elementAt(i + 1).y;
            qreal x3 = path.elementAt(i + 2).x;
            qreal y3 = path.elementAt(i + 2).y;
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
    qreal sp = spatium();
    auto n   = path.elementCount();
    PointF cp(pagePos());
    if (_straight) {
        // limit the number of grips to one
        qreal offset = 0.5 * sp;
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

QString ChordLine::accessibleInfo() const
{
    QString rez = EngravingItem::accessibleInfo();
    if (chordLineType() != ChordLineType::NOTYPE) {
        rez = QString("%1: %2").arg(rez, scorelineNames[static_cast<int>(chordLineType()) - 1]);
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

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid ChordLine::propertyId(const QStringRef& name) const
{
    if (name == "subtype") {
        return Pid::CHORD_LINE_TYPE;
    } else if (name == "straight") {
        return Pid::CHORD_LINE_STRAIGHT;
    }
    return EngravingItem::propertyId(name);
}
}
