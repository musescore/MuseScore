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
static const std::unordered_map<DynamicType, real_t> ORDINARY_DYNAMIC_VALUES {
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

static constexpr real_t DYNAMIC_STEP = real_t::make(0.05);

// Dynamics and measure repeats can only appear on these segment types
static constexpr SegmentType RELEVANT_SEGMENT_TYPES = SegmentType::ChordRest | SegmentType::TimeTick;

static const std::unordered_map<DynamicType, real_t> SINGLE_NOTE_DYNAMIC_VALUES {
    { DynamicType::SF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::SFFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::RFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::RF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
};

static const std::unordered_map<DynamicType, std::pair<real_t, real_t> > COMPOUND_DYNAMIC_VALUES {
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

static std::optional<real_t> dynamicValue(DynamicType type, bool startValue)
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

static std::optional<real_t> startHairpinValue(const Hairpin* hairpin)
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

static std::optional<real_t> endHairpinValue(const Hairpin* hairpin)
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

static const AutomationPoint* activePoint(const AutomationCurveMap& curves, const AutomationCurveKey& key, utick_t tick)
{
    static const AutomationCurve EMPTY_CURVE;

    const auto keyCurveIt = curves.find(key);
    const AutomationCurve& keyCurve = keyCurveIt != curves.end() ? keyCurveIt->second : EMPTY_CURVE;
    const auto keyIt = muse::findLessOrEqual(keyCurve, tick);

    if (key.voiceIdx.has_value()) {
        AutomationCurveKey sharedKey = key;
        sharedKey.voiceIdx = std::nullopt;
        const auto sharedCurveIt = curves.find(sharedKey);
        const AutomationCurve& sharedCurve = sharedCurveIt != curves.end() ? sharedCurveIt->second : EMPTY_CURVE;
        const auto sharedIt = muse::findLessOrEqual(sharedCurve, tick);

        if (sharedIt != sharedCurve.cend()) {
            if (keyIt == keyCurve.cend() || sharedIt->first > keyIt->first) {
                return &sharedIt->second;
            }
        }
    }

    return keyIt != keyCurve.cend() ? &keyIt->second : nullptr;
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
    update(score, 0, muse::nidx, muse::nidx);
}

void ScoreAutomationController::insertTime(const Score* score, const Fraction& tick, const Fraction& len)
{
    const utick_t diff = len.ticks();
    if (m_automation->isEmpty() || diff == 0) {
        return;
    }

    TRACEFUNC;

    const utick_t utick = score->repeatList().tick2utick(tick.ticks());

    if (diff < 0) {
        m_automation->removeTicks(utick + diff, utick);
    } else if (diff > 0) {
        m_automation->moveTicks(utick, diff);
    }
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

    if (voiceAssignmentChanged) {
        update(score, tickFrom, muse::nidx, muse::nidx);
    } else {
        update(score, tickFrom, changes.staffIdxFrom, changes.staffIdxTo);
    }
}

void ScoreAutomationController::update(const Score* score, int tickFrom, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(score) {
        return;
    }

    const RepeatList& repeatList = score->repeatList();
    const auto repeatFromIt = std::find_if(repeatList.cbegin(), repeatList.cend(),
                                           [tickFrom](const RepeatSegment* seg) {
        return seg->endTick() > tickFrom;
    });

    IF_ASSERT_FAILED(repeatFromIt != repeatList.cend()) {
        return;
    }

    const StaffRange range(score, staffIdxFrom, staffIdxTo);
    const RepeatSegment* firstSeg = *repeatFromIt;

    UpdateContext ctx;

    // The first utick the rebuild will regenerate; everything before it is kept as is
    ctx.clearFromUTick = std::max(firstSeg->utick, tickFrom + (firstSeg->utick - firstSeg->tick));

    // Copy only the curves this update can affect, dropping the generated points that the rebuild
    // below re-creates; all other curves stay untouched in m_automation
    copyCurvesForRebuild(m_automation->curves(), range, ctx.clearFromUTick, ctx.curves);

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

            for (const Segment* segment = measure->first(RELEVANT_SEGMENT_TYPES); segment;
                 segment = segment->next(RELEVANT_SEGMENT_TYPES)) {
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

    // Step 2: spanner points: ctx.dynamicPriorities fully populated; sets inValues on hairpin-end dynamics
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        addSpannerPoints(score, seg->tick, seg->endTick(), seg->utick - seg->tick, range, ctx);
    }

    // Step 3: fill each voice curve with any points from the base (all-voice) curve it doesn't already have
    fillVoiceCurvesFromBase(ctx);

    // Step 4: resolve deferred inValues — shared curves and their voice curve copies
    resolveDeferredInValues(ctx);

    // Step 5: measure repeats: source curves fully resolved
    addMeasureRepeatPoints(ctx);

    // Step 6: the new curves are fully built; merge them back, replacing only the affected ones
    m_automation->replaceCurves(std::move(ctx.curves));
}

void ScoreAutomationController::copyCurvesForRebuild(const AutomationCurveMap& curves, const StaffRange& range, utick_t clearFromUTick,
                                                     AutomationCurveMap& destCurves)
{
    for (const auto& [key, curve] : curves) {
        if (key.type != AutomationType::Dynamics || !range.contains(key.staffId)) {
            continue;
        }

        AutomationCurve& curveCopy = destCurves.emplace_hint(destCurves.end(), key, AutomationCurve())->second;

        for (const auto& [tick, point] : curve) {
            if (tick < clearFromUTick || !point.generated) {
                curveCopy.emplace_hint(curveCopy.end(), tick, point);
            }
        }
    }
}

void ScoreAutomationController::addSegmentPoints(const Segment* segment, int tickOffset, const StaffRange& range, UpdateContext& ctx)
{
    TRACEFUNC;

    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->isDynamic()) {
            continue;
        }

        if (!range.contains(annotation->staffIdx())) {
            continue;
        }

        addDynamicPoints(toDynamic(annotation), tickOffset, range, ctx);
    }
}

void ScoreAutomationController::addDynamicPoints(const Dynamic* dynamic, int tickOffset, const StaffRange& range, UpdateContext& ctx)
{
    TRACEFUNC;

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
        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = it->second;
        point.itemId = eid;
        point.generated = true;
        addDeferredPoint(key, dynamicUTick, point, priority, ctx);
        return;
    }

    if (auto it = SINGLE_NOTE_DYNAMIC_VALUES.find(dynamicType); it != SINGLE_NOTE_DYNAMIC_VALUES.end()) {
        const Segment* nextSeg = dynamic->segment()->next();
        const AutomationPoint* prevPoint = nextSeg ? activePoint(ctx.curves, key, dynamicUTick) : nullptr;

        AutomationPoint point;
        point.inValue = 0.0;
        point.outValue = it->second;
        point.itemId = eid;
        point.generated = true;
        addDeferredPoint(key, dynamicUTick, point, priority, ctx);

        if (nextSeg) {
            AutomationPoint nextPoint = prevPoint ? *prevPoint : AutomationPoint{};
            nextPoint.inValue = point.outValue;
            nextPoint.generated = true;
            tryAddDynamicPoint(key, nextSeg->tick().ticks() + tickOffset, nextPoint, priority, ctx);
        }

        return;
    }

    if (auto it = COMPOUND_DYNAMIC_VALUES.find(dynamicType); it != COMPOUND_DYNAMIC_VALUES.end()) {
        const std::pair<real_t, real_t>& values = it->second;
        const utick_t endPointTick = dynamicUTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.inValue = 0.0;
        startPoint.outValue = values.first;
        startPoint.itemId = eid;
        startPoint.generated = true;
        addDeferredPoint(key, dynamicUTick, startPoint, priority, ctx);

        AutomationPoint endPoint;
        endPoint.inValue = values.second;
        endPoint.outValue = values.second;
        endPoint.itemId = eid;
        endPoint.generated = true;
        tryAddDynamicPoint(key, endPointTick, endPoint, priority, ctx);

        return;
    }
}

