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

ScoreAutomationController::StaffRange::StaffRange(const Score* score, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo)
{
    const staff_idx_t lastStaffIdx = score->nstaves() - 1;

    if (staffIdxFrom == muse::nidx) {
        from = 0;
        to = lastStaffIdx;
    } else {
        from = staffIdxFrom;
        to = std::min(staffIdxTo, lastStaffIdx);
    }

    isFull = (from == 0 && to == lastStaffIdx);

    if (!isFull) {
        for (staff_idx_t staffIdx = from; staffIdx <= to; ++staffIdx) {
            if (const Staff* staff = score->staff(staffIdx)) {
                staffIds.insert(staff->id());
            }
        }
    }
}

bool ScoreAutomationController::StaffRange::contains(staff_idx_t staffIdx) const
{
    return isFull || (staffIdx >= from && staffIdx <= to);
}

bool ScoreAutomationController::StaffRange::contains(const muse::ID& staffId) const
{
    return isFull || muse::contains(staffIds, staffId);
}

void ScoreAutomationController::tryAddStaffKey(const Score* score, staff_idx_t staffIdx, const StaffRange& range,
                                               AutomationCurveKey key, std::vector<AutomationCurveKey>& result)
{
    if (!range.contains(staffIdx)) {
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

std::vector<AutomationCurveKey> ScoreAutomationController::resolveKeys(const EngravingItem* item, AutomationType type,
                                                                       const StaffRange& range)
{
    const VoiceAssignment voiceAssignment = item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
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
            tryAddStaffKey(score, staffIdx, range, key, result);
        }
    } break;
    case VoiceAssignment::CURRENT_VOICE_ONLY:
        key.voiceIdx = item->voice();
    // fallthrough
    case VoiceAssignment::ALL_VOICE_IN_STAFF:
        tryAddStaffKey(score, item->staffIdx(), range, key, result);
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

    // VoiceAssignment change shifts which staves a dynamic/hairpin covers,
    // so the old points on staves outside the new assignment must also be cleared
    const bool voiceAssignmentChanged = muse::contains(changes.changedPropertyIdSet, Pid::VOICE_ASSIGNMENT);

    m_automation->beginTransaction();
    if (voiceAssignmentChanged) {
        update(score, tickFrom, muse::nidx, muse::nidx);
    } else {
        update(score, tickFrom, changes.staffIdxFrom, changes.staffIdxTo);
    }
    m_automation->commitTransaction();
}

void ScoreAutomationController::update(const Score* score, int tickFrom, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo)
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

    const StaffRange range(score, staffIdxFrom, staffIdxTo);
    UpdateContext ctx;

    removeGeneratedPoints(*repeatFromIt, tickFrom, range);

    // Step 1: segment dynamics: populates ctx.dynamicPriorities, inserts points with deferred inValues
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

                collectMeasureRepeats(segment, tickOffset, range, ctx.measureRepeats);

                if (segment->annotations().empty()) {
                    continue;
                }

                addSegmentPoints(segment, tickOffset, range, ctx);
            }
        }
    }

    // Step 1.5: anchored dynamics — processed after all regular dynamics so that a regular dynamic
    // at the same tick (possibly in a different segment) has already claimed outValue
    for (const auto& [dyn, tickOffset] : ctx.pendingAnchorDynamics) {
        addDynamicPoints(dyn, tickOffset, range, ctx);
    }

    // Step 2: spanner points: ctx.dynamicPriorities fully populated; sets inValues on hairpin-end dynamics
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        addSpannerPoints(score, seg->tick, seg->endTick(), seg->utick - seg->tick, range, ctx);
    }

    // Step 3: fill each voice curve with any points from the base (all-voice) curve it doesn't already have
    copyVoiceCurves(range);

    // Step 4: resolve deferred inValues — shared curves and their voice curve copies
    resolveDeferredInValues(ctx);

    // Step 5: measure repeats: source curves fully resolved
    addMeasureRepeatPoints(ctx);
}

void ScoreAutomationController::removeGeneratedPoints(const RepeatSegment* seg, int tickFrom, const StaffRange& range)
{
    const int tickOffset = seg->utick - seg->tick;
    const utick_t clearFromTick = std::max(seg->utick, tickFrom + tickOffset);

    m_automation->removePoints([clearFromTick, &range](const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point) {
        return tick >= clearFromTick
               && key.type == AutomationType::Dynamics
               && point.itemId.has_value()
               && range.contains(key.staffId);
    });
}

