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

#include "whammybar.h"

#include "score.h"
#include "system.h"
#include "text.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle whammyBarStyle {
    { Sid::whammyBarFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::whammyBarFontFace,                      Pid::CONTINUE_FONT_FACE },
    { Sid::whammyBarFontFace,                      Pid::END_FONT_FACE },
    { Sid::whammyBarFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::whammyBarFontSize,                      Pid::CONTINUE_FONT_SIZE },
    { Sid::whammyBarFontSize,                      Pid::END_FONT_SIZE },
    { Sid::whammyBarFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::whammyBarFontStyle,                     Pid::CONTINUE_FONT_STYLE },
    { Sid::whammyBarFontStyle,                     Pid::END_FONT_STYLE },
    { Sid::whammyBarTextAlign,                     Pid::BEGIN_TEXT_ALIGN },
    { Sid::whammyBarTextAlign,                     Pid::CONTINUE_TEXT_ALIGN },
    { Sid::whammyBarTextAlign,                     Pid::END_TEXT_ALIGN },
    { Sid::whammyBarPosition,                      Pid::BEGIN_TEXT_POSITION },
    { Sid::whammyBarPosition,                      Pid::CONTINUE_TEXT_POSITION },
    { Sid::whammyBarPosition,                      Pid::END_TEXT_POSITION },
    { Sid::whammyBarHookHeight,                    Pid::BEGIN_HOOK_HEIGHT },
    { Sid::whammyBarHookHeight,                    Pid::END_HOOK_HEIGHT },
    { Sid::whammyBarLineStyle,                     Pid::LINE_STYLE },
    { Sid::whammyBarDashLineLen,                   Pid::DASH_LINE_LEN },
    { Sid::whammyBarDashGapLen,                    Pid::DASH_GAP_LEN },
    { Sid::whammyBarFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
    { Sid::whammyBarEndHookType,                   Pid::END_HOOK_TYPE },
    { Sid::whammyBarLineWidth,                     Pid::LINE_WIDTH },
    { Sid::whammyBarText,                          Pid::BEGIN_TEXT },
};

WhammyBarSegment::WhammyBarSegment(WhammyBar* sp, System* parent)
    : TextLineBaseSegment(ElementType::WHAMMY_BAR_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_text->setTextStyleType(propertyDefault(Pid::TEXT_STYLE).value<TextStyleType>());
    m_endText->setTextStyleType(propertyDefault(Pid::TEXT_STYLE).value<TextStyleType>());
}

//---------------------------------------------------------
//   WhammyBar
//---------------------------------------------------------

WhammyBar::WhammyBar(EngravingItem* parent)
    : ChordTextLineBase(ElementType::WHAMMY_BAR, parent)
{
    initElementStyle(&whammyBarStyle);
    resetProperty(Pid::LINE_VISIBLE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::END_TEXT);
}

static const ElementStyle whammyBarSegmentStyle {
    { Sid::letRingMinDistance,    Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* WhammyBar::createLineSegment(System* parent)
{
    WhammyBarSegment* wb = new WhammyBarSegment(this, parent);
    wb->setTrack(track());
    wb->initElementStyle(&whammyBarSegmentStyle);
    return wb;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue WhammyBar::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return style().styleV(Sid::whammyBarLineWidth);

    case Pid::LINE_STYLE:
        return style().styleV(Sid::whammyBarLineStyle);

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return style().styleV(Sid::whammyBarFontStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
        return style().styleV(Sid::whammyBarText).value<String>(); // TODO: fix the style

    case Pid::END_TEXT:
        return "";

    case Pid::BEGIN_HOOK_TYPE:
        return HookType::NONE;

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::AUTO;

    case Pid::TEXT_STYLE:
        return TextStyleType::WHAMMY_BAR;

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid WhammyBar::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::ottava8VAPlacement; // TODO: fix the style
    case Pid::BEGIN_FONT_FACE:
        return Sid::whammyBarFontFace;
    case Pid::BEGIN_FONT_SIZE:
    case Pid::CONTINUE_FONT_SIZE:
    case Pid::END_FONT_SIZE:
        return Sid::whammyBarFontSize;
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return Sid::whammyBarFontStyle;
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return Sid::whammyBarTextAlign;
    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Sid::whammyBarHookHeight;
    default:
        break;
    }
    return TextLineBase::getPropertyStyle(id);
}
}
