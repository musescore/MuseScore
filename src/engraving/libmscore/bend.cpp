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

#include "bend.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "xml.h"
#include "draw/pen.h"
#include "draw/brush.h"

#include "draw/fontmetrics.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   label
//---------------------------------------------------------

static const char* label[] = {
    "", "1/4", "1/2", "3/4", "full",
    "1 1/4", "1 1/2", "1 3/4", "2",
    "2 1/4", "2 1/2", "2 3/4", "3"
};

static const ElementStyle bendStyle {
    { Sid::bendFontFace,                       Pid::FONT_FACE },
    { Sid::bendFontSize,                       Pid::FONT_SIZE },
    { Sid::bendFontStyle,                      Pid::FONT_STYLE },
    { Sid::bendLineWidth,                      Pid::LINE_WIDTH },
};

static const QList<PitchValue> BEND_CURVE = { PitchValue(0, 0),
                                              PitchValue(15, 100),
                                              PitchValue(60, 100) };

static const QList<PitchValue> BEND_RELEASE_CURVE = { PitchValue(0, 0),
                                                      PitchValue(10, 100),
                                                      PitchValue(20, 100),
                                                      PitchValue(30, 0),
                                                      PitchValue(60, 0) };

static const QList<PitchValue> BEND_RELEASE_BEND_CURVE = { PitchValue(0, 0),
                                                           PitchValue(10, 100),
                                                           PitchValue(20, 100),
                                                           PitchValue(30, 0),
                                                           PitchValue(40, 0),
                                                           PitchValue(50, 100),
                                                           PitchValue(60, 100) };

static const QList<PitchValue> PREBEND_CURVE = { PitchValue(0, 100),
                                                 PitchValue(60, 100) };

