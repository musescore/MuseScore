/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/hairpin.h"
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

static constexpr double DYNAMIC_STEP(0.071);

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

static std::vector<AutomationCurveKey> resolveKeys(const EngravingItem* item, AutomationType type)
{
    const VoiceAssignment voiceAssignment = item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();

    std::vector<AutomationCurveKey> result;
    AutomationCurveKey key;
    key.type = type;

    switch (voiceAssignment) {
    case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT: {
        const Score* score = item->score();
        const Part* part = item->part();
        IF_ASSERT_FAILED(score && part) {
            return result;
        }

        const staff_idx_t startStaffIdx = track2staff(part->startTrack());
        const staff_idx_t endStaffIdx = startStaffIdx + part->nstaves();
        result.reserve(endStaffIdx - startStaffIdx);

        for (staff_idx_t staffIdx = startStaffIdx; staffIdx < endStaffIdx; ++staffIdx) {
            const Staff* staff = score->staff(staffIdx);
            IF_ASSERT_FAILED(staff) {
                continue;
            }

            if (!staff->isPrimaryStaff()) {
                continue; // ignore linked staves
            }

            key.staffId = staff->id();
            result.push_back(key);
        }
    } break;
    case VoiceAssignment::ALL_VOICE_IN_STAFF: {
        const Staff* staff = item->staff();
        IF_ASSERT_FAILED(staff) {
            return result;
        }

        if (!staff->isPrimaryStaff()) {
            return result; // ignore linked staves
        }

        key.staffId = staff->id();
        result.push_back(key);
    } break;
    case VoiceAssignment::CURRENT_VOICE_ONLY: {
        const Staff* staff = item->staff();
        IF_ASSERT_FAILED(staff) {
            return result;
        }

        if (!staff->isPrimaryStaff()) {
            return result; // ignore linked staves
        }

        key.staffId = staff->id();
        key.voiceIdx = item->voice();
        result.push_back(key);
    } break;
    }

    return result;
}

static std::optional<double> dynamicValue(DynamicType type, bool startValue)
{
    if (auto it = ORDINARY_DYNAMIC_VALUES.find(type); it != ORDINARY_DYNAMIC_VALUES.end()) {
        return it->second;
    }

    if (auto it = SINGLE_NOTE_DYNAMIC_VALUES.find(type); it != SINGLE_NOTE_DYNAMIC_VALUES.end()) {
        return it->second;
    }

    if (auto it = COMPOUND_DYNAMIC_VALUES.find(type); it != COMPOUND_DYNAMIC_VALUES.end()) {
        return startValue ? it->second.first : it->second.second;
    }

    return std::nullopt;
}

static std::optional<double> startHairpinValue(const Hairpin* hairpin)
{
    const DynamicType type = hairpin->dynamicTypeFrom();
    if (type == DynamicType::OTHER) {
        return std::nullopt;
    }

    return dynamicValue(type, false);
}

static DynamicType findEndDynamicType(const Hairpin* hairpin)
{
    const DynamicType textType = hairpin->dynamicTypeTo();
    if (textType != DynamicType::OTHER) {
        return textType;
    }

    if (hairpin->spannerSegments().empty()) {
        const Segment* endSegment = hairpin->endSegment();
        if (!endSegment) {
            return DynamicType::OTHER;
        }

        const track_idx_t trackIdx = hairpin->track();
        const std::vector<EngravingItem*> dynamics = endSegment->findAnnotations(ElementType::DYNAMIC, trackIdx, trackIdx);
        for (const EngravingItem* item : dynamics) {
            if (item && item->isDynamic() && toDynamic(item)->playDynamic()) {
                return toDynamic(item)->dynamicType();
            }
        }

        return DynamicType::OTHER;
    }

    const LineSegment* seg = hairpin->backSegment();
    if (!seg) {
        return DynamicType::OTHER;
    }

    // Optimization: first check if there is a cached dynamic
    const EngravingItem* snappedItem = seg->ldata()->itemSnappedAfter();
    if (!snappedItem || !snappedItem->isDynamic() || !toDynamic(snappedItem)->playDynamic()) {
        snappedItem = toHairpinSegment(seg)->findElementToSnapAfter(false /*ignoreInvisible*/, true /*requirePlayable*/);
        if (!snappedItem || !snappedItem->isDynamic() || !toDynamic(snappedItem)->playDynamic()) {
            return DynamicType::OTHER;
        }
    }

    return toDynamic(snappedItem)->dynamicType();
}

