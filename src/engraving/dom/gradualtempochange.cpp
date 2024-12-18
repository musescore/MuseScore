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

#include "gradualtempochange.h"

#include "measure.h"
#include "score.h"
#include "segment.h"
#include "system.h"
#include "tempotext.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const ElementStyle tempoStyle {
    { Sid::tempoChangeSystemFlag, Pid::SYSTEM_FLAG },
    { Sid::tempoChangePlacement, Pid::PLACEMENT },
    { Sid::tempoChangeMinDistance, Pid::MIN_DISTANCE },
    { Sid::tempoChangeLineSpacing, Pid::TEXT_LINE_SPACING },

    { Sid::tempoChangeColor, Pid::COLOR },
    { Sid::tempoChangePosAbove, Pid::OFFSET },

    { Sid::tempoChangeFontFace, Pid::BEGIN_FONT_FACE },
    { Sid::tempoChangeFontFace, Pid::CONTINUE_FONT_FACE },
    { Sid::tempoChangeFontFace, Pid::END_FONT_FACE },

    { Sid::tempoChangeFontSize, Pid::BEGIN_FONT_SIZE },
    { Sid::tempoChangeFontSize, Pid::CONTINUE_FONT_SIZE },
    { Sid::tempoChangeFontSize, Pid::END_FONT_SIZE },

    { Sid::tempoChangeFontStyle, Pid::BEGIN_FONT_STYLE },
    { Sid::tempoChangeFontStyle, Pid::CONTINUE_FONT_STYLE },
    { Sid::tempoChangeFontStyle, Pid::END_FONT_STYLE },

    { Sid::tempoChangeAlign, Pid::BEGIN_TEXT_ALIGN },
    { Sid::tempoChangeAlign, Pid::CONTINUE_TEXT_ALIGN },
    { Sid::tempoChangeAlign, Pid::END_TEXT_ALIGN },

    { Sid::tempoChangeFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::tempoChangeLineWidth, Pid::LINE_WIDTH },
    { Sid::tempoChangeLineStyle, Pid::LINE_STYLE },
    { Sid::tempoChangeDashLineLen, Pid::DASH_LINE_LEN },
    { Sid::tempoChangeDashGapLen, Pid::DASH_GAP_LEN },
    { Sid::tempoChangeFontSpatiumDependent, Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
};

static const ElementStyle tempoSegmentStyle {
    { Sid::tempoChangePosAbove, Pid::OFFSET },
    { Sid::tempoChangeMinDistance, Pid::MIN_DISTANCE }
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

LineSegment* GradualTempoChange::createLineSegment(System* parent)
{
    GradualTempoChangeSegment* lineSegment = new GradualTempoChangeSegment(this, parent);
    lineSegment->setTrack(track());
    return lineSegment;
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

    return muse::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);
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
    case Pid::SNAP_AFTER:
        return snapToItemAfter();
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
    case Pid::SNAP_AFTER:
        setSnapToItemAfter(val.toBool());
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
        return style().styleV(Sid::tempoChangeAlign);

    case Pid::LINE_WIDTH:
        return style().styleV(Sid::tempoChangeLineWidth);
    case Pid::LINE_STYLE:
        return style().styleV(Sid::tempoChangeLineStyle);
    case Pid::LINE_VISIBLE:
        return true;

    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF(0, 0));

    case Pid::BEGIN_FONT_STYLE:
        return style().styleV(Sid::tempoChangeFontStyle);

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
        return muse::value(DEFAULT_FACTORS_MAP, m_tempoChangeType, 1.0);

    case Pid::SNAP_AFTER:
        return true;

    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

