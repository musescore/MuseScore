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

#include "whammybar.h"
#include "rw/xml.h"
#include "system.h"
#include "measure.h"
#include "chordrest.h"
#include "score.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
static const ElementStyle whammyBarStyle {
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
    { Sid::letRingBeginTextOffset,               Pid::BEGIN_TEXT_OFFSET },
    { Sid::letRingEndHookType,                   Pid::END_HOOK_TYPE },
    { Sid::letRingLineWidth,                     Pid::LINE_WIDTH },
    { Sid::ottava8VAPlacement,                   Pid::PLACEMENT }
};

WhammyBarSegment::WhammyBarSegment(WhammyBar* sp, System* parent)
    : TextLineBaseSegment(ElementType::WHAMMY_BAR_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void WhammyBarSegment::layout()
{
    TextLineBaseSegment::layout();
    autoplaceSpannerSegment();
}

//---------------------------------------------------------
//   WhammyBar
//---------------------------------------------------------

WhammyBar::WhammyBar(EngravingItem* parent)
    : TextLineBase(ElementType::WHAMMY_BAR, parent)
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

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void WhammyBar::read(XmlReader& e)
{
    if (score()->mscVersion() < 301) {
        e.context()->addSpanner(e.intAttribute("id", -1), this);
    }
    while (e.readNextStartElement()) {
        if (readProperty(e.name(), e, Pid::LINE_WIDTH)) {
            setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
        } else if (!TextLineBase::readProperties(e)) {
            e.unknown();
        }
    }
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
        return score()->styleV(Sid::letRingLineWidth);

    case Pid::ALIGN:
        return Align(AlignH::LEFT, AlignV::BASELINE);

    case Pid::LINE_STYLE:
        return score()->styleV(Sid::letRingLineStyle);

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return score()->styleV(Sid::letRingFontStyle);

    case Pid::BEGIN_TEXT:
        return PropertyValue::fromValue("w/bar"); // TODO: fix the style
    case Pid::CONTINUE_TEXT:
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

Sid WhammyBar::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::ottava8VAPlacement; // TODO: fix the style
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

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

PointF WhammyBar::linePos(Grip grip, System** sys) const
{
    qreal x = 0.0;
    qreal nhw = score()->noteHeadWidth();
    System* s = nullptr;
    if (grip == Grip::START) {
        ChordRest* c = toChordRest(startElement());
        if (!c) {
            return PointF();
        }
        s = c->segment()->system();
        x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
        if (c->isRest() && c->durationType() == DurationType::V_MEASURE) {
            x -= c->x();
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
            if (c->isRest() && c->durationType() == DurationType::V_MEASURE) {
                x -= c->x();
            }
        }
        if (!s) {
            Fraction t = tick2();
            Measure* m = score()->tick2measure(t);
            s = m->system();
            x = m->tick2pos(t);
        }
        x += nhw;
    }

    *sys = s;
    return PointF(x, 0);
}
}