static std::optional<double> endHairpinValue(const Hairpin* hairpin)
{
    const DynamicType type = findEndDynamicType(hairpin);
    if (type == DynamicType::OTHER) {
        return std::nullopt;
    }

    return dynamicValue(type, true);
}

static bool isEndDynamicOfHairpin(const Dynamic* dynamic, double dynamicValue, double prevValue)
{
    const SpannerMap& spannerMap = dynamic->score()->spannerMap();
    if (spannerMap.empty()) {
        return false;
    }

    const int dynamicTick = dynamic->tick().ticks();
    const auto& intervals = spannerMap.findOverlapping(dynamicTick - 1, dynamicTick + 1);

    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;
        if (!spanner->isHairpin()) {
            continue;
        }

        if (spanner->track() != dynamic->track()) {
            continue;
        }

        if (spanner->tick2().ticks() != dynamicTick) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);
        std::optional<double> startValue = startHairpinValue(hairpin);
        if (!startValue.has_value()) {
            startValue = prevValue;
        }

        if (hairpin->isCrescendo()) {
            return dynamicValue > startValue.value();
        }

        return dynamicValue < startValue.value();
    }

    return false;
}

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

    m_automation->clear();

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        const int tickOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                addSegmentPoints(segment, tickOffset);
            }
        }

        addSpannerPoints(score, repeatSegment->tick, repeatSegment->endTick(), tickOffset);
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
    if (!dynamic->playDynamic()) {
        return;
    }

    const std::vector<AutomationCurveKey> keys = resolveKeys(dynamic, AutomationType::Dynamics);
    for (const AutomationCurveKey& key : keys) {
        addDynamicPoints(dynamic, tickOffset, key);
    }
}

void AutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    const DynamicType dynamicType = dynamic->dynamicType();
    const int dynamicUTick = dynamic->tick().ticks() + tickOffset;
    AutomationPoint prevPoint = m_automation->activePoint(key, dynamicUTick);

    EID eid = dynamic->eid();
    if (!eid.isValid()) {
        eid = dynamic->assignNewEID();
    }

    if (muse::contains(ORDINARY_DYNAMIC_VALUES, dynamicType)) {
        AutomationPoint point;
        point.outValue = muse::value(ORDINARY_DYNAMIC_VALUES, dynamicType);
        const bool isHairpinEnd = isEndDynamicOfHairpin(dynamic, point.outValue, prevPoint.outValue);
        point.inValue = isHairpinEnd ? point.outValue : prevPoint.outValue;
        point.itemId = eid;
        m_automation->addPoint(key, dynamicUTick, point);
        return;
    }

    if (muse::contains(SINGLE_NOTE_DYNAMIC_VALUES, dynamicType)) {
        AutomationPoint point;
        point.outValue = muse::value(SINGLE_NOTE_DYNAMIC_VALUES, dynamicType);
        const bool isHairpinEnd = isEndDynamicOfHairpin(dynamic, point.outValue, prevPoint.outValue);
        point.inValue = isHairpinEnd ? point.outValue : prevPoint.outValue;
        point.itemId = eid;
        m_automation->addPoint(key, dynamicUTick, point);

        if (const Segment* nextSeg = dynamic->segment()->next()) {
            prevPoint.inValue = point.outValue;
            m_automation->addPoint(key, nextSeg->tick().ticks() + tickOffset, prevPoint);
        }

        return;
    }

    if (muse::contains(COMPOUND_DYNAMIC_VALUES, dynamicType)) {
        const std::pair<double, double>& values = COMPOUND_DYNAMIC_VALUES.at(dynamicType);
        const int endPointTick = dynamicUTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.outValue = values.first;
        const bool isHairpinEnd = isEndDynamicOfHairpin(dynamic, startPoint.outValue, prevPoint.outValue);
        startPoint.inValue = isHairpinEnd ? startPoint.outValue : prevPoint.outValue;
        startPoint.interpolation = AutomationPoint::InterpolationType::Exponential;
        startPoint.itemId = eid;
        m_automation->addPoint(key, dynamicUTick, startPoint);

        AutomationPoint endPoint;
        endPoint.inValue = startPoint.outValue;
        endPoint.outValue = values.second;
        endPoint.itemId = eid;
        m_automation->addPoint(key, endPointTick, endPoint);

        return;
    }
}