void ScoreAutomationController::addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick,
                                                 int tickOffset, const StaffRange& range, UpdateContext& ctx)
{
    TRACEFUNC;

    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    const int overlapStart = std::max(repeatStartTick + 1, ctx.clearFromUTick - tickOffset);
    const int overlapStop = repeatEndTick - 1;
    if (overlapStart > overlapStop) {
        return;
    }

    const auto& intervals = spannerMap.findOverlapping(overlapStart, overlapStop);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;
        if (!spanner->isHairpin() || !spanner->playSpanner()) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);
        const std::vector<AutomationCurveKey> keys = resolveKeys(hairpin, AutomationType::Dynamics, range);
        if (!keys.empty()) {
            addHairpinPoints(hairpin, tickOffset, keys, ctx);
        }
    }
}

void ScoreAutomationController::addHairpinPoints(const Hairpin* hairpin, int tickOffset, const std::vector<AutomationCurveKey>& keys,
                                                 UpdateContext& ctx)
{
    HairpinInfo info;
    info.from = hairpin->tick().ticks() + tickOffset;
    info.to = info.from + hairpin->ticks().ticks();

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
            info.from += startDynamic->velocityChangeLength().ticks();
        }
    }

    IF_ASSERT_FAILED(info.from < info.to) {
        return;
    }

    info.eid = hairpin->eid();
    if (!info.eid.isValid()) {
        info.eid = hairpin->assignNewEID();
    }

    info.priority = dynamicPriority(hairpin);
    info.isCrescendo = hairpin->isCrescendo();
    info.nominalValueFrom = startHairpinValue(hairpin);
    info.nominalValueTo = endHairpinValue(hairpin);

    for (const AutomationCurveKey& key : keys) {
        addHairpinPoints(info, key, ctx);
    }
}

