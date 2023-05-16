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

#include "pedal.h"

#include "chordrest.h"
#include "measure.h"
#include "score.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
static const ElementStyle pedalStyle {
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
    { Sid::pedalPosBelow,                      Pid::OFFSET },
    { Sid::pedalFontSpatiumDependent,          Pid::TEXT_SIZE_SPATIUM_DEPENDENT }
};

const String Pedal::PEDAL_SYMBOL = u"<sym>keyboardPedalPed</sym>";
const String Pedal::STAR_SYMBOL = u"<sym>keyboardPedalUp</sym>";

PedalSegment::PedalSegment(Pedal* sp, System* parent)
    : TextLineBaseSegment(ElementType::PEDAL_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
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
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::pedalPosAbove : Sid::pedalPosBelow;
    }
    return TextLineBase::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(EngravingItem* parent)
    : TextLineBase(ElementType::PEDAL, parent)
{
    initElementStyle(&pedalStyle);
    setLineVisible(true);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT);

    resetProperty(Pid::LINE_WIDTH);
    resetProperty(Pid::LINE_STYLE);

    resetProperty(Pid::BEGIN_HOOK_TYPE);
    resetProperty(Pid::END_HOOK_TYPE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::LINE_VISIBLE);
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
        return score()->styleMM(Sid::pedalLineWidth);              // return point, not spatium

    case Pid::LINE_STYLE:
        return score()->styleV(Sid::pedalLineStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return "";

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;

    case Pid::BEGIN_HOOK_TYPE:
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::PLACEMENT:
        return score()->styleV(Sid::pedalPlacement);

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

PointF Pedal::linePos(Grip grip, System** sys) const
{
    double x = 0.0;
    double nhw = score()->noteHeadWidth();
    System* s = nullptr;
    if (grip == Grip::START) {
        ChordRest* c = toChordRest(startElement());
        if (c) {
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->type() == ElementType::REST && c->durationType() == DurationType::V_MEASURE) {
                x -= c->x();
            }
            if (beginHookType() == HookType::HOOK_45) {
                x += nhw * .5;
            }
        }
    } else {
        EngravingItem* e = endElement();
        ChordRest* c = toChordRest(endElement());
        if (!e || e == startElement() || (endHookType() == HookType::HOOK_90)) {
            // pedal marking on single note or ends with non-angled hook:
            // extend to next note or end of measure
            Segment* seg = nullptr;
            if (!e) {
                seg = startSegment();
            } else {
                seg = c->segment();
            }
            if (seg) {
                seg = seg->next();
                for (; seg; seg = seg->next()) {
                    if (seg->segmentType() == SegmentType::ChordRest) {
                        // look for a chord/rest in any voice on this staff
                        bool crFound = false;
                        track_idx_t track = staffIdx() * VOICES;
                        for (voice_idx_t i = 0; i < VOICES; ++i) {
                            if (seg->element(track + i)) {
                                crFound = true;
                                break;
                            }
                        }
                        if (crFound) {
                            break;
                        }
                    } else if (seg->segmentType() == SegmentType::EndBarLine) {
                        if (!seg->enabled()) {
                            // disabled barline layout is not reliable
                            // use width of measure instead
                            Measure* m = seg->measure();
                            s = seg->system();
                            x = m->width() + m->pos().x() - nhw * 2;
                            seg = nullptr;
                        }
                        break;
                    }
                }
            }
            if (seg) {
                s = seg->system();
                x = seg->pos().x() + seg->measure()->pos().x() - nhw * 2;
            }
        } else if (c) {
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->type() == ElementType::REST && c->durationType() == DurationType::V_MEASURE) {
                x -= c->x();
            }
        }
        if (!s) {
            Fraction t = tick2();
            Measure* m = score()->tick2measure(t);
            s = m->system();
            x = m->tick2pos(t);
        }
        if (endHookType() == HookType::HOOK_45) {
            x += nhw * .5;
        } else {
            x += nhw;
        }
    }

    *sys = s;
    return PointF(x, 0);
}
}
