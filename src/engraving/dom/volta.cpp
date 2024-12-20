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

#include "volta.h"

#include <algorithm>
#include <vector>

#include "types/typesconv.h"

#include "barline.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "tempo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle voltaStyle {
    { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::voltaFontFace,                      Pid::CONTINUE_FONT_FACE },
    { Sid::voltaFontFace,                      Pid::END_FONT_FACE },
    { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::voltaFontSize,                      Pid::CONTINUE_FONT_SIZE },
    { Sid::voltaFontSize,                      Pid::END_FONT_SIZE },
    { Sid::voltaFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::voltaFontStyle,                     Pid::CONTINUE_FONT_STYLE },
    { Sid::voltaFontStyle,                     Pid::END_FONT_STYLE },
    { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN },
    { Sid::voltaAlign,                         Pid::CONTINUE_TEXT_ALIGN },
    { Sid::voltaAlign,                         Pid::END_TEXT_ALIGN },
    { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET },
    { Sid::voltaOffset,                        Pid::CONTINUE_TEXT_OFFSET },
    { Sid::voltaOffset,                        Pid::END_TEXT_OFFSET },
    { Sid::voltaLineWidth,                     Pid::LINE_WIDTH },
    { Sid::voltaLineStyle,                     Pid::LINE_STYLE },
    { Sid::voltaDashLineLen,                   Pid::DASH_LINE_LEN },
    { Sid::voltaDashGapLen,                    Pid::DASH_GAP_LEN },
    { Sid::voltaHook,                          Pid::BEGIN_HOOK_HEIGHT },
    { Sid::voltaHook,                          Pid::END_HOOK_HEIGHT },
    { Sid::voltaPosAbove,                      Pid::OFFSET },
    { Sid::voltaFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
};

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

VoltaSegment::VoltaSegment(Volta* sp, System* parent)
    : TextLineBaseSegment(ElementType::VOLTA_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
{
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* VoltaSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::BEGIN_HOOK_TYPE || pid == Pid::END_HOOK_TYPE || pid == Pid::VOLTA_ENDING) {
        return spanner();
    }
    return TextLineBaseSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(EngravingItem* parent)
    : TextLineBase(ElementType::VOLTA, parent, ElementFlag::SYSTEM)
{
    setPlacement(PlacementV::ABOVE);
    initElementStyle(&voltaStyle);

    setBeginTextPlace(TextPlace::BELOW);
    setContinueTextPlace(TextPlace::BELOW);
    setLineVisible(true);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT);
    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::BEGIN_HOOK_TYPE);
    resetProperty(Pid::END_HOOK_TYPE);

    setAnchor(VOLTA_ANCHOR);
}

///
/// \brief sorts the provided list in ascending order
///
void Volta::setEndings(const std::vector<int>& l)
{
    m_endings = l;
    std::sort(m_endings.begin(), m_endings.end());
}

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const String& s)
{
    setBeginText(s);
}

//---------------------------------------------------------
//   text
//---------------------------------------------------------

