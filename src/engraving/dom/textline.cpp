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
#include "textline.h"

#include "score.h"
#include "system.h"
#include "undo.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   textLineSegmentStyle
//---------------------------------------------------------

static const ElementStyle textLineSegmentStyle {
    { Sid::textLinePosAbove,      Pid::OFFSET },
    { Sid::textLineMinDistance,   Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   systemTextLineSegmentStyle
//---------------------------------------------------------

static const ElementStyle systemTextLineSegmentStyle {
    { Sid::systemTextLinePosAbove,      Pid::OFFSET },
    { Sid::systemTextLineMinDistance,   Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   textLineStyle
//---------------------------------------------------------

static const ElementStyle textLineStyle {
//       { Sid::textLineSystemFlag,                 Pid::SYSTEM_FLAG             },
    { Sid::textLineFontFace,                   Pid::BEGIN_FONT_FACE },
    { Sid::textLineFontFace,                   Pid::CONTINUE_FONT_FACE },
    { Sid::textLineFontFace,                   Pid::END_FONT_FACE },
    { Sid::textLineFontSize,                   Pid::BEGIN_FONT_SIZE },
    { Sid::textLineFontSize,                   Pid::CONTINUE_FONT_SIZE },
    { Sid::textLineFontSize,                   Pid::END_FONT_SIZE },
    { Sid::textLineFontStyle,                  Pid::BEGIN_FONT_STYLE },
    { Sid::textLineFontStyle,                  Pid::CONTINUE_FONT_STYLE },
    { Sid::textLineFontStyle,                  Pid::END_FONT_STYLE },
    { Sid::textLineTextAlign,                  Pid::BEGIN_TEXT_ALIGN },
    { Sid::textLineTextAlign,                  Pid::CONTINUE_TEXT_ALIGN },
    { Sid::textLineTextAlign,                  Pid::END_TEXT_ALIGN },
    { Sid::textLineHookHeight,                 Pid::BEGIN_HOOK_HEIGHT },
    { Sid::textLineHookHeight,                 Pid::END_HOOK_HEIGHT },
    { Sid::textLineLineWidth,                  Pid::LINE_WIDTH },
    { Sid::textLineDashLineLen,                Pid::DASH_LINE_LEN },
    { Sid::textLineDashGapLen,                 Pid::DASH_GAP_LEN },
    { Sid::textLinePlacement,                  Pid::PLACEMENT },
    { Sid::textLineLineStyle,                  Pid::LINE_STYLE },
    { Sid::textLinePosAbove,                   Pid::OFFSET },
    { Sid::textLineFontSpatiumDependent,       Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
};

//---------------------------------------------------------
//   systemTextLineStyle
//---------------------------------------------------------

static const ElementStyle systemTextLineStyle {
//       { Sid::systemTextLineSystemFlag,           Pid::SYSTEM_FLAG             },
    { Sid::systemTextLineFontFace,             Pid::BEGIN_FONT_FACE },
    { Sid::systemTextLineFontFace,             Pid::CONTINUE_FONT_FACE },
    { Sid::systemTextLineFontFace,             Pid::END_FONT_FACE },
    { Sid::systemTextLineFontSize,             Pid::BEGIN_FONT_SIZE },
    { Sid::systemTextLineFontSize,             Pid::CONTINUE_FONT_SIZE },
    { Sid::systemTextLineFontSize,             Pid::END_FONT_SIZE },
    { Sid::systemTextLineFontStyle,            Pid::BEGIN_FONT_STYLE },
    { Sid::systemTextLineFontStyle,            Pid::CONTINUE_FONT_STYLE },
    { Sid::systemTextLineFontStyle,            Pid::END_FONT_STYLE },
    { Sid::systemTextLineTextAlign,            Pid::BEGIN_TEXT_ALIGN },
    { Sid::systemTextLineTextAlign,            Pid::CONTINUE_TEXT_ALIGN },
    { Sid::systemTextLineTextAlign,            Pid::END_TEXT_ALIGN },
    { Sid::systemTextLineHookHeight,           Pid::BEGIN_HOOK_HEIGHT },
    { Sid::systemTextLineHookHeight,           Pid::END_HOOK_HEIGHT },
    { Sid::systemTextLineLineWidth,            Pid::LINE_WIDTH },
    { Sid::systemTextLineDashLineLen,          Pid::DASH_LINE_LEN },
    { Sid::systemTextLineDashGapLen,           Pid::DASH_GAP_LEN },
    { Sid::systemTextLinePlacement,            Pid::PLACEMENT },
    { Sid::systemTextLineLineStyle,            Pid::LINE_STYLE },
    { Sid::systemTextLinePosAbove,             Pid::OFFSET },
};

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Spanner* sp, System* parent, bool system)
    : TextLineBaseSegment(ElementType::TEXTLINE_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    setSystemFlag(system);
    initStyle();
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* TextLineSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::SYSTEM_FLAG) {
        return static_cast<TextLine*>(spanner());
    }
    return TextLineBaseSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(EngravingItem* parent, bool system)
    : TextLineBase(ElementType::TEXTLINE, parent)
{
    setSystemFlag(system);

    initStyle();

    setBeginText(u"");
    setContinueText(u"");
    setEndText(u"");
    setBeginTextOffset(PointF(0, 0));
    setContinueTextOffset(PointF(0, 0));
    setEndTextOffset(PointF(0, 0));
    setLineVisible(true);

    setBeginHookType(HookType::NONE);
    setEndHookType(HookType::NONE);
    setBeginHookHeight(Spatium(1.5));
    setEndHookHeight(Spatium(1.5));
    setGapBetweenTextAndLine(Spatium(0.5));

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::END_TEXT_PLACE);
}

TextLine::TextLine(const TextLine& tl)
    : TextLineBase(tl)
{
}

//---------------------------------------------------------
//   initStyle
//---------------------------------------------------------

void TextLine::initStyle()
{
    if (systemFlag()) {
        initElementStyle(&systemTextLineStyle);
    } else {
        initElementStyle(&textLineStyle);
    }
}

void TextLineSegment::initStyle()
{
    if (systemFlag()) {
        initElementStyle(&systemTextLineSegmentStyle);
    } else {
        initElementStyle(&textLineSegmentStyle);
    }
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment(System* parent)
{
    TextLineSegment* seg = new TextLineSegment(this, parent, systemFlag());
    seg->setTrack(track());
    // note-anchored line segments are relative to system not to staff
    if (anchor() == Spanner::Anchor::NOTE) {
        seg->setFlag(ElementFlag::ON_STAFF, false);
    }
    seg->initStyle();

    return seg;
}

//---------------------------------------------------------
//   getTextLinePos
//---------------------------------------------------------

Sid TextLineSegment::getTextLinePos(bool above) const
{
    return textLine()->getTextLinePos(above);
}

Sid TextLine::getTextLinePos(bool above) const
{
    if (systemFlag()) {
        return above ? Sid::systemTextLinePosAbove : Sid::systemTextLinePosBelow;
    } else {
        return above ? Sid::textLinePosAbove : Sid::textLinePosBelow;
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TextLineSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        if (spanner()->anchor() == Spanner::Anchor::NOTE) {
            return Sid::NOSTYLE;
        } else {
            return getTextLinePos(spanner()->placeAbove());
        }
    }
    return TextLineBaseSegment::getPropertyStyle(pid);
}

Sid TextLine::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        if (anchor() == Spanner::Anchor::NOTE) {
            return Sid::NOSTYLE;
        } else {
            return getTextLinePos(placeAbove());
        }
    }
    return TextLineBase::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TextLine::propertyDefault(Pid propertyId) const
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
    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   allowTimeAnchor
//---------------------------------------------------------
bool TextLine::allowTimeAnchor() const
{
    return !(anchor() == Spanner::Anchor::NOTE);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLine::setProperty(Pid id, const engraving::PropertyValue& v)
{
    switch (id) {
    case Pid::PLAY:
        break;
    case Pid::PLACEMENT:
        setPlacement(v.value<PlacementV>());
        break;
    default:
        return TextLineBase::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TextLine::getProperty(Pid id) const
{
    switch (id) {
    case Pid::PLAY:
        return PropertyValue();
    default:
        break;
    }
    return TextLineBase::getProperty(id);
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void TextLine::undoChangeProperty(Pid id, const engraving::PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::SYSTEM_FLAG) {
        score()->undo(new ChangeTextLineProperty(this, v));
        for (SpannerSegment* s : spannerSegments()) {
            score()->undo(new ChangeTextLineProperty(s, v));
            triggerLayout();
        }
        return;
    }
    TextLineBase::undoChangeProperty(id, v, ps);
}
} // namespace mu::engraving
