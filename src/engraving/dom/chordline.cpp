/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <functional>

#include "types/translatablestring.h"

#include "../editing/elementeditdata.h"
#include "../types/typesconv.h"

#include "chord.h"
#include "note.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(DummyParentOr<Chord> parent)
    : EngravingItem(ElementType::CHORDLINE, parent, ElementFlag::MOVABLE)
{
}

ChordLine::ChordLine(const ChordLine& cl)
    : EngravingItem(cl)
{
    m_chordLineType = cl.m_chordLineType;
    m_straight = cl.m_straight;
    m_wavy = cl.m_wavy;
    m_bezierOffsets = cl.m_bezierOffsets;
    m_playChordLine = cl.m_playChordLine;
    m_note = cl.m_note;
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
    return TConv::userName(m_chordLineType, m_straight, m_wavy);
}

ChordLine::Points ChordLine::defaultPoints() const
{
    const Spatium x = isToTheLeft() ? -1.2_sp : 1.2_sp; // let the symbols extend a bit more horizontally
    const Spatium y = isBelow() ? 1.0_sp : -1.0_sp;

    if (isToTheLeft()) {
        return { PointF(), PointF(0.0, (y / 2).val()), PointF((x / 2).val(), y.val()), PointF(x.val(), y.val()) };
    }

    return { PointF(), PointF((x / 2).val(), 0.0), PointF(x.val(), (y / 2).val()), PointF(x.val(), y.val()) };
}

double ChordLine::chordMag() const
{
    const EngravingObject* parent = ownershipParent();
    return parent && parent->isChord() ? toChord(parent)->intrinsicMag() : 1.0;
}

Pid ChordLine::bezierOffsetPid(Point point)
{
    switch (point) {
    case Point::START: return Pid::BEZIER_OFFSET1;
    case Point::BEZIER1: return Pid::BEZIER_OFFSET2;
    case Point::BEZIER2: return Pid::BEZIER_OFFSET3;
    case Point::END: return Pid::BEZIER_OFFSET4;
    case Point::POINTS_COUNT: break;
    }

    UNREACHABLE;
    return Pid::END;
}

ChordLine::Point ChordLine::gripPoint(Grip grip) const
{
    // straight lines only have one grip, for the end point
    return m_straight ? Point::END : Point(static_cast<int>(grip));
}

int ChordLine::gripsCount() const
{
    if (m_wavy || m_chordLineType == ChordLineType::NOTYPE) {
        return 0;
    }

    return m_straight ? 1 : static_cast<int>(Point::POINTS_COUNT);
}

//---------------------------------------------------------
//   startDragGrip
//---------------------------------------------------------

void ChordLine::startDragGrip(EditData& ed)
{
    EngravingItem::startDragGrip(ed);
    ElementEditDataPtr eed = ed.getData(this);

    for (size_t i = 0; i < size_t(Point::POINTS_COUNT); ++i) {
        eed->pushProperty(bezierOffsetPid(Point(i)));
    }
}

//---------------------------------------------------------
//   dragGrip
//---------------------------------------------------------

void ChordLine::dragGrip(EditData& ed)
{
    if (m_wavy || ed.curGrip == Grip::NO_GRIP || int(ed.curGrip) >= gripsCount()) {
        return;
    }

    const Point point = gripPoint(ed.curGrip);
    // offsets are at the chord's normal size, the drawn shape is scaled by its mag
    const PointF offset = bezierOffset(point) + ed.delta / chordMag();

    if (m_straight) {
        // stop the user from being able to turn one kind of slide into another
        // (the shape at the chord's normal size is enough to check the direction)
        const Points points = defaultPoints();
        const double sp = spatium();
        const PointF start = points[size_t(Point::START)] * sp + bezierOffset(Point::START);
        const PointF end = points[size_t(Point::END)] * sp + offset;
        if ((m_chordLineType == ChordLineType::FALL && (end.x() < start.x() || end.y() < start.y()))
            || (m_chordLineType == ChordLineType::DOIT && (end.x() < start.x() || end.y() > start.y()))
            || (m_chordLineType == ChordLineType::SCOOP && (end.x() > start.x() || end.y() < start.y()))
            || (m_chordLineType == ChordLineType::PLOP && (end.x() > start.x() || end.y() > start.y()))) {
            return;
        }
    }

    setBezierOffset(point, offset);
    triggerLayout();
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

    const PainterPath& path = ldata()->path;

    size_t n = path.elementCount();
    if (n != (m_straight ? 2 : size_t(Point::POINTS_COUNT))) {
        return {};
    }

    PointF cp(pagePos());
    if (m_straight) {
        // limit the number of grips to one
        double offset = 0.5;
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
        p += (cp + PointF(path.elementAt(1).x, path.elementAt(1).y));
        return { p };
    } else {
        std::vector<PointF> grips(n);
        for (size_t i = 0; i < n; ++i) {
            grips[i] = cp + PointF(path.elementAt(i).x, path.elementAt(i).y);
        }
        return grips;
    }
}