static const QList<PitchValue> PREBEND_RELEASE_CURVE = { PitchValue(0, 100),
                                                         PitchValue(15, 100),
                                                         PitchValue(30, 0),
                                                         PitchValue(60, 0) };

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
    : Element(s, ElementFlag::MOVABLE)
{
    initElementStyle(&bendStyle);
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

mu::draw::Font Bend::font(qreal sp) const
{
    mu::draw::Font f(_fontFace);
    f.setBold(_fontStyle & FontStyle::Bold);
    f.setItalic(_fontStyle & FontStyle::Italic);
    f.setUnderline(_fontStyle & FontStyle::Underline);
    qreal m = _fontSize;
    m *= sp / SPATIUM20;

    f.setPointSizeF(m);
    return f;
}

BendType Bend::parseBendTypeFromCurve() const
{
    if (m_points == BEND_CURVE) {
        return BendType::BEND;
    } else if (m_points == BEND_RELEASE_CURVE) {
        return BendType::BEND_RELEASE;
    } else if (m_points == BEND_RELEASE_BEND_CURVE) {
        return BendType::BEND_RELEASE_BEND;
    } else if (m_points == PREBEND_CURVE) {
        return BendType::PREBEND;
    } else if (m_points == PREBEND_RELEASE_CURVE) {
        return BendType::PREBEND_RELEASE;
    } else {
        return BendType::CUSTOM;
    }
}

void Bend::updatePointsByBendType(const BendType bendType)
{
    switch (bendType) {
    case BendType::BEND:
        m_points = BEND_CURVE;
        break;
    case BendType::BEND_RELEASE:
        m_points = BEND_RELEASE_CURVE;
        break;
    case BendType::BEND_RELEASE_BEND:
        m_points = BEND_RELEASE_BEND_CURVE;
        break;
    case BendType::PREBEND:
        m_points = PREBEND_CURVE;
        break;
    case BendType::PREBEND_RELEASE:
        m_points = PREBEND_RELEASE_CURVE;
        break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bend::layout()
{
    // during mtest, there may be no score. If so, exit.
    if (!score()) {
        return;
    }

    qreal _spatium = spatium();

    if (staff() && !staff()->isTabStaff(tick())) {
        if (!parent()) {
            m_noteWidth = -_spatium * 2;
            m_notePos   = PointF(0.0, _spatium * 3);
        }
    }

    qreal _lw = _lineWidth;
    Note* note = toNote(parent());
    if (note == 0) {
        m_noteWidth = 0.0;
        m_notePos = PointF();
    } else {
        m_notePos   = note->pos();
        m_notePos.ry() = qMax(m_notePos.y(), 0.0);
        m_noteWidth = note->width();
    }
    RectF bb;

    mu::draw::FontMetrics fm(font(_spatium));

    int n   = m_points.size();
    qreal x = m_noteWidth;
    qreal y = -_spatium * .8;
    qreal x2, y2;

    qreal aw = _spatium * .5;
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    for (int pt = 0; pt < n; ++pt) {
        if (pt == (n - 1)) {
            break;
        }
        int pitch = m_points[pt].pitch;
        if (pt == 0 && pitch) {
            y2 = -m_notePos.y() - _spatium * 2;
            x2 = x;
            bb.unite(RectF(x, y, x2 - x, y2 - y));

            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

            int idx = (pitch + 12) / 25;
            const char* l = label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l)));
            y = y2;
        }
        if (pitch == m_points[pt + 1].pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + _spatium;
            y2 = y;
            bb.unite(RectF(x, y, x2 - x, y2 - y));
        } else if (pitch < m_points[pt + 1].pitch) {
            // up
            x2 = x + _spatium * .5;
            y2 = -m_notePos.y() - _spatium * 2;
            qreal dx = x2 - x;
            qreal dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());
            bb.unite(arrowUp.translated(x2, y2 + _spatium * .2).boundingRect());

            int idx = (m_points[pt + 1].pitch + 12) / 25;
            const char* l = label[idx];
            bb.unite(fm.boundingRect(RectF(x2, y2, 0, 0),
                                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l)));
        } else {
            // down
            x2 = x + _spatium * .5;
            y2 = y + _spatium * 3;
            qreal dx = x2 - x;
            qreal dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            bb.unite(path.boundingRect());

            bb.unite(arrowDown.translated(x2, y2 - _spatium * .2).boundingRect());
        }
        x = x2;
        y = y2;
    }
    bb.adjust(-_lw, -_lw, _lw, _lw);
    setbbox(bb);
    setPos(0.0, 0.0);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bend::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    qreal _spatium = spatium();
    qreal _lw = _lineWidth;

    Pen pen(curColor(), _lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(curColor()));

    mu::draw::Font f = font(_spatium * MScore::pixelRatio);
    painter->setFont(f);

    qreal x  = m_noteWidth + _spatium * .2;
    qreal y  = -_spatium * .8;
    qreal x2, y2;

    qreal aw = score()->styleP(Sid::bendArrowWidth);
    PolygonF arrowUp;
    arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    PolygonF arrowDown;
    arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);

    int n = m_points.size();
    for (int pt = 0; pt < n - 1; ++pt) {
        int pitch = m_points[pt].pitch;
        if (pt == 0 && pitch) {
            y2 = -m_notePos.y() - _spatium * 2;
            x2 = x;
            painter->drawLine(LineF(x, y, x2, y2));

            painter->setBrush(curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (pitch + 12) / 25;
            const char* l = label[idx];
            QString s(l);
            painter->drawText(RectF(x2, y2, .0, .0), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, s);

            y = y2;
        }
        if (pitch == m_points[pt + 1].pitch) {
            if (pt == (n - 2)) {
                break;
            }
            x2 = x + _spatium;
            y2 = y;
            painter->drawLine(LineF(x, y, x2, y2));
        } else if (pitch < m_points[pt + 1].pitch) {
            // up
            x2 = x + _spatium * .5;
            y2 = -m_notePos.y() - _spatium * 2;
            qreal dx = x2 - x;
            qreal dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(curColor());
            painter->drawPolygon(arrowUp.translated(x2, y2));

            int idx = (m_points[pt + 1].pitch + 12) / 25;
            const char* l = label[idx];
            qreal ty = y2;       // - _spatium;
            painter->drawText(RectF(x2, ty, .0, .0),
                              Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
        } else {
            // down
            x2 = x + _spatium * .5;
            y2 = y + _spatium * 3;
            qreal dx = x2 - x;
            qreal dy = y2 - y;

            PainterPath path;
            path.moveTo(x, y);
            path.cubicTo(x + dx / 2, y, x2, y + dy / 4, x2, y2);
            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);

            painter->setBrush(curColor());
            painter->drawPolygon(arrowDown.translated(x2, y2));
        }
        x = x2;
        y = y2;
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Bend::write(XmlWriter& xml) const
{
    xml.stag(this);
    for (const PitchValue& v : m_points) {
        xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
                 .arg(v.time).arg(v.pitch).arg(v.vibrato));
    }
    writeStyledProperties(xml);
    writeProperty(xml, Pid::PLAY);
    Element::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Bend::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (readStyledProperty(e, tag)) {
        } else if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            m_points.append(pv);
            e.readNext();
        } else if (tag == "play") {
            setPlayBend(e.readBool());
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Bend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::FONT_FACE:
        return _fontFace;
    case Pid::FONT_SIZE:
        return _fontSize;
    case Pid::FONT_STYLE:
        return int(_fontStyle);
    case Pid::PLAY:
        return bool(playBend());
    case Pid::LINE_WIDTH:
        return _lineWidth;
    case Pid::BEND_TYPE:
        return static_cast<int>(parseBendTypeFromCurve());
    case Pid::BEND_CURVE:
        return QVariant::fromValue(m_points);
    default:
        return Element::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bend::setProperty(Pid id, const QVariant& v)
{
    switch (id) {
    case Pid::FONT_FACE:
        _fontFace = v.toString();
        break;
    case Pid::FONT_SIZE:
        _fontSize = v.toReal();
        break;
    case Pid::FONT_STYLE:
        _fontStyle = FontStyle(v.toInt());
        break;
    case Pid::PLAY:
        setPlayBend(v.toBool());
        break;
    case Pid::LINE_WIDTH:
        _lineWidth = v.toReal();
        break;
    case Pid::BEND_TYPE:
        updatePointsByBendType(static_cast<BendType>(v.toInt()));
        break;
    case Pid::BEND_CURVE:
        setPoints(v.value<QList<Ms::PitchValue> >());
        break;
    default:
        return Element::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Bend::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLAY:
        return true;
    case Pid::BEND_TYPE:
        return static_cast<int>(BendType::BEND);
    case Pid::BEND_CURVE:
        return QVariant::fromValue(BEND_CURVE);
    default:
        return Element::propertyDefault(id);
    }
}
}
