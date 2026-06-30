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
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/hairpin.h"
#include "engraving/types/typesconv.h"

#include "engraving/automation/internal/automation.h"

#include "log.h"

using namespace mu::engraving;

// Normalized [0.0, 1.0] dynamic levels, aligned with MPE dynamic level percentages (5% steps starting at 17.5%)
static const std::unordered_map<DynamicType, double> ORDINARY_DYNAMIC_VALUES {
    { DynamicType::N,      0.000 },
    { DynamicType::PPPPPP, 0.175 },
    { DynamicType::PPPPP,  0.225 },
    { DynamicType::PPPP,   0.275 },
    { DynamicType::PPP,    0.325 },
    { DynamicType::PP,     0.375 },
    { DynamicType::P,      0.425 },
    { DynamicType::MP,     0.475 },
    { DynamicType::MF,     0.525 },
    { DynamicType::F,      0.575 },
    { DynamicType::FF,     0.625 },
    { DynamicType::FFF,    0.675 },
    { DynamicType::FFFF,   0.725 },
    { DynamicType::FFFFF,  0.775 },
    { DynamicType::FFFFFF, 0.825 },
};

static constexpr double DYNAMIC_STEP(0.05);

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

static int dynamicPriority(const EngravingItem* item)
{
    return static_cast<int>(item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>());
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
        ElementType::MEASURE_REPEAT,
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
    DynamicPriorities dynamicPriorities;
    DeferredInValuePoints deferredInValuePoints;

    std::vector<std::pair<const MeasureRepeat*, int> > measureRepeats;

    // Step 1: segment dynamics: populates dynamicPriorities, inserts points with deferred inValues
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        const int tickOffset = seg->utick - seg->tick;

        // tickFrom limits only the first affected repeat segment: later segments may replay
        // measures from before tickFrom, and all their points were cleared, so rebuild them in full
        const int measureFrom = (it == repeatFromIt) ? std::max(seg->tick, tickFrom) : seg->tick;

        for (const Measure* measure : seg->measureList()) {
            if (measure->endTick().ticks() <= measureFrom) {
                continue;
            }

            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (segment->tick().ticks() < measureFrom) {
                    continue;
                }

                collectMeasureRepeats(score, segment, measureRepeats, tickOffset, staffIdxFrom, staffIdxTo);

                if (segment->annotations().empty()) {
                    continue;
                }

                addSegmentPoints(segment, tickOffset, staffIdxFrom, staffIdxTo, dynamicPriorities, deferredInValuePoints);
            }
        }
    }

    // Step 2: spanner points: dynamicPriorities fully populated; sets inValues on hairpin-end dynamics
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        addSpannerPoints(score, seg->tick, seg->endTick(), seg->utick - seg->tick, staffIdxFrom, staffIdxTo, dynamicPriorities,
                         deferredInValuePoints);
    }

    // Step 3: copy shared dynamics points into each voice curve
    copyVoiceCurves(score, staffIdxFrom, staffIdxTo);

    // Step 4: resolve deferred inValues — shared curves and their voice curve copies
    resolveDeferredInValues(deferredInValuePoints);

    // Step 5: measure repeats: source curves fully resolved
    addMeasureRepeatPoints(measureRepeats, dynamicPriorities);
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
                                                 size_t staffIdxFrom, size_t staffIdxTo, DynamicPriorities& dynamicPriorities,
                                                 DeferredInValuePoints& deferredInValuePoints)
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
            addDynamicPoints(toDynamic(annotation), tickOffset, staffIdxFrom, staffIdxTo, dynamicPriorities, deferredInValuePoints);
        }
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset,
                                                 size_t staffIdxFrom, size_t staffIdxTo, DynamicPriorities& dynamicPriorities,
                                                 DeferredInValuePoints& deferredInValuePoints)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const std::vector<AutomationCurveKey> keys = resolveKeys(dynamic, AutomationType::Dynamics, staffIdxFrom, staffIdxTo);
    for (const AutomationCurveKey& key : keys) {
        addDynamicPoints(dynamic, tickOffset, key, dynamicPriorities, deferredInValuePoints);
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key,
                                                 DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    const int priority = dynamicPriority(dynamic);
    const DynamicType dynamicType = dynamic->dynamicType();
    const utick_t dynamicUTick = dynamic->tick().ticks() + tickOffset;

    EID eid = dynamic->eid();
    if (!eid.isValid()) {
        eid = dynamic->assignNewEID();
    }

    if (muse::contains(ORDINARY_DYNAMIC_VALUES, dynamicType)) {
        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = muse::value(ORDINARY_DYNAMIC_VALUES, dynamicType);
        point.itemId = eid;
        if (tryAddDynamicPoint(key, dynamicUTick, point, priority, dynamicPriorities)) {
            deferredInValuePoints.emplace(key, dynamicUTick);
        }
        return;
    }

    if (muse::contains(SINGLE_NOTE_DYNAMIC_VALUES, dynamicType)) {
        const AutomationPoint* prevPoint = m_automation->activePoint(key, dynamicUTick);

        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = muse::value(SINGLE_NOTE_DYNAMIC_VALUES, dynamicType);
        point.itemId = eid;
        if (tryAddDynamicPoint(key, dynamicUTick, point, priority, dynamicPriorities)) {
            deferredInValuePoints.emplace(key, dynamicUTick);
        }

        if (const Segment* nextSeg = dynamic->segment()->next()) {
            AutomationPoint nextPoint = prevPoint ? *prevPoint : AutomationPoint{};
            nextPoint.inValue = point.outValue;
            tryAddDynamicPoint(key, nextSeg->tick().ticks() + tickOffset, nextPoint, priority, dynamicPriorities);
        }

        return;
    }

    if (muse::contains(COMPOUND_DYNAMIC_VALUES, dynamicType)) {
        const std::pair<double, double>& values = COMPOUND_DYNAMIC_VALUES.at(dynamicType);
        const utick_t endPointTick = dynamicUTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.inValue = 0.0;
        startPoint.outValue = values.first;
        startPoint.itemId = eid;
        if (tryAddDynamicPoint(key, dynamicUTick, startPoint, priority, dynamicPriorities)) {
            deferredInValuePoints.emplace(key, dynamicUTick);
        }

        AutomationPoint endPoint;
        endPoint.inValue = values.second;
        endPoint.outValue = values.second;
        endPoint.itemId = eid;
        tryAddDynamicPoint(key, endPointTick, endPoint, priority, dynamicPriorities);

        return;
    }
}