void ScoreAutomationController::addHairpinPoints(const HairpinInfo& info, const AutomationCurveKey& key, UpdateContext& ctx)
{
    // --- Determine valueFrom
    const AutomationPoint* prevPoint = activePoint(ctx.curves, key, info.from);
    const real_t prevOutValue = prevPoint ? prevPoint->outValue : real_t(0.0);

    // If the hairpin has no specific start value, use the currently-applicable value at the start tick of the hairpin
    const real_t valueFrom = info.nominalValueFrom.value_or(prevOutValue);

    {
        AutomationPoint startPoint;
        startPoint.outValue = valueFrom;
        startPoint.inValue = prevOutValue;
        startPoint.itemId = info.eid;
        startPoint.generated = true;
        tryAddDynamicPoint(key, info.from, startPoint, info.priority, ctx);
    }

    // --- Determine valueTo
    // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs dim.)
    const bool useNominalValueTo = info.nominalValueTo.has_value()
                                   && (info.isCrescendo ? info.nominalValueTo.value() > valueFrom
                                       : info.nominalValueTo.value() < valueFrom);

    // --- Check end tick
    const real_t valueTo = useNominalValueTo
                           ? info.nominalValueTo.value()
                           : valueFrom + (info.isCrescendo ? DYNAMIC_STEP : -DYNAMIC_STEP);

    // Re-fetch curve in case tryAddDynamicPoint above created it
    const auto curveIt = ctx.curves.find(key);
    AutomationCurve::iterator endPointIt;
    bool hasPointAtEnd = false;
    if (curveIt != ctx.curves.end()) {
        endPointIt = curveIt->second.find(info.to);
        hasPointAtEnd = endPointIt != curveIt->second.end();
    }

    if (hasPointAtEnd) {
        // A point already exists at the end tick; encode the hairpin's arrival via inValue, but only
        // if this hairpin has at least as much priority as whoever placed that point
        bool canModify = true;
        const auto prioKeyIt = ctx.dynamicPriorities.find(key);
        if (prioKeyIt != ctx.dynamicPriorities.end()) {
            const auto tickIt = prioKeyIt->second.find(info.to);
            canModify = tickIt == prioKeyIt->second.end() || info.priority >= tickIt->second;
        }

        if (canModify) {
            endPointIt->second.inValue = valueTo;
            // Remove from deferred so the resolve pass doesn't overwrite the inValue we just set
            removeDeferredInValue(ctx, key, info.to);
        }
        return;
    }

    if (info.from < info.to) {
        AutomationPoint point;
        point.inValue = valueTo;
        point.outValue = valueTo;
        point.itemId = info.eid;
        point.generated = true;
        tryAddDynamicPoint(key, info.to, point, info.priority, ctx);
    }
}