void AutomationController::addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset)
{
    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    const auto& intervals = spannerMap.findOverlapping(repeatStartTick + 1, repeatEndTick - 1);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;
        if (!spanner->isHairpin() || !spanner->playSpanner()) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);
        const std::vector<AutomationCurveKey> keys = resolveKeys(hairpin, AutomationType::Dynamics);
        for (const AutomationCurveKey& key : keys) {
            addHairpinPoints(hairpin, tickOffset, key);
        }
    }
}

void AutomationController::addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key)
{
    int hairpinFrom = hairpin->tick().ticks() + tickOffset;
    const int hairpinTo = hairpinFrom + hairpin->ticks().ticks();

    // --- Check start tick
    {
        const Segment* startSegment = hairpin->startSegment();
        const track_idx_t trackIdx = hairpin->track();
        const Dynamic* startDynamic = startSegment
                                      ? toDynamic(startSegment->findAnnotation(ElementType::DYNAMIC, trackIdx, trackIdx))
                                      : nullptr;
        if (startDynamic && muse::contains(COMPOUND_DYNAMIC_VALUES, startDynamic->dynamicType())) {
            // The hairpin starts with a compound dynamic; we should start the hairpin after the transition is complete
            // This solution should be replaced once we have better infrastructure to see relations between Dynamics and Hairpins.
            hairpinFrom += startDynamic->velocityChangeLength().ticks();
        }
    }

    IF_ASSERT_FAILED(hairpinFrom < hairpinTo) {
        return;
    }

    EID eid = hairpin->eid();
    if (!eid.isValid()) {
        eid = hairpin->assignNewEID();
    }

    // --- Determine valueFrom
    const std::optional<double> nominalValueFrom = startHairpinValue(hairpin);
    const AutomationPoint& prevPoint = m_automation->activePoint(key, hairpinFrom);
    const AutomationCurve& curve = m_automation->curve(key);

    // If the hairpin has no specific start value, use the currently-applicable value at the start tick of the hairpin
    const double valueFrom = nominalValueFrom.value_or(prevPoint.outValue);

    if (!muse::contains(curve, hairpinFrom)) {
        AutomationPoint startPoint;
        startPoint.outValue = valueFrom;
        startPoint.inValue = prevPoint.outValue;
        startPoint.itemId = eid;
        m_automation->addPoint(key, hairpinFrom, startPoint);
    }

    // --- Determine valueTo
    const std::optional<double> nominalValueTo = endHairpinValue(hairpin);
    const bool hasNominalValueTo = nominalValueTo.has_value();
    const bool isCrescendo = hairpin->isCrescendo();

    // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs dim.)
    const bool useNominalValueTo = hasNominalValueTo
                                   && (isCrescendo ? nominalValueTo.value() > valueFrom
                                                   : nominalValueTo.value() < valueFrom);

    // --- Check end tick
    const double valueTo = useNominalValueTo
                                   ? nominalValueTo.value()
                                   : valueFrom + (isCrescendo ? DYNAMIC_STEP : -DYNAMIC_STEP);

    auto endPointToIt = curve.find(hairpinTo);
    if (endPointToIt != curve.cend()) {
        // A dynamic already sits at hairpinTo. If the hairpin arrives at a different value,
        // encode the transition via inValue != outValue on the existing point
        if (!muse::RealIsEqual(endPointToIt->second.outValue, valueTo)) {
            m_automation->setPointInValue(key, hairpinTo, valueTo);
        }
    } else {
        AutomationPoint point;
        point.inValue = valueTo;
        // If the end dynamic contradicts the hairpin direction it is still notated and must take effect;
        // encode it as an instantaneous level change at hairpinTo: the hairpin arrives at valueTo (inValue),
        // then the dynamic immediately overrides it to nominalValueTo (outValue)
        point.outValue = (hasNominalValueTo && !useNominalValueTo) ? nominalValueTo.value() : valueTo;
        point.itemId = eid;
        m_automation->addPoint(key, hairpinTo, point);
    }
}