void ScoreAutomationController::addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset,
                                                 size_t staffIdxFrom, size_t staffIdxTo, DynamicPriorities& dynamicPriorities,
                                                 DeferredInValuePoints& deferredInValuePoints)
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
            addHairpinPoints(hairpin, tickOffset, key, dynamicPriorities, deferredInValuePoints);
        }
    }
}

void ScoreAutomationController::addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key,
                                                 DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints)
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
    const AutomationPoint* prevPoint = m_automation->activePoint(key, hairpinFrom);
    const double prevOutValue = prevPoint ? prevPoint->outValue : 0.0;

    // If the hairpin has no specific start value, use the currently-applicable value at the start tick of the hairpin
    const double valueFrom = nominalValueFrom.value_or(prevOutValue);
    const int priority = dynamicPriority(hairpin);

    {
        AutomationPoint startPoint;
        startPoint.outValue = valueFrom;
        startPoint.inValue = prevOutValue;
        startPoint.itemId = eid;
        tryAddDynamicPoint(key, hairpinFrom, startPoint, priority, dynamicPriorities);
    }

    // --- Determine valueTo
    const std::optional<double> nominalValueTo = endHairpinValue(hairpin);
    const bool isCrescendo = hairpin->isCrescendo();

    // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs dim.)
    const bool useNominalValueTo = nominalValueTo.has_value()
                                   && (isCrescendo ? nominalValueTo.value() > valueFrom
                                       : nominalValueTo.value() < valueFrom);

    // --- Check end tick
    const double valueTo = useNominalValueTo
                           ? nominalValueTo.value()
                           : valueFrom + (isCrescendo ? DYNAMIC_STEP : -DYNAMIC_STEP);

    // Re-fetch curve in case tryAddDynamicPoint above created it
    const AutomationCurve& curve = m_automation->curve(key);
    const bool hasPointAtEnd = muse::contains(curve, hairpinTo);

    if (hasPointAtEnd) {
        // A point already exists at the end tick; encode the hairpin's arrival via inValue, but only
        // if this hairpin has at least as much priority as whoever placed that point
        bool canModify = true;
        const auto prioKeyIt = dynamicPriorities.find(key);
        if (prioKeyIt != dynamicPriorities.end()) {
            const auto tickIt = prioKeyIt->second.find(hairpinTo);
            canModify = tickIt == prioKeyIt->second.end() || priority >= tickIt->second;
        }

        if (canModify) {
            m_automation->setPointInValue(key, hairpinTo, valueTo);
            // Remove from deferred so the resolve pass doesn't overwrite the inValue we just set
            deferredInValuePoints.erase({ key, hairpinTo });
        }
        return;
    }

    if (hairpinFrom < hairpinTo) {
        AutomationPoint point;
        point.inValue = valueTo;
        point.outValue = valueTo;
        point.itemId = eid;
        tryAddDynamicPoint(key, hairpinTo, point, priority, dynamicPriorities);
    }
}