void ScoreAutomationController::fillVoiceCurvesFromBase(UpdateContext& ctx)
{
    TRACEFUNC;

    for (auto& [key, curve] : ctx.curves) {
        if (!key.voiceIdx.has_value()) {
            continue;
        }

        AutomationCurveKey baseKey = key;
        baseKey.voiceIdx = std::nullopt;

        const auto baseIt = ctx.curves.find(baseKey);
        if (baseIt == ctx.curves.end()) {
            continue;
        }

        const auto baseDeferredIt = ctx.deferredInValuePoints.find(baseKey);
        const std::set<utick_t>* baseDeferredTicks
            = baseDeferredIt != ctx.deferredInValuePoints.end() ? &baseDeferredIt->second : nullptr;

        std::set<utick_t>* voiceDeferredTicks = nullptr;

        for (const auto& [tick, point] : baseIt->second) {
            const bool inserted = curve.try_emplace(tick, point).second;
            if (!inserted || !baseDeferredTicks || !muse::contains(*baseDeferredTicks, tick)) {
                continue;
            }

            if (!voiceDeferredTicks) {
                voiceDeferredTicks = &ctx.deferredInValuePoints[key];
            }
            voiceDeferredTicks->insert(tick);
        }
    }
}

void ScoreAutomationController::collectMeasureRepeats(const Segment* segment, int tickOffset, const StaffRange& range,
                                                      MeasureRepeats& result)
{
    TRACEFUNC;

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
    TRACEFUNC;

    if (ctx.measureRepeats.empty()) {
        return;
    }

    std::map<muse::ID, std::vector<AutomationCurve*> > curvesByStaff;
    for (auto& [key, curve] : ctx.curves) {
        curvesByStaff[key.staffId].push_back(&curve);
    }

    for (const auto& [mr, tickOffset] : ctx.measureRepeats) {
        const Staff* staff = mr->staff();
        if (!staff) {
            continue;
        }

        const auto staffCurvesIt = curvesByStaff.find(staff->id());
        if (staffCurvesIt == curvesByStaff.end()) {
            continue;
        }

        const auto& staffCurves = staffCurvesIt->second;
        const Measure* currMeasure = mr->firstMeasureOfGroup();

        for (int num = 0; currMeasure && num < mr->numMeasures(); ++num, currMeasure = currMeasure->nextMeasure()) {
            const Measure* referringMeasure = mr->referringMeasure(currMeasure);
            IF_ASSERT_FAILED(referringMeasure && referringMeasure != currMeasure) {
                continue;
            }

            const int tickShift = currMeasure->tick().ticks() - referringMeasure->tick().ticks();
            const utick_t srcFrom = referringMeasure->tick().ticks() + tickOffset;
            const utick_t srcTo = referringMeasure->endTick().ticks() + tickOffset;

            for (AutomationCurve* curve : staffCurves) {
                AutomationCurve shifted;
                for (auto it = curve->lower_bound(srcFrom), end = curve->lower_bound(srcTo); it != end; ++it) {
                    shifted.emplace_hint(shifted.end(), it->first + tickShift, it->second);
                }

                if (!shifted.empty()) {
                    // Sync the inValue of the first copied point to the level active at the destination.
                    // Voice curves already have the shared curve's points merged in by this point
                    // (Step 3), so a plain lookup on this curve is enough
                    const auto prevIt = muse::findLessOrEqual(*curve, shifted.begin()->first);
                    shifted.begin()->second.inValue = prevIt != curve->end() ? prevIt->second.outValue : real_t(0.0);
                }

                curve->merge(std::move(shifted));
            }
        }
    }
}

