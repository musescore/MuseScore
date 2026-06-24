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

#include "scoreautomationcontroller.h"

#include <set>
#include <unordered_set>

#include "global/containers.h"

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

static void tryAddStaffKey(const Score* score, staff_idx_t staffIdx,
                           staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo,
                           AutomationCurveKey key, std::vector<AutomationCurveKey>& result)
{
    if (staffIdx < staffIdxFrom || staffIdx > staffIdxTo) {
        return;
    }

    const Staff* staff = score ? score->staff(staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff) {
        return;
    }

    if (!staff->isPrimaryStaff()) {
        return; // ignore linked staves
    }

    key.staffId = staff->id();
    result.push_back(key);
}

static std::vector<AutomationCurveKey> resolveKeys(const EngravingItem* item, AutomationType type,
                                                   staff_idx_t staffIdxFrom = muse::nidx,
                                                   staff_idx_t staffIdxTo = muse::nidx)
{
    const VoiceAssignment voiceAssignment = item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);
    const staff_idx_t from = hasStaffFilter ? staffIdxFrom : 0;
    const staff_idx_t to = hasStaffFilter ? staffIdxTo : muse::nidx;
    const Score* score = item->score();

    std::vector<AutomationCurveKey> result;
    AutomationCurveKey key;
    key.type = type;

    switch (voiceAssignment) {
    case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT: {
        const Part* part = item->part();
        IF_ASSERT_FAILED(part) {
            return result;
        }

        const staff_idx_t startStaffIdx = track2staff(part->startTrack());
        const staff_idx_t endStaffIdx = startStaffIdx + part->nstaves();
        result.reserve(endStaffIdx - startStaffIdx);

        for (staff_idx_t staffIdx = startStaffIdx; staffIdx < endStaffIdx; ++staffIdx) {
            tryAddStaffKey(score, staffIdx, from, to, key, result);
        }
    } break;
    case VoiceAssignment::CURRENT_VOICE_ONLY:
        key.voiceIdx = item->voice();
        // fallthrough
    case VoiceAssignment::ALL_VOICE_IN_STAFF:
        tryAddStaffKey(score, item->staffIdx(), from, to, key, result);
        break;
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

ScoreAutomationController::ScoreAutomationController()
{
    m_automation = new Automation();
}

ScoreAutomationController::~ScoreAutomationController()
{
    delete m_automation;
}

static bool isRelevantChange(const ScoreChanges& changes)
{
    static const std::unordered_set<ElementType> RELEVANT_TYPES {
        ElementType::DYNAMIC,
        ElementType::HAIRPIN,
        ElementType::HAIRPIN_SEGMENT,
    };

    for (const ElementType type : changes.changedTypes) {
        if (muse::contains(RELEVANT_TYPES, type)) {
            return true;
        }
    }

    return false;
}

void ScoreAutomationController::init(const Score* score)
{
    TRACEFUNC;

    m_automation->beginTransaction();
    update(score, 0, muse::nidx, muse::nidx);
    m_automation->commitTransaction();
}

void ScoreAutomationController::insertTime(const Score* score, const Fraction& tick, const Fraction& len)
{
    const utick_t diff = len.ticks();
    if (m_automation->isEmpty() || diff == 0) {
        return;
    }

    const utick_t utick = score->repeatList().tick2utick(tick.ticks());

    m_automation->beginTransaction();
    if (diff < 0) {
        m_automation->removeTicks(utick + diff, utick);
    } else if (diff > 0) {
        m_automation->moveTicks(utick, diff);
    }
    m_automation->commitTransaction();
}

void ScoreAutomationController::update(const Score* score, const ScoreChanges& changes)
{
    if (changes.isTextEditing || !isRelevantChange(changes)) {
        return;
    }

    const int tickFrom = std::max(changes.tickFrom, 0);

    // VoiceAssignment change shifts which staves a dynamic/hairpin covers, so the old points
    // on staves outside the new assignment must also be cleared
    const bool voiceAssignmentChanged = muse::contains(changes.changedPropertyIdSet, Pid::VOICE_ASSIGNMENT);

    m_automation->beginTransaction();
    if (voiceAssignmentChanged) {
        update(score, tickFrom, muse::nidx, muse::nidx);
    } else {
        update(score, tickFrom, changes.staffIdxFrom, changes.staffIdxTo);
    }
    m_automation->commitTransaction();
}

void ScoreAutomationController::update(const Score* score, int tickFrom, size_t staffIdxFrom, size_t staffIdxTo)
{
    TRACEFUNC;

    if (!score) {
        m_automation->clear();
        return;
    }

    const RepeatList& repeatList = score->repeatList();
    const auto repeatFromIt = std::find_if(repeatList.cbegin(), repeatList.cend(),
                                           [tickFrom](const RepeatSegment* seg) {
        return seg->endTick() > tickFrom;
    });

    if (repeatFromIt == repeatList.cend()) {
        m_automation->clear();
        return;
    }

    removeGeneratedPoints(score, *repeatFromIt, tickFrom, staffIdxFrom, staffIdxTo);

    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        const int tickOffset = seg->utick - seg->tick;
        const int measureFrom = std::max(seg->tick, tickFrom);

        for (const Measure* measure : seg->measureList()) {
            if (measure->endTick().ticks() <= measureFrom) {
                continue;
            }

            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (segment->annotations().empty() || segment->tick().ticks() < measureFrom) {
                    continue;
                }

                addSegmentPoints(segment, tickOffset, staffIdxFrom, staffIdxTo);
            }
        }

        addSpannerPoints(score, seg->tick, seg->endTick(), tickOffset, staffIdxFrom, staffIdxTo);
    }
}

