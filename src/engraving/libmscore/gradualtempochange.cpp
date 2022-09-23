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

#include "gradualtempochange.h"

#include "rw/xml.h"
#include "log.h"

#include "measure.h"
#include "score.h"
#include "segment.h"

using namespace mu;
using namespace mu::engraving;

static const ElementStyle tempoStyle {
    { Sid::tempoSystemFlag, Pid::SYSTEM_FLAG },
    { Sid::tempoPlacement, Pid::PLACEMENT },
    { Sid::tempoMinDistance, Pid::MIN_DISTANCE },
    { Sid::tempoLineSpacing, Pid::TEXT_LINE_SPACING },

    { Sid::tempoColor, Pid::COLOR },
    { Sid::tempoPosAbove, Pid::OFFSET },

    { Sid::tempoFontFace, Pid::BEGIN_FONT_FACE },
    { Sid::tempoFontFace, Pid::CONTINUE_FONT_FACE },
    { Sid::tempoFontFace, Pid::END_FONT_FACE },

    { Sid::tempoFontSize, Pid::BEGIN_FONT_SIZE },
    { Sid::tempoFontSize, Pid::CONTINUE_FONT_SIZE },
    { Sid::tempoFontSize, Pid::END_FONT_SIZE },

    { Sid::tempoFontStyle, Pid::BEGIN_FONT_STYLE },
    { Sid::tempoFontStyle, Pid::CONTINUE_FONT_STYLE },
    { Sid::tempoFontStyle, Pid::END_FONT_STYLE },

    { Sid::tempoAlign, Pid::BEGIN_TEXT_ALIGN },
    { Sid::tempoAlign, Pid::CONTINUE_TEXT_ALIGN },
    { Sid::tempoAlign, Pid::END_TEXT_ALIGN },

    { Sid::tempoFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::tempoChangeLineWidth, Pid::LINE_WIDTH },
    { Sid::tempoChangeLineStyle, Pid::LINE_STYLE },
    { Sid::tempoChangeDashLineLen, Pid::DASH_LINE_LEN },
    { Sid::tempoChangeDashGapLen, Pid::DASH_GAP_LEN }
};

static const ElementStyle tempoSegmentStyle {
    { Sid::tempoMinDistance, Pid::MIN_DISTANCE }
};

static const std::unordered_map<GradualTempoChangeType, double> DEFAULT_FACTORS_MAP {
    { GradualTempoChangeType::Accelerando, 1.33 },
    { GradualTempoChangeType::Allargando, 0.75 },
    { GradualTempoChangeType::Calando, 0.5 },
    { GradualTempoChangeType::Lentando, 0.75 },
    { GradualTempoChangeType::Morendo, 0.5 },
    { GradualTempoChangeType::Precipitando, 1.15 },
    { GradualTempoChangeType::Rallentando, 0.75 },
    { GradualTempoChangeType::Ritardando, 0.75 },
    { GradualTempoChangeType::Smorzando, 0.5 },
    { GradualTempoChangeType::Sostenuto, 0.95 },
    { GradualTempoChangeType::Stringendo, 1.5 }
};

GradualTempoChange::GradualTempoChange(EngravingItem* parent)
    : TextLineBase(ElementType::GRADUAL_TEMPO_CHANGE, parent, ElementFlag::SYSTEM)
{
    initElementStyle(&tempoStyle);
    setAnchor(Anchor::SEGMENT);

    resetProperty(Pid::LINE_VISIBLE);
    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::END_TEXT);
}

GradualTempoChange* GradualTempoChange::clone() const
{
    return new GradualTempoChange(*this);
}

void GradualTempoChange::read(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const AsciiStringView tag(reader.name());

        if (readProperty(tag, reader, Pid::LINE_WIDTH)) {
            setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
            continue;
        }

        if (readProperty(tag, reader, Pid::TEMPO_CHANGE_TYPE)) {
            continue;
        }

        if (readProperty(tag, reader, Pid::TEMPO_EASING_METHOD)) {
            continue;
        }

        if (readProperty(tag, reader, Pid::TEMPO_CHANGE_FACTOR)) {
            continue;
        }

        if (!TextLineBase::readProperties(reader)) {
            reader.unknown();
        }
    }
}

void GradualTempoChange::write(XmlWriter& writer) const
{
    writer.startElement(this);
    writeProperty(writer, Pid::TEMPO_CHANGE_TYPE);
    writeProperty(writer, Pid::TEMPO_EASING_METHOD);
    writeProperty(writer, Pid::TEMPO_CHANGE_FACTOR);
    TextLineBase::writeProperties(writer);
    writer.endElement();
}