void ScoreAutomationController::resolveDeferredInValues(UpdateContext& ctx)
{
    TRACEFUNC;

    for (const auto& [key, ticks] : ctx.deferredInValuePoints) {
        const auto curveIt = ctx.curves.find(key);
        if (curveIt == ctx.curves.end()) {
            continue;
        }

        AutomationCurve& curve = curveIt->second;
        for (utick_t tick : ticks) {
            const auto it = curve.find(tick);
            if (it != curve.end() && it != curve.begin()) {
                it->second.inValue = std::prev(it)->second.outValue;
            }
        }
    }

    for (auto& [key, curve] : ctx.curves) {
        for (auto it = curve.begin(); it != curve.end(); ++it) {
            if (it->second.itemId.has_value()) {
                continue;
            }

            if (it != curve.begin()) {
                it->second.inValue = std::prev(it)->second.outValue;
            }

            if (auto nextIt = std::next(it); nextIt != curve.end()) {
                nextIt->second.inValue = it->second.outValue;
            }
        }
    }
}

void ScoreAutomationController::removeDeferredInValue(UpdateContext& ctx, const AutomationCurveKey& key, utick_t tick)
{
    if (const auto it = ctx.deferredInValuePoints.find(key); it != ctx.deferredInValuePoints.end()) {
        it->second.erase(tick);
    }
}

bool ScoreAutomationController::tryAddDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point,
                                                   int priority, UpdateContext& ctx)
{
    //! See: https://github.com/musescore/MuseScore/issues/23355
    AutomationCurve& curve = ctx.curves[key];
    std::map<utick_t, int>& tickPrioMap = ctx.dynamicPriorities[key];

    const auto prioIt = tickPrioMap.lower_bound(tick);
    const bool hasPrio = prioIt != tickPrioMap.end() && prioIt->first == tick;

    if (hasPrio) {
        if (priority <= prioIt->second) {
            return false;
        }

        curve[tick] = point;
        prioIt->second = priority;
        return true;
    }

    const auto curveIt = curve.lower_bound(tick);
    if (curveIt != curve.end() && curveIt->first == tick) {
        return false; // point from a prior update pass — don't overwrite
    }

    curve.emplace_hint(curveIt, tick, point);
    tickPrioMap.emplace_hint(prioIt, tick, priority);

    return true;
}

void ScoreAutomationController::addDeferredPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point,
                                                 int priority, UpdateContext& ctx)
{
    // If a point with the same priority already exists at this tick, merge them:
    // keep the existing arrival value (inValue) and use this point's departure value (outValue)
    const auto prioKeyIt = ctx.dynamicPriorities.find(key);
    if (prioKeyIt != ctx.dynamicPriorities.end()) {
        const auto prioIt = prioKeyIt->second.find(tick);
        if (prioIt != prioKeyIt->second.end() && prioIt->second == priority) {
            AutomationPoint& existing = ctx.curves[key][tick];
            const real_t arrivalValue = existing.outValue;
            existing = point;
            existing.inValue = arrivalValue;
            removeDeferredInValue(ctx, key, tick);
            return;
        }
    }

    if (tryAddDynamicPoint(key, tick, point, priority, ctx)) {
        ctx.deferredInValuePoints[key].insert(tick);
    }
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

        const TrackRange trackRange = part->trackRange();
        const staff_idx_t startStaffIdx = track2staff(trackRange.startTrack);
        const staff_idx_t endStaffIdx = track2staff(trackRange.endTrack);
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
