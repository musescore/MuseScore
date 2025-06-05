/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "noteline.h"
#include "linkedobjects.h"
#include "factory.h"
#include "note.h"
#include "textline.h"

namespace mu::engraving {
static const ElementStyle noteLineStyle {
    { Sid::noteLinePlacement,                  Pid::PLACEMENT },
    { Sid::noteLineFontFace,                   Pid::BEGIN_FONT_FACE },
    { Sid::noteLineFontFace,                   Pid::CONTINUE_FONT_FACE },
    { Sid::noteLineFontFace,                   Pid::END_FONT_FACE },
    { Sid::noteLineFontSize,                   Pid::BEGIN_FONT_SIZE },
    { Sid::noteLineFontSize,                   Pid::CONTINUE_FONT_SIZE },
    { Sid::noteLineFontSize,                   Pid::END_FONT_SIZE },
    { Sid::noteLineFontStyle,                  Pid::BEGIN_FONT_STYLE },
    { Sid::noteLineFontStyle,                  Pid::CONTINUE_FONT_STYLE },
    { Sid::noteLineFontStyle,                  Pid::END_FONT_STYLE },
    { Sid::noteLineAlign,                      Pid::BEGIN_TEXT_ALIGN },
    { Sid::noteLineAlign,                      Pid::CONTINUE_TEXT_ALIGN },
    { Sid::noteLineAlign,                      Pid::END_TEXT_ALIGN },
    { Sid::noteLineFontSpatiumDependent,       Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
    { Sid::noteLineWidth,                      Pid::LINE_WIDTH },
    { Sid::noteLineStyle,                      Pid::LINE_STYLE },
    { Sid::noteLineDashLineLen,                Pid::DASH_LINE_LEN },
    { Sid::noteLineDashGapLen,                 Pid::DASH_GAP_LEN },
};

Sid NoteLineSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return Sid::NOSTYLE;
    }
    return TextLineBaseSegment::getPropertyStyle(pid);
}

NoteLineSegment::NoteLineSegment(Spanner* sp, System* parent)
    : TextLineBaseSegment(ElementType::NOTELINE_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
}

EngravingItem* NoteLineSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::NOTELINE_PLACEMENT) {
        return noteLine();
    }

    return TextLineBaseSegment::propertyDelegate(pid);
}

Sid NoteLine::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return Sid::NOSTYLE;
    }
    return TextLineBase::getPropertyStyle(pid);
}

NoteLine::NoteLine(EngravingItem* parent)
    : TextLineBase(ElementType::NOTELINE, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&noteLineStyle);

    static const std::array<Pid, 19> propertiesToInitialise {
        Pid::BEGIN_TEXT,
        Pid::CONTINUE_TEXT,
        Pid::END_TEXT,
        Pid::LINE_VISIBLE,
        Pid::BEGIN_TEXT_OFFSET,
        Pid::CONTINUE_TEXT_OFFSET,
        Pid::END_TEXT_OFFSET,
        Pid::BEGIN_HOOK_TYPE,
        Pid::END_HOOK_TYPE,
        Pid::BEGIN_TEXT_PLACE,
        Pid::CONTINUE_TEXT_PLACE,
        Pid::END_TEXT_PLACE,
        Pid::BEGIN_HOOK_HEIGHT,
        Pid::END_HOOK_HEIGHT,
        Pid::DIAGONAL,
        Pid::NOTELINE_PLACEMENT,
        Pid::GAP_BETWEEN_TEXT_AND_LINE,
        Pid::SYSTEM_FLAG,
        Pid::ANCHOR
    };

    for (const Pid& pid : propertiesToInitialise) {
        resetProperty(pid);
    }
}

NoteLine::NoteLine(const NoteLine& nl)
    : TextLineBase(nl)
{
}

LineSegment* NoteLine::createLineSegment(System* parent)
{
    NoteLineSegment* seg = new NoteLineSegment(this, parent);
    seg->setTrack(track());
    return seg;
}

PropertyValue NoteLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return "";
    case Pid::LINE_VISIBLE:
        return true;
    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PointF(0, 0);
    case Pid::BEGIN_HOOK_TYPE:
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;
    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;
    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Spatium(1.5);
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::DIAGONAL:
        return true;
    case Pid::NOTELINE_PLACEMENT:
        return NoteLineEndPlacement::OFFSET_ENDS;
    case Pid::OFFSET:
        return PointF();
    case Pid::GAP_BETWEEN_TEXT_AND_LINE:
        return Spatium(0.5);
    case Pid::SYSTEM_FLAG:
        return false;
    case Pid::ANCHOR:
        return int(Spanner::Anchor::NOTE);
    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

PropertyValue NoteLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PLAY:
        return PropertyValue();
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::NOTELINE_PLACEMENT:
        return m_lineEndPlacement;
    default:
        return TextLineBase::getProperty(propertyId);
    }
}

bool NoteLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PLAY:
        break;
    case Pid::NOTELINE_PLACEMENT:
        setLineEndPlacement(val.value<NoteLineEndPlacement>());
        break;
    default:
        return TextLineBase::setProperty(propertyId, val);
    }

    return true;
}

void NoteLine::reset()
{
    undoResetProperty(Pid::NOTELINE_PLACEMENT);
    TextLineBase::reset();
}
} // namespace mu::engraving
