/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "automationcontroller.h"

#include "engraving/dom/score.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/staff.h"
#include "engraving/types/typesconv.h"

#include "engraving/automation/internal/automation.h"

#include "log.h"

using namespace mu::engraving;

//! NOTE: Linear values
static const std::unordered_map<DynamicType, double> ORDINARY_DYNAMIC_VALUES {
    { DynamicType::N,      0.000 },
    { DynamicType::PPPPPP, 0.071 },
    { DynamicType::PPPPP,  0.143 },
    { DynamicType::PPPP,   0.214 },
    { DynamicType::PPP,    0.286 },
    { DynamicType::PP,     0.357 },
    { DynamicType::P,      0.429 },
    { DynamicType::MP,     0.500 },
    { DynamicType::MF,     0.571 },
    { DynamicType::F,      0.643 },
    { DynamicType::FF,     0.714 },
    { DynamicType::FFF,    0.786 },
    { DynamicType::FFFF,   0.857 },
    { DynamicType::FFFFF,  0.929 },
    { DynamicType::FFFFFF, 1.000 },
};

static const std::unordered_map<DynamicType, double> SINGLE_NOTE_DYNAMIC_VALUES {
    { DynamicType::SF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::SFFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::RFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::RF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
};

static const std::unordered_map<DynamicType, std::pair<double, double> > COMPOUND_DYNAMIC_VALUES {
    { DynamicType::FP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::P) } },
    { DynamicType::PF, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::P), ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) } },
    { DynamicType::SFP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::P) } },
    { DynamicType::SFPP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::PP) } },
};

AutomationController::AutomationController()
{
    m_automation = new Automation();
}

AutomationController::~AutomationController()
{
    delete m_automation;
}

void AutomationController::init(Score* score)
{
    TRACEFUNC;

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        const int tickOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                addSegmentPoints(segment, tickOffset);
            }
        }
    }
}

void AutomationController::addSegmentPoints(const Segment* segment, int tickOffset)
{
    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation) {
            continue;
        }

        if (annotation->isDynamic()) {
            addDynamicPoints(toDynamic(annotation), tickOffset);
        }
    }
}

void AutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset)
{
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = dynamic->staff() ? dynamic->staff()->id() : muse::ID();

    addDynamicPoints(dynamic, tickOffset, key);
}

void AutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    const int dynamicTick = dynamic->tick().ticks() + tickOffset;
    AutomationPoint prevPoint = m_automation->activePoint(key, dynamicTick);

    if (muse::contains(ORDINARY_DYNAMIC_VALUES, dynamic->dynamicType())) {
        AutomationPoint point;
        point.inValue = prevPoint.outValue;
        point.outValue = muse::value(ORDINARY_DYNAMIC_VALUES, dynamic->dynamicType());
        point.interpolation = AutomationPoint::InterpolationType::Linear;
        m_automation->addPoint(key, dynamicTick, point);
        return;
    }

    if (muse::contains(SINGLE_NOTE_DYNAMIC_VALUES, dynamic->dynamicType())) {
        AutomationPoint point;
        point.inValue = prevPoint.outValue;
        point.outValue = muse::value(SINGLE_NOTE_DYNAMIC_VALUES, dynamic->dynamicType());
        point.interpolation = AutomationPoint::InterpolationType::Linear;
        m_automation->addPoint(key, dynamicTick, point);

        if (const Segment* nextSeg = dynamic->segment()->next()) {
            prevPoint.inValue = point.outValue;
            m_automation->addPoint(key, nextSeg->tick().ticks() + tickOffset, prevPoint);
        }

        return;
    }

    if (muse::contains(COMPOUND_DYNAMIC_VALUES, dynamic->dynamicType())) {
        const std::pair<double, double>& values = COMPOUND_DYNAMIC_VALUES.at(dynamic->dynamicType());
        const int startTick = dynamic->tick().ticks() + tickOffset;
        const int endTick = startTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.inValue = prevPoint.outValue;
        startPoint.outValue = values.first;
        startPoint.interpolation = AutomationPoint::InterpolationType::Exponential;
        m_automation->addPoint(key, startTick, startPoint);

        AutomationPoint endPoint;
        endPoint.inValue = startPoint.outValue;
        endPoint.outValue = values.second;
        endPoint.interpolation = AutomationPoint::InterpolationType::Linear;
        m_automation->addPoint(key, endTick, endPoint);

        return;
    }
}
