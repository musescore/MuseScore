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

#include "harmonicmark.h"

#include "score.h"
#include "stafftype.h"
#include "system.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle harmonicMarkStyle {
    { Sid::letRingFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::letRingFontFace,                      Pid::CONTINUE_FONT_FACE },
    { Sid::letRingFontFace,                      Pid::END_FONT_FACE },
    { Sid::letRingFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::letRingFontSize,                      Pid::CONTINUE_FONT_SIZE },
    { Sid::letRingFontSize,                      Pid::END_FONT_SIZE },
    { Sid::letRingFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::letRingFontStyle,                     Pid::CONTINUE_FONT_STYLE },
    { Sid::letRingFontStyle,                     Pid::END_FONT_STYLE },
    { Sid::letRingTextAlign,                     Pid::BEGIN_TEXT_ALIGN },
    { Sid::letRingTextAlign,                     Pid::CONTINUE_TEXT_ALIGN },
    { Sid::letRingTextAlign,                     Pid::END_TEXT_ALIGN },
    { Sid::letRingHookHeight,                    Pid::BEGIN_HOOK_HEIGHT },
    { Sid::letRingHookHeight,                    Pid::END_HOOK_HEIGHT },
    { Sid::letRingLineStyle,                     Pid::LINE_STYLE },
    { Sid::letRingDashLineLen,                   Pid::DASH_LINE_LEN },
    { Sid::letRingDashGapLen,                    Pid::DASH_GAP_LEN },
    { Sid::letRingFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
    { Sid::letRingEndHookType,                   Pid::END_HOOK_TYPE },
    { Sid::letRingLineWidth,                     Pid::LINE_WIDTH },
    { Sid::ottava8VAPlacement,                   Pid::PLACEMENT }
};

HarmonicMarkSegment::HarmonicMarkSegment(HarmonicMark* sp, System* parent)
    : TextLineBaseSegment(ElementType::HARMONIC_MARK_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   HarmonicMark
//---------------------------------------------------------

HarmonicMark::HarmonicMark(EngravingItem* parent)
    : ChordTextLineBase(ElementType::HARMONIC_MARK, parent)
{
    initElementStyle(&harmonicMarkStyle);
    resetProperty(Pid::LINE_VISIBLE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::END_TEXT);
}

bool HarmonicMark::setProperty(Pid propertyId, const PropertyValue& value)
{
    if (propertyId == Pid::BEGIN_TEXT || propertyId == Pid::CONTINUE_TEXT) {
        m_text = value.value<String>();
    }

    return TextLineBase::setProperty(propertyId, value);
}

static const ElementStyle harmonicMarkSegmentStyle {
    { Sid::letRingMinDistance,    Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* HarmonicMark::createLineSegment(System* parent)
{
    HarmonicMarkSegment* hm = new HarmonicMarkSegment(this, parent);
    hm->setTrack(track());
    hm->initElementStyle(&harmonicMarkSegmentStyle);
    return hm;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue HarmonicMark::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return style().styleV(Sid::letRingLineWidth);

    case Pid::ALIGN:
        return Align(AlignH::LEFT, AlignV::BASELINE);

    case Pid::LINE_STYLE:
        return style().styleV(Sid::letRingLineStyle);

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return style().styleV(Sid::letRingFontStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
        return PropertyValue::fromValue(m_text); // TODO: fix the style
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

Sid HarmonicMark::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::ottava8VAPlacement;
    case Pid::BEGIN_FONT_FACE:
        return Sid::letRingFontFace;
    case Pid::BEGIN_FONT_SIZE:
    case Pid::CONTINUE_FONT_SIZE:
    case Pid::END_FONT_SIZE:
        return Sid::letRingFontSize;
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return Sid::letRingFontStyle;
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return Sid::letRingTextAlign;
    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Sid::letRingHookHeight;
    default:
        break;
    }
    return TextLineBase::getPropertyStyle(id);
}
}