void ScoreAutomationController::removeGeneratedPoints(const Score* score, const RepeatSegment* seg, int tickFrom,
                                                      size_t staffIdxFrom, size_t staffIdxTo)
{
    const int tickOffset = seg->utick - seg->tick;
    const utick_t clearFromTick = std::max(seg->utick, tickFrom + tickOffset);
    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);
    const staff_idx_t fromIdx = hasStaffFilter ? staffIdxFrom : 0;
    const staff_idx_t toIdx = hasStaffFilter ? staffIdxTo : score->nstaves() - 1;

    std::set<muse::ID> staffIds;
    for (staff_idx_t staffIdx = fromIdx; staffIdx <= toIdx; ++staffIdx) {
        if (const Staff* staff = score->staff(staffIdx)) {
            staffIds.insert(staff->id());
        }
    }

    m_automation->removePoints([clearFromTick, &staffIds](const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point) {
        return tick >= clearFromTick
               && key.type == AutomationType::Dynamics
               && point.itemId.has_value()
               && muse::contains(staffIds, key.staffId);
    });
}

void ScoreAutomationController::addSegmentPoints(const Segment* segment, int tickOffset,
                                                 size_t staffIdxFrom, size_t staffIdxTo)
{
    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);

    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation) {
            continue;
        }

        if (hasStaffFilter) {
            const staff_idx_t staffIdx = annotation->staffIdx();
            if (staffIdx < staffIdxFrom || staffIdx > staffIdxTo) {
                continue;
            }
        }

        if (annotation->isDynamic()) {
            addDynamicPoints(toDynamic(annotation), tickOffset, staffIdxFrom, staffIdxTo);
        }
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset,
                                                 size_t staffIdxFrom, size_t staffIdxTo)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const std::vector<AutomationCurveKey> keys = resolveKeys(dynamic, AutomationType::Dynamics, staffIdxFrom, staffIdxTo);
    for (const AutomationCurveKey& key : keys) {
        addDynamicPoints(dynamic, tickOffset, key);
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    const DynamicType dynamicType = dynamic->dynamicType();
    const utick_t dynamicUTick = dynamic->tick().ticks() + tickOffset;
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
        const utick_t endPointTick = dynamicUTick + dynamic->velocityChangeLength().ticks();

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

void ScoreAutomationController::addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset,
                                                 size_t staffIdxFrom, size_t staffIdxTo)
{
    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);

    const auto& intervals = spannerMap.findOverlapping(repeatStartTick + 1, repeatEndTick - 1);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;
        if (!spanner->isHairpin() || !spanner->playSpanner()) {
            continue;
        }

        if (hasStaffFilter) {
            const staff_idx_t staffIdx = spanner->staffIdx();
            if (staffIdx < staffIdxFrom || staffIdx > staffIdxTo) {
                continue;
            }
        }

        const Hairpin* hairpin = toHairpin(spanner);
        const std::vector<AutomationCurveKey> keys = resolveKeys(hairpin, AutomationType::Dynamics,
                                                                 staffIdxFrom, staffIdxTo);
        for (const AutomationCurveKey& key : keys) {
            addHairpinPoints(hairpin, tickOffset, key);
        }
    }
}

void ScoreAutomationController::addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key)
{
    utick_t hairpinFrom = hairpin->tick().ticks() + tickOffset;
    const utick_t hairpinTo = hairpinFrom + hairpin->ticks().ticks();

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