Sid GradualTempoChange::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return Sid::tempoChangePlacement;
    case Pid::BEGIN_FONT_FACE:
    case Pid::CONTINUE_FONT_FACE:
    case Pid::END_FONT_FACE:
        return Sid::tempoChangeFontFace;
    case Pid::BEGIN_FONT_SIZE:
    case Pid::CONTINUE_FONT_SIZE:
    case Pid::END_FONT_SIZE:
        return Sid::tempoChangeFontSize;
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return Sid::tempoChangeFontStyle;
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return Sid::tempoChangeAlign;
    case Pid::BEGIN_TEXT:
        return Sid::letRingText;
    case Pid::OFFSET:
        if (placeAbove()) {
            return Sid::tempoChangePosAbove;
        } else {
            return Sid::tempoChangePosBelow;
        }
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

    score()->setUpTempoMapLater();
}

GradualTempoChangeSegment::GradualTempoChangeSegment(GradualTempoChange* annotation, System* parent)
    : TextLineBaseSegment(ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, annotation, parent,
                          ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
{
    initElementStyle(&tempoSegmentStyle);
}

GradualTempoChangeSegment* GradualTempoChangeSegment::clone() const
{
    return new GradualTempoChangeSegment(*this);
}

GradualTempoChange* GradualTempoChangeSegment::tempoChange() const
{
    return static_cast<GradualTempoChange*>(spanner());
}

Sid GradualTempoChangeSegment::getPropertyStyle(Pid id) const
{
    if (id == Pid::OFFSET) {
        if (placeAbove()) {
            return Sid::tempoPosAbove;
        } else {
            return Sid::tempoPosBelow;
        }
    }
    return TextLineBaseSegment::getPropertyStyle(id);
}

GradualTempoChangeSegment* GradualTempoChangeSegment::findElementToSnapBefore() const
{
    const System* sys = system();
    IF_ASSERT_FAILED(sys) {
        return nullptr;
    }

    GradualTempoChange* thisTempoChange = tempoChange();
    Fraction startTick = thisTempoChange->tick();
    if (!sys->measures().empty() && startTick == sys->measures().front()->tick()) {
        return nullptr;
    }

    auto intervals = score()->spannerMap().findOverlapping(startTick.ticks(), startTick.ticks());
    for (auto interval : intervals) {
        Spanner* spanner = interval.value;
        bool isValidTempoChange = spanner->isGradualTempoChange() && !spanner->segmentsEmpty() && spanner->visible()
                                  && spanner != thisTempoChange;
        if (!isValidTempoChange) {
            continue;
        }

        GradualTempoChange* precedingTempoChange = toGradualTempoChange(spanner);
        bool endsMatch = precedingTempoChange->track() == thisTempoChange->track()
                         && precedingTempoChange->tick2() == startTick
                         && precedingTempoChange->placeAbove() == thisTempoChange->placeAbove();

        if (endsMatch && precedingTempoChange->snapToItemAfter()) {
            return toGradualTempoChangeSegment(precedingTempoChange->backSegment());
        }
    }

    return nullptr;
}

TempoText* GradualTempoChangeSegment::findElementToSnapAfter() const
{
    if (!tempoChange()->snapToItemAfter()) {
        return nullptr;
    }

    System* sys = system();
    IF_ASSERT_FAILED(sys) {
        return nullptr;
    }

    // Note: we don't need to look for a tempoChange after.
    // It is the next tempoChange which looks for a tempoChange before.
    Fraction refTick = tempoChange()->tick2();
    Measure* measure = score()->tick2measureMM(refTick);
    if (!measure) {
        return nullptr;
    }

    for (Segment* segment = measure->last(); segment; segment = segment->prev1()) {
        if (segment->system() != sys) {
            continue;
        }
        Fraction segmentTick = segment->tick();
        if (segmentTick > refTick) {
            continue;
        }
        if (segmentTick < refTick) {
            break;
        }
        EngravingItem* tempoText = segment->findAnnotation(ElementType::TEMPO_TEXT, track(), track());
        if (tempoText && tempoText->placeAbove() == placeAbove() && tempoText->visible()) {
            return toTempoText(tempoText);
        }
    }

    return nullptr;
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