void ScoreAutomationController::addSegmentPoints(const Segment* segment, int tickOffset, const StaffRange& range, UpdateContext& ctx)
{
    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->isDynamic()) {
            continue;
        }

        if (!range.contains(annotation->staffIdx())) {
            continue;
        }

        const Dynamic* dyn = toDynamic(annotation);
        if (dyn->anchorToEndOfPrevious()) {
            ctx.pendingAnchorDynamics.emplace_back(dyn, tickOffset);
        } else {
            addDynamicPoints(dyn, tickOffset, range, ctx);
        }
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const StaffRange& range, UpdateContext& ctx)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const std::vector<AutomationCurveKey> keys = resolveKeys(dynamic, AutomationType::Dynamics, range);
    for (const AutomationCurveKey& key : keys) {
        addDynamicPoints(dynamic, tickOffset, key, ctx);
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key,
                                                 UpdateContext& ctx)
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

    if (auto it = ORDINARY_DYNAMIC_VALUES.find(dynamicType); it != ORDINARY_DYNAMIC_VALUES.end()) {
        const double value = it->second;
        // Called after all non-anchored dynamics in this segment, so a regular dynamic at the
        // same tick has already claimed outValue. Contribute only inValue; otherwise add normally.
        if (dynamic->anchorToEndOfPrevious() && muse::contains(m_automation->curve(key), dynamicUTick)) {
            m_automation->setPointInValue(key, dynamicUTick, value);
            ctx.deferredInValuePoints.erase({ key, dynamicUTick });
            return;
        }
        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = value;
        point.itemId = eid;
        addDeferredPoint(key, dynamicUTick, point, priority, ctx);
        return;
    }

    if (auto it = SINGLE_NOTE_DYNAMIC_VALUES.find(dynamicType); it != SINGLE_NOTE_DYNAMIC_VALUES.end()) {
        const AutomationPoint* prevPoint = m_automation->activePoint(key, dynamicUTick);

        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = it->second;
        point.itemId = eid;
        addDeferredPoint(key, dynamicUTick, point, priority, ctx);

        if (const Segment* nextSeg = dynamic->segment()->next()) {
            AutomationPoint nextPoint = prevPoint ? *prevPoint : AutomationPoint{};
            nextPoint.inValue = point.outValue;
            tryAddDynamicPoint(key, nextSeg->tick().ticks() + tickOffset, nextPoint, priority, ctx);
        }

        return;
    }

    if (auto it = COMPOUND_DYNAMIC_VALUES.find(dynamicType); it != COMPOUND_DYNAMIC_VALUES.end()) {
        const std::pair<double, double>& values = it->second;
        const utick_t endPointTick = dynamicUTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.inValue = 0.0;
        startPoint.outValue = values.first;
        startPoint.itemId = eid;
        addDeferredPoint(key, dynamicUTick, startPoint, priority, ctx);

        AutomationPoint endPoint;
        endPoint.inValue = values.second;
        endPoint.outValue = values.second;
        endPoint.itemId = eid;
        tryAddDynamicPoint(key, endPointTick, endPoint, priority, ctx);

        return;
    }
}

void ScoreAutomationController::addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick,
                                                 int tickOffset, const StaffRange& range, UpdateContext& ctx)
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

        if (!range.contains(spanner->staffIdx())) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);
        const std::vector<AutomationCurveKey> keys = resolveKeys(hairpin, AutomationType::Dynamics, range);
        for (const AutomationCurveKey& key : keys) {
            addHairpinPoints(hairpin, tickOffset, key, ctx);
        }
    }
}

void ScoreAutomationController::addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key,
                                                 UpdateContext& ctx)
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
        tryAddDynamicPoint(key, hairpinFrom, startPoint, priority, ctx);
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
        const auto prioKeyIt = ctx.dynamicPriorities.find(key);
        if (prioKeyIt != ctx.dynamicPriorities.end()) {
            const auto tickIt = prioKeyIt->second.find(hairpinTo);
            canModify = tickIt == prioKeyIt->second.end() || priority >= tickIt->second;
        }

        if (canModify) {
            m_automation->setPointInValue(key, hairpinTo, valueTo);
            // Remove from deferred so the resolve pass doesn't overwrite the inValue we just set
            ctx.deferredInValuePoints.erase({ key, hairpinTo });
        }
        return;
    }

    if (hairpinFrom < hairpinTo) {
        AutomationPoint point;
        point.inValue = valueTo;
        point.outValue = valueTo;
        point.itemId = eid;
        tryAddDynamicPoint(key, hairpinTo, point, priority, ctx);
    }
}

void ScoreAutomationController::copyVoiceCurves(const StaffRange& range)
{
    for (const auto& [key, curve] : m_automation->curves()) {
        if (key.type != AutomationType::Dynamics || !key.voiceIdx.has_value()) {
            continue;
        }

        if (!range.contains(key.staffId)) {
            continue;
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

void ScoreAutomationController::collectMeasureRepeats(const Segment* segment, int tickOffset, const StaffRange& range,
                                                      std::vector<std::pair<const MeasureRepeat*, int> >& result) const
{
    if (!segment->isChordRestType()) {
        return;
    }

    for (staff_idx_t staffIdx = range.from; staffIdx <= range.to; ++staffIdx) {
        const EngravingItem* item = segment->element(staff2track(staffIdx));
        if (!item || !item->isMeasureRepeat()) {
            continue;
        }

        result.emplace_back(toMeasureRepeat(item), tickOffset);
    }
}

void ScoreAutomationController::addMeasureRepeatPoints(UpdateContext& ctx)
{
    if (ctx.measureRepeats.empty()) {
        return;
    }

    std::map<muse::ID, std::vector<std::pair<AutomationCurveKey, const AutomationCurve*> > > curvesByStaff;
    for (const auto& [key, curve] : m_automation->curves()) {
        curvesByStaff[key.staffId].emplace_back(key, &curve);
    }

    for (const auto& [mr, tickOffset] : ctx.measureRepeats) {
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

        const auto staffCurvesIt = curvesByStaff.find(staff->id());
        if (staffCurvesIt == curvesByStaff.end()) {
            continue;
        }

        const int tickShift = currMeasure->tick().ticks() - referringMeasure->tick().ticks();
        const auto& staffCurves = staffCurvesIt->second;

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

                    tryAddDynamicPoint(key, dstTick, point, 0, ctx);
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

void ScoreAutomationController::resolveDeferredInValues(UpdateContext& ctx)
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

    for (const auto& [key, tick] : ctx.deferredInValuePoints) {
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

void ScoreAutomationController::addDeferredPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point,
                                                 int priority, UpdateContext& ctx)
{
    if (tryAddDynamicPoint(key, tick, point, priority, ctx)) {
        ctx.deferredInValuePoints.emplace(key, tick);
    }
}

bool ScoreAutomationController::tryAddDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point,
                                                   int priority, UpdateContext& ctx)
{
    //! See: https://github.com/musescore/MuseScore/issues/23355
    auto& tickPrioMap = ctx.dynamicPriorities[key];
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