void ScoreAutomationController::copyVoiceCurves(const Score* score, size_t staffIdxFrom, size_t staffIdxTo)
{
    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);

    for (const auto& [key, curve] : m_automation->curves()) {
        if (key.type != AutomationType::Dynamics || !key.voiceIdx.has_value() || curve.empty()) {
            continue;
        }

        if (hasStaffFilter) {
            const Staff* staff = score->staffById(key.staffId);
            if (!staff || staff->idx() < staffIdxFrom || staff->idx() > staffIdxTo) {
                continue;
            }
        }

        AutomationCurveKey sharedKey = key;
        sharedKey.voiceIdx = std::nullopt;
        const AutomationCurve& sharedCurve = m_automation->curve(sharedKey);

        for (const auto& [tick, point] : sharedCurve) {
            if (!muse::contains(curve, tick)) {
                m_automation->addPoint(key, tick, point);
            }
        }
    }
}

void ScoreAutomationController::collectMeasureRepeats(const Score* score, const Segment* segment,
                                                      std::vector<std::pair<const MeasureRepeat*, int> >& result, int tickOffset,
                                                      size_t staffIdxFrom, size_t staffIdxTo) const
{
    if (!segment->isChordRestType()) {
        return;
    }

    const bool hasStaffFilter = (staffIdxFrom != muse::nidx);
    const size_t nstaves = score->nstaves();

    std::set<std::pair<const MeasureRepeat*, int> > visited;
    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        if (hasStaffFilter && (staffIdx < staffIdxFrom || staffIdx > staffIdxTo)) {
            continue;
        }

        const EngravingItem* item = segment->element(staff2track(staffIdx));
        if (!item || !item->isMeasureRepeat()) {
            continue;
        }

        const MeasureRepeat* mr = toMeasureRepeat(item);
        if (visited.insert({ mr, tickOffset }).second) {
            result.emplace_back(mr, tickOffset);
        }
    }
}

