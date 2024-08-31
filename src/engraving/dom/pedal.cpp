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

#include "pedal.h"

#include "chord.h"
#include "chordrest.h"
#include "measure.h"
#include "note.h"
#include "score.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
static const ElementStyle pedalStyle {
    { Sid::pedalText,                          Pid::BEGIN_TEXT },
    { Sid::pedalContinueText,                  Pid::CONTINUE_TEXT },
    { Sid::pedalRosetteEndText,                Pid::END_TEXT },
    { Sid::pedalFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::pedalFontFace,                      Pid::CONTINUE_FONT_FACE },
    { Sid::pedalFontFace,                      Pid::END_FONT_FACE },
    { Sid::pedalFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::pedalFontSize,                      Pid::CONTINUE_FONT_SIZE },
    { Sid::pedalFontSize,                      Pid::END_FONT_SIZE },
    { Sid::pedalFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::pedalFontStyle,                     Pid::CONTINUE_FONT_STYLE },
    { Sid::pedalFontStyle,                     Pid::END_FONT_STYLE },
    { Sid::pedalTextAlign,                     Pid::BEGIN_TEXT_ALIGN },
    { Sid::pedalTextAlign,                     Pid::CONTINUE_TEXT_ALIGN },
    { Sid::pedalTextAlign,                     Pid::END_TEXT_ALIGN },
    { Sid::pedalHookHeight,                    Pid::BEGIN_HOOK_HEIGHT },
    { Sid::pedalHookHeight,                    Pid::END_HOOK_HEIGHT },
    { Sid::pedalLineWidth,                     Pid::LINE_WIDTH },
    { Sid::pedalDashLineLen,                   Pid::DASH_LINE_LEN },
    { Sid::pedalDashGapLen,                    Pid::DASH_GAP_LEN },
    { Sid::pedalPlacement,                     Pid::PLACEMENT },
    { Sid::pedalLineStyle,                     Pid::LINE_STYLE },
    { Sid::pedalPosBelow,                      Pid::OFFSET },
    { Sid::pedalFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT }
};

const String Pedal::PEDAL_SYMBOL = u"<sym>keyboardPedalPed</sym>";
const String Pedal::STAR_SYMBOL = u"<sym>keyboardPedalUp</sym>";

PedalSegment::PedalSegment(Pedal* sp, System* parent)
    : TextLineBaseSegment(ElementType::PEDAL_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_text->setTextStyleType(TextStyleType::PEDAL);
    m_endText->setTextStyleType(TextStyleType::PEDAL);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid PedalSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return spanner()->placeAbove() ? Sid::pedalPosAbove : Sid::pedalPosBelow;
    }
    return TextLineBaseSegment::getPropertyStyle(pid);
}

Sid Pedal::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::OFFSET:
        return placeAbove() ? Sid::pedalPosAbove : Sid::pedalPosBelow;
    case Pid::END_TEXT:
        return lineVisible() ? Sid::pedalEndText : Sid::pedalRosetteEndText;
    case Pid::BEGIN_TEXT:
        return beginHookType() == HookType::NONE ? Sid::pedalText : Sid::pedalHookText;
    case Pid::CONTINUE_TEXT:
        return beginHookType() == HookType::NONE ? Sid::pedalContinueText : Sid:: pedalContinueHookText;
    default:
        return TextLineBase::getPropertyStyle(pid);
    }
}

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(EngravingItem* parent)
    : TextLineBase(ElementType::PEDAL, parent)
{
    initElementStyle(&pedalStyle);
    setLineVisible(true);

    resetProperty(Pid::LINE_WIDTH);
    resetProperty(Pid::LINE_STYLE);

    resetProperty(Pid::BEGIN_HOOK_TYPE);
    resetProperty(Pid::END_HOOK_TYPE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::LINE_VISIBLE);
    resetProperty(Pid::END_TEXT);
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle pedalSegmentStyle {
    { Sid::pedalPosBelow, Pid::OFFSET },
    { Sid::pedalMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Pedal::createLineSegment(System* parent)
{
    PedalSegment* p = new PedalSegment(this, parent);
    p->setTrack(track());
    p->initElementStyle(&pedalSegmentStyle);
    return p;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Pedal::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return style().styleS(Sid::pedalLineWidth);              // return point, not spatium

    case Pid::LINE_STYLE:
        return style().styleV(Sid::pedalLineStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return style().styleV(getPropertyStyle(propertyId));

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;

    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_HOOK_TYPE:
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::PLACEMENT:
        return style().styleV(Sid::pedalPlacement);

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

Pedal* Pedal::findNextInStaff() const
{
    Fraction endTick = tick2();
    auto spanners = score()->spannerMap().findOverlapping(endTick.ticks(), score()->endTick().ticks());
    for (auto element : spanners) {
        Spanner* spanner = element.value;
        if (spanner->isPedal() && spanner != this && spanner->staffIdx() == staffIdx() && spanner->tick() == endTick) {
            return toPedal(spanner);
        }
    }

    return nullptr;
}

bool Pedal::connect45HookToNext() const
{
    if (endHookType() != HookType::HOOK_45) {
        return false;
    }

    Pedal* nextPedal = findNextInStaff();

    return nextPedal && nextPedal->tick() == tick2() && nextPedal->beginHookType() == HookType::HOOK_45;
}

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

PointF Pedal::linePos(Grip grip, System** sys) const
{
    bool start = grip == Grip::START;

    if (start) {
        Segment* startSeg = startSegment();
        if (!startSeg) {
            return PointF();
        }
        *sys = startSeg->measure()->system();
        double x = startSeg->x() + startSeg->measure()->x();
        if (beginText() == "<sym>keyboardPedalPed</sym>") {
            x -= 0.5 * spatium();
        } else if (beginHookType() == HookType::HOOK_90 || beginHookType() == HookType::HOOK_90T) {
            x += 0.5 * absoluteFromSpatium(lineWidth());
        } else if (beginHookType() == HookType::HOOK_45) {
            EngravingItem* item = startElement();
            if (item && item->isChord()) {
                Note* downNote = toChord(item)->downNote();
                x += 0.5 * downNote->headWidth();
            } else if (item && item->isRest()) {
                x += 0.5 * item->width();
            }
        }
        return PointF(x, 0.0);
    }

    Segment* endSeg = endSegment();
    if (!endSeg) {
        return PointF();
    }

    Pedal* nextPedal = findNextInStaff();

    if (nextPedal && endHookType() == HookType::HOOK_45) {
        *sys = endSeg->measure()->system();
        double x = endSeg->x() + endSeg->measure()->x();
        EngravingItem* item = endElement();
        if (item && item->isChord()) {
            Note* downNote = toChord(item)->downNote();
            x += 0.5 * downNote->headWidth();
        } else if (item && item->isRest()) {
            x += 0.5 * item->width();
        }
        return PointF(x, 0.0);
    }

    if (endSeg->rtick() == Fraction(0, 1)) {
        Segment* prevSeg = endSeg->prev1(SegmentType::EndBarLine);
        if (prevSeg) {
            endSeg = prevSeg;
        }
    }

    *sys = endSeg->measure()->system();
    double x = endSeg->x() + endSeg->measure()->x();

    if (endText() == "<sym>keyboardPedalUp</sym>") {
        x -= symWidth(SymId::keyboardPedalUp);
    }

    x -= (endSeg->isChordRestType() && nextPedal ? 1.25 : 0.75) * spatium();

    return PointF(x, 0.0);
}
}
