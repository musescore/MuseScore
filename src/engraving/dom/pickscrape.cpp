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

#include "pickscrape.h"

#include "score.h"
#include "system.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle pickScrapeStyle {
    { Sid::palmMuteFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::palmMuteFontFace,                      Pid::CONTINUE_FONT_FACE },
    { Sid::palmMuteFontFace,                      Pid::END_FONT_FACE },
    { Sid::palmMuteFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::palmMuteFontSize,                      Pid::CONTINUE_FONT_SIZE },
    { Sid::palmMuteFontSize,                      Pid::END_FONT_SIZE },
    { Sid::palmMuteFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::palmMuteFontStyle,                     Pid::CONTINUE_FONT_STYLE },
    { Sid::palmMuteFontStyle,                     Pid::END_FONT_STYLE },
    { Sid::palmMuteTextAlign,                     Pid::BEGIN_TEXT_ALIGN },
    { Sid::palmMuteTextAlign,                     Pid::CONTINUE_TEXT_ALIGN },
    { Sid::palmMuteTextAlign,                     Pid::END_TEXT_ALIGN },
    { Sid::palmMuteHookHeight,                    Pid::BEGIN_HOOK_HEIGHT },
    { Sid::palmMuteHookHeight,                    Pid::END_HOOK_HEIGHT },
    { Sid::palmMuteLineStyle,                     Pid::LINE_STYLE },
    { Sid::palmMuteDashLineLen,                   Pid::DASH_LINE_LEN },
    { Sid::palmMuteDashGapLen,                    Pid::DASH_GAP_LEN },
    { Sid::palmMuteFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
    { Sid::palmMuteEndHookType,                   Pid::END_HOOK_TYPE },
    { Sid::palmMuteLineWidth,                     Pid::LINE_WIDTH },
    { Sid::palmMutePlacement,                     Pid::PLACEMENT }
};

PickScrapeSegment::PickScrapeSegment(PickScrape* sp, System* parent)
    : TextLineBaseSegment(ElementType::PICK_SCRAPE_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   PickScrape
//---------------------------------------------------------

PickScrape::PickScrape(EngravingItem* parent)
    : ChordTextLineBase(ElementType::PICK_SCRAPE, parent)
{
    initElementStyle(&pickScrapeStyle);
    resetProperty(Pid::LINE_VISIBLE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::END_TEXT);
}

static const ElementStyle pickScrapeSegmentStyle {
    { Sid::palmMuteMinDistance,    Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* PickScrape::createLineSegment(System* parent)
{
    PickScrapeSegment* wb = new PickScrapeSegment(this, parent);
    wb->setTrack(track());
    wb->initElementStyle(&pickScrapeSegmentStyle);
    return wb;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue PickScrape::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return style().styleV(Sid::palmMuteLineWidth);

    case Pid::ALIGN:
        return Align(AlignH::LEFT, AlignV::BASELINE);

    case Pid::LINE_STYLE:
        return style().styleV(Sid::palmMuteLineStyle);

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return style().styleV(Sid::palmMuteFontStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
        return PropertyValue::fromValue("P.S."); // TODO: fix the style

    case Pid::END_TEXT:
        return "";

    case Pid::BEGIN_HOOK_TYPE:
        return HookType::NONE;

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::AUTO;

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid PickScrape::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::palmMutePlacement;
    case Pid::BEGIN_FONT_FACE:
        return Sid::palmMuteFontFace;
    case Pid::BEGIN_FONT_SIZE:
    case Pid::CONTINUE_FONT_SIZE:
    case Pid::END_FONT_SIZE:
        return Sid::palmMuteFontSize;
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return Sid::palmMuteFontStyle;
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return Sid::palmMuteTextAlign;
    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Sid::palmMuteHookHeight;
    default:
        break;
    }
    return TextLineBase::getPropertyStyle(id);
}
}