void ScoreAutomationController::addMeasureRepeatPoints(const std::vector<std::pair<const MeasureRepeat*, int> >& measureRepeats,
                                                       DynamicPriorities& dynamicPriorities)
{
    for (const auto& [mr, tickOffset] : measureRepeats) {
        const Measure* currMeasure = mr->firstMeasureOfGroup();
        if (!currMeasure) {
            continue;
        }

        const Measure* referringMeasure = mr->referringMeasure(currMeasure);
        if (!referringMeasure) {
            continue;
        }

        const Staff* staff = mr->staff();
        if (!staff) {
            continue;
        }

        const int tickShift = currMeasure->tick().ticks() - referringMeasure->tick().ticks();

        std::vector<std::pair<AutomationCurveKey, const AutomationCurve*> > staffCurves;
        for (const auto& [key, curve] : m_automation->curves()) {
            if (key.staffId == staff->id()) {
                staffCurves.emplace_back(key, &curve);
            }
        }

        for (int num = 0; num < mr->numMeasures(); ++num) {
            const utick_t srcFrom = referringMeasure->tick().ticks() + tickOffset;
            const utick_t srcTo = referringMeasure->endTick().ticks() + tickOffset;

            for (const auto& [key, curvePtr] : staffCurves) {
                const AutomationCurve& curve = *curvePtr;
                bool firstPoint = true;

                for (auto it = curve.lower_bound(srcFrom), end = curve.lower_bound(srcTo); it != end; ++it) {
                    AutomationPoint point = it->second;
                    const utick_t dstTick = it->first + tickShift;

                    if (firstPoint) {
                        // Sync the inValue of the first copied point to the level active at the destination
                        const AutomationPoint* prev = m_automation->activePoint(key, dstTick);
                        point.inValue = prev ? prev->outValue : 0.0;
                        firstPoint = false;
                    }

                    tryAddDynamicPoint(key, dstTick, point, 0, dynamicPriorities);
                }
            }

            currMeasure = currMeasure->nextMeasure();
            if (!currMeasure) {
                break;
            }

            referringMeasure = mr->referringMeasure(currMeasure);
            if (!referringMeasure) {
                break;
            }
        }
    }
}

void ScoreAutomationController::resolveDeferredInValues(DeferredInValuePoints& deferredInValuePoints)
{
    // Segment dynamics are inserted in Step 1 before hairpin endpoints exist in the curve,
    // so non-hairpin-end points carry inValue=0.0 as a placeholder. Resolve them here after
    // all points are in the curve so the predecessor lookup finds the correct value.
    // Voice curves (Step 3) copy these placeholder points, so they need the same treatment
    auto resolvePoint = [this](const AutomationCurveKey& key, utick_t tick) {
        const AutomationCurve& curve = m_automation->curve(key);
        const auto it = curve.find(tick);
        if (it == curve.end() || it == curve.begin()) {
            return;
        }
        m_automation->setPointInValue(key, tick, std::prev(it)->second.outValue);
    };

    std::map<muse::ID, std::vector<AutomationCurveKey> > voiceCurvesByStaff;
    for (const auto& [key, curve] : m_automation->curves()) {
        if (key.type == AutomationType::Dynamics && key.voiceIdx.has_value()) {
            voiceCurvesByStaff[key.staffId].push_back(key);
        }
    }

    for (const auto& [key, tick] : deferredInValuePoints) {
        resolvePoint(key, tick);
        if (!key.voiceIdx.has_value()) {
            const auto it = voiceCurvesByStaff.find(key.staffId);
            if (it != voiceCurvesByStaff.end()) {
                for (const AutomationCurveKey& voiceKey : it->second) {
                    if (muse::contains(m_automation->curve(voiceKey), tick)) {
                        resolvePoint(voiceKey, tick);
                    }
                }
            }
        }
    }

    // User-added points (no itemId) act as breakpoints: the curve holds flat before them
    // (inValue = prev.outValue) and holds flat at their level until the next point
    // (nextPoint.inValue = outValue).
    for (const auto& [key, curve] : m_automation->curves()) {
        for (auto it = curve.begin(); it != curve.end(); ++it) {
            const auto& [tick, point] = *it;
            if (point.itemId.has_value()) {
                continue;
            }
            resolvePoint(key, tick);
            const auto nextIt = std::next(it);
            if (nextIt != curve.end()) {
                m_automation->setPointInValue(key, nextIt->first, point.outValue);
            }
        }
    }
}

bool ScoreAutomationController::tryAddDynamicPoint(const AutomationCurveKey& key, utick_t tick,
                                                   const AutomationPoint& point, int priority, DynamicPriorities& dynamicPriorities)
{
    //! See: https://github.com/musescore/MuseScore/issues/23355
    auto& tickPrioMap = dynamicPriorities[key];
    const auto prioIt = tickPrioMap.find(tick);

    if (prioIt != tickPrioMap.end()) {
        if (priority <= prioIt->second) {
            return false;
        }
    } else if (muse::contains(m_automation->curve(key), tick)) {
        return false; // point from a prior update pass — don't overwrite
    }

    m_automation->addPoint(key, tick, point);
    tickPrioMap[tick] = priority;

    return true;
}