String Volta::text() const
{
    return beginText();
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle voltaSegmentStyle {
    { Sid::voltaPosAbove,                      Pid::OFFSET },
    { Sid::voltaMinDistance,                   Pid::MIN_DISTANCE },
};

LineSegment* Volta::createLineSegment(System* parent)
{
    VoltaSegment* vs = new VoltaSegment(this, parent);
    vs->setTrack(track());
    vs->initElementStyle(&voltaSegmentStyle);
    return vs;
}

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
{
    for (int ending : endings()) {
        if (ending == repeat) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   firstEnding
//---------------------------------------------------------

int Volta::firstEnding() const
{
    if (m_endings.empty()) {
        return 0;
    }
    return m_endings.front();
}

//---------------------------------------------------------
//   lastEnding
//---------------------------------------------------------

int Volta::lastEnding() const
{
    if (m_endings.empty()) {
        return 0;
    }
    return m_endings.back();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Volta::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VOLTA_ENDING:
        return PropertyValue::fromValue(endings());
    default:
        break;
    }
    return TextLineBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::VOLTA_ENDING: {
        setEndings(val.value<std::vector<int> >());
    } break;
    default:
        if (!TextLineBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Volta::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VOLTA_ENDING:
        return PropertyValue::fromValue(std::vector<int>());
    case Pid::ANCHOR:
        return int(VOLTA_ANCHOR);
    case Pid::BEGIN_HOOK_TYPE:
        return HookType::HOOK_90;
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;
    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return "";
    case Pid::LINE_VISIBLE:
        return true;
    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::ABOVE;

    case Pid::PLACEMENT:
        return PlacementV::ABOVE;

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Volta::setChannel() const
{
    Measure* startMeasure = Spanner::startMeasure();
    Measure* endMeasure = Spanner::endMeasure();

    if (startMeasure && endMeasure) {
        if (!endMeasure->repeatEnd()) {
            return;
        }

        Fraction startTick = startMeasure->tick() - Fraction::fromTicks(1);
        Fraction endTick  = endMeasure->endTick() - Fraction::fromTicks(1);
        Staff* st = staff();
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            int channel = st->channel(startTick, voice);
            st->insertIntoChannelList(voice, endTick, channel);
        }
    }
}

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void Volta::setTempo() const
{
    Measure* startMeasure = Spanner::startMeasure();
    Measure* endMeasure = Spanner::endMeasure();

    if (startMeasure && endMeasure) {
        if (!endMeasure->repeatEnd()) {
            return;
        }
        Fraction startTick = startMeasure->tick() - Fraction::fromTicks(1);
        Fraction endTick  = endMeasure->endTick() - Fraction::fromTicks(1);
        BeatsPerSecond tempoBeforeVolta = score()->tempomap()->tempo(startTick.ticks());
        score()->setTempo(endTick, tempoBeforeVolta);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Volta::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), text());
}

PointF Volta::linePos(Grip grip, System** system) const
{
    bool start = grip == Grip::START;

    Segment* segment = score()->tick2leftSegment(start ? tick() : tick2(), true,
                                                 SegmentType::ChordRest | SegmentType::StartRepeatBarLine | SegmentType::EndBarLine);
    if (!segment) {
        return PointF();
    }

    const Measure* measure = segment->measure();
    bool isAtSystemStart = segment->rtick().isZero() && measure && measure->system() && measure->isFirstInSystem();

    if (start && segment->rtick().isZero()) {
        while (!segment->isType(SegmentType::BarLineType)) {
            Segment* prev = segment->prev1MMenabled();
            if (prev && (prev->isType(SegmentType::BarLineType) || (prev->tick() == segment->tick() && !isAtSystemStart))) {
                segment = prev;
            } else {
                break;
            }
        }
    } else if (!start) {
        Segment* prev = segment;
        while (prev && !prev->isEndBarLineType() && prev->tick() == segment->tick()) {
            prev = prev->prev1MMenabled();
        }
        if (prev && prev->isEndBarLineType()) {
            segment = prev;
        }
    }

    *system = segment->measure()->system();
    double x = segment->x() + segment->measure()->x();

    if (start) {
        if (segment->isChordRestType()) {
            x -= style().styleMM(Sid::barNoteDistance);
        } else if (segment->segmentType() & SegmentType::BarLineType && !isAtSystemStart) {
            x += segment->width();
        }
        x += (isAtSystemStart ? 0.5 : -0.5) * absoluteFromSpatium(lineWidth());
    } else {
        if ((*system) && segment->tick() == (*system)->endTick()) {
            x += segment->staffShape(staffIdxOrNextVisible()).right();
            x -= 0.5 * absoluteFromSpatium(lineWidth());
        } else if (segment->segmentType() & SegmentType::BarLineType) {
            BarLine* barLine = toBarLine(segment->elementAt(track()));
            if (barLine->barLineType() == BarLineType::END_REPEAT || barLine->barLineType() == BarLineType::END_START_REPEAT) {
                x += symWidth(SymId::repeatDot) + style().styleMM(Sid::repeatBarlineDotSeparation);
            }
            x += 0.5 * absoluteFromSpatium(lineWidth());
        }
    }

    return PointF(x, 0.0);
}

//---------------------------------------------------------
//   setVoltaType
//    deprecated
//---------------------------------------------------------

void Volta::setVoltaType(Type val)
{
    setEndHookType(Type::CLOSED == val ? HookType::HOOK_90 : HookType::NONE);
}

//---------------------------------------------------------
//   voltaType
//    deprecated
//---------------------------------------------------------

Volta::Type Volta::voltaType() const
{
    return endHookType() != HookType::NONE ? Type::CLOSED : Type::OPEN;
}
}