//---------------------------------------------------------
//   slideType
//---------------------------------------------------------

static Note::SlideType slideType(ChordLineType type)
{
    static const std::unordered_map<ChordLineType, Note::SlideType> chordLineToSlideTypes {
        { ChordLineType::FALL, Note::SlideType::DownFromNote },
        { ChordLineType::DOIT, Note::SlideType::UpFromNote },
        { ChordLineType::SCOOP, Note::SlideType::UpToNote },
        { ChordLineType::PLOP, Note::SlideType::DownToNote }
    };

    auto it = chordLineToSlideTypes.find(type);
    if (it != chordLineToSlideTypes.end()) {
        return it->second;
    }

    return Note::SlideType::Undefined;
}

//---------------------------------------------------------
//   setNote
//---------------------------------------------------------

void ChordLine::setNote(Note* note)
{
    m_note = note;

    if (note) {
        note->attachSlide(slideType(m_chordLineType));
    }
}

SymId ChordLine::waveSym() const
{
    if (m_chordLineType == ChordLineType::FALL || m_chordLineType == ChordLineType::PLOP) {
        return SymId::brassFallRoughShort;
    }

    return SymId::brassLiftShort;
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

int ChordLine::subtype() const
{
    size_t h1 = std::hash<ChordLineType> {}(m_chordLineType);
    size_t h2 = std::hash<bool> {}(m_straight);
    size_t h3 = std::hash<bool> {}(m_wavy);

    return static_cast<int>(h1 ^ (h2 << 1) ^ (h3 << 2));
}

muse::TranslatableString ChordLine::subtypeUserName() const
{
    return chordLineTypeName();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue ChordLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BEZIER_OFFSET1:
        return bezierOffset(Point::START);
    case Pid::BEZIER_OFFSET2:
        return bezierOffset(Point::BEZIER1);
    case Pid::BEZIER_OFFSET3:
        return bezierOffset(Point::BEZIER2);
    case Pid::BEZIER_OFFSET4:
        return bezierOffset(Point::END);
    case Pid::CHORD_LINE_TYPE:
        return int(m_chordLineType);
    case Pid::CHORD_LINE_STRAIGHT:
        return m_straight;
    case Pid::CHORD_LINE_WAVY:
        return m_wavy;
    case Pid::PLAY:
        return m_playChordLine;
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
    case Pid::BEZIER_OFFSET1:
        setBezierOffset(Point::START, val.value<PointF>());
        break;
    case Pid::BEZIER_OFFSET2:
        setBezierOffset(Point::BEZIER1, val.value<PointF>());
        break;
    case Pid::BEZIER_OFFSET3:
        setBezierOffset(Point::BEZIER2, val.value<PointF>());
        break;
    case Pid::BEZIER_OFFSET4:
        setBezierOffset(Point::END, val.value<PointF>());
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
    case Pid::PLAY:
        setPlayChordLine(val.toBool());
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
    case Pid::PLAY:
        return true;
    case Pid::BEZIER_OFFSET1:
    case Pid::BEZIER_OFFSET2:
    case Pid::BEZIER_OFFSET3:
    case Pid::BEZIER_OFFSET4:
        return PointF();
    default:
        break;
    }
    return EngravingItem::propertyDefault(pid);
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void ChordLine::reset()
{
    for (size_t i = 0; i < size_t(Point::POINTS_COUNT); ++i) {
        undoResetProperty(bezierOffsetPid(Point(i)));
    }
    EngravingItem::reset();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void ChordLine::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    for (PointF& offset : m_bezierOffsets) {
        offset *= newValue / oldValue;
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void ChordLine::localSpatiumChanged(double oldValue, double newValue)
{
    EngravingItem::localSpatiumChanged(oldValue, newValue);
    for (PointF& offset : m_bezierOffsets) {
        offset *= newValue / oldValue;
    }
}
}