LineSegment* GradualTempoChange::createLineSegment(System* parent)
{
    GradualTempoChangeSegment* lineSegment = new GradualTempoChangeSegment(this, parent);
    lineSegment->setTrack(track());
    lineSegment->initElementStyle(&tempoSegmentStyle);
    return lineSegment;
}

SpannerSegment* GradualTempoChange::layoutSystem(System* system)
{
    SpannerSegment* segment = TextLineBase::layoutSystem(system);
    moveToSystemTopIfNeed(segment);

    return segment;
}

GradualTempoChangeType GradualTempoChange::tempoChangeType() const
{
    return m_tempoChangeType;
}

ChangeMethod GradualTempoChange::easingMethod() const
{
    return m_tempoEasingMethod;
}

void GradualTempoChange::setTempoChangeType(const GradualTempoChangeType type)
{
    m_tempoChangeType = type;
}

double GradualTempoChange::tempoChangeFactor() const
{
    if (m_tempoChangeFactor.has_value()) {
        return m_tempoChangeFactor.value();
    }

    return mu::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);
}

PropertyValue GradualTempoChange::getProperty(Pid id) const
{
    switch (id) {
    case Pid::TEMPO_CHANGE_TYPE:
        return m_tempoChangeType;
    case Pid::TEMPO_EASING_METHOD:
        return m_tempoEasingMethod;
    case Pid::TEMPO_CHANGE_FACTOR:
        return tempoChangeFactor();
    default:
        return TextLineBase::getProperty(id);
    }
}

bool GradualTempoChange::setProperty(Pid id, const PropertyValue& val)
{
    switch (id) {
    case Pid::TEMPO_CHANGE_TYPE:
        m_tempoChangeType = GradualTempoChangeType(val.toInt());
        break;
    case Pid::TEMPO_EASING_METHOD:
        m_tempoEasingMethod = ChangeMethod(val.toInt());
        break;
    case Pid::TEMPO_CHANGE_FACTOR:
        m_tempoChangeFactor = val.toReal();
        break;
    default:
        if (!TextLineBase::setProperty(id, val)) {
            return false;
        }
        break;
    }

    triggerLayout();
    return true;
}

PropertyValue GradualTempoChange::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ALIGN:
        return score()->styleV(Sid::tempoAlign);

    case Pid::LINE_WIDTH:
        return score()->styleV(Sid::tempoChangeLineWidth);
    case Pid::LINE_STYLE:
        return score()->styleV(Sid::tempoChangeLineStyle);
    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return score()->styleV(Sid::tempoFontStyle);

    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return "";

    case Pid::BEGIN_HOOK_TYPE:
        return HookType::NONE;

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::AUTO;

    case Pid::TEMPO_CHANGE_TYPE:
        return GradualTempoChangeType::Undefined;
    case Pid::TEMPO_EASING_METHOD:
        return ChangeMethod::NORMAL;
    case Pid::TEMPO_CHANGE_FACTOR:
        return mu::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

Sid GradualTempoChange::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::tempoPlacement;
    case Pid::BEGIN_FONT_FACE:
    case Pid::CONTINUE_FONT_FACE:
    case Pid::END_FONT_FACE:
        return Sid::tempoFontFace;
    case Pid::BEGIN_FONT_SIZE:
    case Pid::CONTINUE_FONT_SIZE:
    case Pid::END_FONT_SIZE:
        return Sid::tempoFontSize;
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return Sid::tempoFontStyle;
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return Sid::tempoAlign;
    case Pid::BEGIN_TEXT:
        return Sid::letRingText;
    default:
        break;
    }
    return TextLineBase::getPropertyStyle(id);
}

void GradualTempoChange::added()
{
    requestToRebuildTempo();
}

void GradualTempoChange::removed()
{
    requestToRebuildTempo();
}

void GradualTempoChange::requestToRebuildTempo()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMap();
}

GradualTempoChangeSegment::GradualTempoChangeSegment(GradualTempoChange* annotation, System* parent)
    : TextLineBaseSegment(ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, annotation, parent,
                          ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
{
}

GradualTempoChangeSegment* GradualTempoChangeSegment::clone() const
{
    return new GradualTempoChangeSegment(*this);
}

GradualTempoChange* GradualTempoChangeSegment::tempoChange() const
{
    return static_cast<GradualTempoChange*>(spanner());
}

void GradualTempoChangeSegment::layout()
{
    TextLineBaseSegment::layout();
    autoplaceSpannerSegment();
}

void GradualTempoChangeSegment::endEdit(EditData& editData)
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    TextLineBaseSegment::endEdit(editData);
    tempoChange()->requestToRebuildTempo();
}

void GradualTempoChangeSegment::added()
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    tempoChange()->requestToRebuildTempo();
}

void GradualTempoChangeSegment::removed()
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    tempoChange()->requestToRebuildTempo();
}
