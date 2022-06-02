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

#include "tempochangeranged.h"

#include "log.h"

#include "score.h"
#include "segment.h"
#include "measure.h"
#include "chordrest.h"
#include "rw/xml.h"

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
    { Sid::tempoChangeLineStyle, Pid::LINE_STYLE }
};

static const ElementStyle tempoSegmentStyle {
    { Sid::tempoMinDistance, Pid::MIN_DISTANCE }
};

static const std::unordered_map<TempoChangeType, float> DEFAULT_FACTORS_MAP {
    { TempoChangeType::Accelerando, 1.25 },
    { TempoChangeType::Allargando, 0.75 },
    { TempoChangeType::Calando, 0.5 },
    { TempoChangeType::Lentando, 0.75 },
    { TempoChangeType::Morendo, 0.5 },
    { TempoChangeType::Precipitando, 1.15 },
    { TempoChangeType::Rallentando, 0.75 },
    { TempoChangeType::Ritardando, 0.75 },
    { TempoChangeType::Smorzando, 0.5 },
    { TempoChangeType::Sostenuto, 0.95 },
    { TempoChangeType::Stringendo, 1.5 }
};

TempoChangeRanged::TempoChangeRanged(EngravingItem* parent)
    : ChordTextLineBase(ElementType::TEMPO_RANGED_CHANGE, parent)
{
    initElementStyle(&tempoStyle);
    resetProperty(Pid::LINE_VISIBLE);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::BEGIN_TEXT);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT);
    resetProperty(Pid::END_TEXT_PLACE);
    resetProperty(Pid::END_TEXT);
}

TempoChangeRanged* TempoChangeRanged::clone() const
{
    return new TempoChangeRanged(*this);
}

void TempoChangeRanged::read(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const AsciiString tag(reader.name());

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

void TempoChangeRanged::write(XmlWriter& writer) const
{
    writer.startElement(this);
    writeProperty(writer, Pid::TEMPO_CHANGE_TYPE);
    writeProperty(writer, Pid::TEMPO_EASING_METHOD);
    writeProperty(writer, Pid::TEMPO_CHANGE_FACTOR);
    TextLineBase::writeProperties(writer);
    writer.endElement();
}

LineSegment* TempoChangeRanged::createLineSegment(System* parent)
{
    TempoChangeRangedSegment* lineSegment = new TempoChangeRangedSegment(this, parent);
    lineSegment->setTrack(track());
    lineSegment->initElementStyle(&tempoSegmentStyle);
    return lineSegment;
}

TempoChangeType TempoChangeRanged::tempoChangeType() const
{
    return m_tempoChangeType;
}

ChangeMethod TempoChangeRanged::easingMethod() const
{
    return m_tempoEasingMethod;
}

void TempoChangeRanged::setTempoChangeType(const TempoChangeType type)
{
    m_tempoChangeType = type;
}

float TempoChangeRanged::tempoChangeFactor() const
{
    if (m_tempoChangeFactor.has_value()) {
        return m_tempoChangeFactor.value();
    }

    return mu::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);
}

PropertyValue TempoChangeRanged::getProperty(Pid id) const
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

bool TempoChangeRanged::setProperty(Pid id, const PropertyValue& val)
{
    switch (id) {
    case Pid::TEMPO_CHANGE_TYPE:
        m_tempoChangeType = TempoChangeType(val.toInt());
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

PropertyValue TempoChangeRanged::propertyDefault(Pid propertyId) const
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
        return TempoChangeType::Undefined;
    case Pid::TEMPO_EASING_METHOD:
        return ChangeMethod::NORMAL;
    case Pid::TEMPO_CHANGE_FACTOR:
        return mu::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

Sid TempoChangeRanged::getPropertyStyle(Pid id) const
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

void TempoChangeRanged::added()
{
    requestToRebuildTempo();
}

void TempoChangeRanged::removed()
{
    requestToRebuildTempo();
}

void TempoChangeRanged::requestToRebuildTempo()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMap();
}

TempoChangeRangedSegment::TempoChangeRangedSegment(TempoChangeRanged* annotation, System* parent)
    : TextLineBaseSegment(ElementType::TEMPO_RANGED_CHANGE_SEGMENT, annotation, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

TempoChangeRangedSegment* TempoChangeRangedSegment::clone() const
{
    return new TempoChangeRangedSegment(*this);
}

TempoChangeRanged* TempoChangeRangedSegment::tempoChange() const
{
    return static_cast<TempoChangeRanged*>(spanner());
}

void TempoChangeRangedSegment::layout()
{
    TextLineBaseSegment::layout();
    autoplaceSpannerSegment();
}

void TempoChangeRangedSegment::endEdit(EditData& editData)
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    TextLineBaseSegment::endEdit(editData);
    tempoChange()->requestToRebuildTempo();
}

void TempoChangeRangedSegment::added()
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    tempoChange()->requestToRebuildTempo();
}

void TempoChangeRangedSegment::removed()
{
    IF_ASSERT_FAILED(tempoChange()) {
        return;
    }

    tempoChange()->requestToRebuildTempo();
}
