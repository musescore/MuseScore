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

#include "iengravingfont.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "chord.h"
#include "note.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
const SymIdList ChordLine::WAVE_SYMBOLS = { SymId::wiggleVIbratoMediumSlower, SymId::wiggleVIbratoMediumSlower };

//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Chord* parent)
    : EngravingItem(ElementType::CHORDLINE, parent, ElementFlag::MOVABLE)
{
}

ChordLine::ChordLine(const ChordLine& cl)
    : EngravingItem(cl)
{
    m_path     = cl.m_path;
    m_modified = cl.m_modified;
    m_chordLineType = cl.m_chordLineType;
    m_straight = cl.m_straight;
    m_wavy = cl.m_wavy;
    m_lengthX = cl.m_lengthX;
    m_lengthY = cl.m_lengthY;
    m_note = cl.m_note;
}

KerningType ChordLine::doComputeKerningType(const EngravingItem* nextItem) const
{
    if (nextItem->isBarLine()) {
        return KerningType::ALLOW_COLLISION;
    }
    return KerningType::KERNING;
}

//---------------------------------------------------------
//   setChordLineType
//---------------------------------------------------------

void ChordLine::setChordLineType(ChordLineType st)
{
    m_chordLineType = st;
}

const TranslatableString& ChordLine::chordLineTypeName() const
{
    return TConv::userName(m_chordLineType, m_straight);
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void ChordLine::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (!m_wavy) {
        painter->setPen(Pen(curColor(), score()->styleMM(Sid::chordlineThickness) * mag(), PenStyle::SolidLine));
        painter->setBrush(BrushStyle::NoBrush);
        painter->drawPath(m_path);
    } else {
        painter->save();
        painter->rotate((m_chordLineType == ChordLineType::FALL ? 1 : -1) * WAVE_ANGEL);
        drawSymbols(ChordLine::WAVE_SYMBOLS, painter);
        painter->restore();
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
    auto n = m_path.elementCount();
    PainterPath p;
    m_lengthX += ed.delta.x();
    m_lengthY += ed.delta.y();

    // used to limit how grips can affect the slide, stops the user from being able to turn one kind of slide into another
    int slideBoundary = 5;
    if ((m_chordLineType == ChordLineType::PLOP || m_chordLineType == ChordLineType::FALL) && m_lengthY < -slideBoundary) {
        m_lengthY = -slideBoundary;
    } else if ((m_chordLineType == ChordLineType::FALL || m_chordLineType == ChordLineType::DOIT) && m_lengthX < -slideBoundary) {
        m_lengthX = -slideBoundary;
    } else if ((m_chordLineType == ChordLineType::DOIT || m_chordLineType == ChordLineType::SCOOP) && m_lengthY > slideBoundary) {
        m_lengthY = slideBoundary;
    } else if ((m_chordLineType == ChordLineType::SCOOP || m_chordLineType == ChordLineType::PLOP) && m_lengthX > slideBoundary) {
        m_lengthX = slideBoundary;
    }

    double dx = ed.delta.x();
    double dy = ed.delta.y();

    bool curvative = !m_wavy && !m_straight;
    for (size_t i = 0; i < n; ++i) {
        const PainterPath::Element& e = curvative ? m_path.elementAt(i) : m_path.elementAt(1);
        if (!curvative) {
            if (i > 0) {
                break;
            }
            // check the gradient of the line
            const PainterPath::Element& startPoint = m_path.elementAt(0);
            if ((m_chordLineType == ChordLineType::FALL && (e.x + dx < startPoint.x || e.y + dy < startPoint.y))
                || (m_chordLineType == ChordLineType::DOIT && (e.x + dx < startPoint.x || e.y + dy > startPoint.y))
                || (m_chordLineType == ChordLineType::SCOOP && (e.x + dx > startPoint.x || e.y + dy < startPoint.y))
                || (m_chordLineType == ChordLineType::PLOP && (e.x + dx > startPoint.x || e.y + dy > startPoint.y))) {
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
            double x2 = m_path.elementAt(i + 1).x;
            double y2 = m_path.elementAt(i + 1).y;
            double x3 = m_path.elementAt(i + 2).x;
            double y3 = m_path.elementAt(i + 2).y;
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
    m_path = p;
    m_modified = true;
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> ChordLine::gripsPositions(const EditData&) const
{
    if (m_wavy) {
        NOT_IMPLEMENTED;
        return {};
    }

    double sp = spatium();
    auto n   = m_path.elementCount();
    PointF cp(pagePos());
    if (m_straight) {
        // limit the number of grips to one
        double offset = 0.5 * sp;
        PointF p;

        if (m_chordLineType == ChordLineType::FALL) {
            p = PointF(offset, -offset);
        } else if (m_chordLineType == ChordLineType::DOIT) {
            p = PointF(offset, offset);
        } else if (m_chordLineType == ChordLineType::SCOOP) {
            p = PointF(-offset, offset);
        } else if (m_chordLineType == ChordLineType::PLOP) {
            p = PointF(-offset, -offset);
        }

        // translate on the length and height - stops the grips from going past boundaries of slide
        p += (cp + PointF(m_path.elementAt(1).x * sp, m_path.elementAt(1).y * sp));
        return { p };
    } else {
        std::vector<PointF> grips(n);
        for (size_t i = 0; i < n; ++i) {
            grips[i] = cp + PointF(m_path.elementAt(i).x, m_path.elementAt(i).y);
        }
        return grips;
    }
}

//---------------------------------------------------------
//   slideType
//---------------------------------------------------------

static Note::SlideType slideType(ChordLineType type)
{
    static std::unordered_map<ChordLineType, Note::SlideType> chordLineToSlideTypes {
        { ChordLineType::FALL, Note::SlideType::DownFromNote },
        { ChordLineType::DOIT, Note::SlideType::UpFromNote },
        { ChordLineType::SCOOP, Note::SlideType::DownToNote },
        { ChordLineType::PLOP, Note::SlideType::UpToNote }
    };

    if (chordLineToSlideTypes.find(type) != chordLineToSlideTypes.end()) {
        return chordLineToSlideTypes.at(type);
    }

    return Note::SlideType::Undefined;
}

//---------------------------------------------------------
//   setNote
//---------------------------------------------------------

void ChordLine::setNote(Note* note)
{
    m_note = note;
    note->attachSlide(slideType(m_chordLineType));
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String ChordLine::accessibleInfo() const
{
    String rez = EngravingItem::accessibleInfo();
    if (chordLineType() != ChordLineType::NOTYPE) {
        rez = String(u"%1: %2").arg(rez, chordLineTypeName().translated());
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
        return PropertyValue::fromValue(m_path);
    case Pid::CHORD_LINE_TYPE:
        return int(m_chordLineType);
    case Pid::CHORD_LINE_STRAIGHT:
        return m_straight;
    case Pid::CHORD_LINE_WAVY:
        return m_wavy;
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
        m_path = val.value<PainterPath>();
        break;
    case Pid::CHORD_LINE_TYPE:
        setChordLineType(ChordLineType(val.toInt()));
        break;
    case Pid::CHORD_LINE_STRAIGHT:
        setStraight(val.toBool());
        break;
    case Pid::CHORD_LINE_WAVY:
        setWavy(val.toBool());
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
    case Pid::CHORD_LINE_WAVY:
        return false;
    default:
        break;
    }
    return EngravingItem::propertyDefault(pid);
}
}
