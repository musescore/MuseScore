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

#include "engraving/automation/automationdata.h"
#include "engraving/editing/editautomationpoints.h"

#include "log.h"

using namespace mu::engraving;

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
    static const AutomationCurve NO_CURVE;

    const auto keyCurveIt = curves.find(key);
    const AutomationCurve& keyCurve = keyCurveIt != curves.end() ? keyCurveIt->second : NO_CURVE;
    const auto keyIt = muse::findLessOrEqual(keyCurve, tick);

    if (key.voiceIdx.has_value()) {
        AutomationCurveKey sharedKey = key;
        sharedKey.voiceIdx = std::nullopt;
        const auto sharedCurveIt = curves.find(sharedKey);
        const AutomationCurve& sharedCurve = sharedCurveIt != curves.end() ? sharedCurveIt->second : NO_CURVE;
        const auto sharedIt = muse::findLessOrEqual(sharedCurve, tick);

        if (sharedIt != sharedCurve.cend()) {
            if (keyIt == keyCurve.cend() || sharedIt->first > keyIt->first) {
                return &sharedIt->second;
            }
        }
    }

    return keyIt != keyCurve.cend() ? &keyIt->second : nullptr;
}

void ScoreAutomationController::setAutomationData(AutomationDataPtr data)
{
    m_automationData = std::move(data);
}

void ScoreAutomationController::editPoints(const AutomationCurveKey& key, AutomationPointEdits& edits)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_score && m_automationData) {
        return;
    }

    if (edits.empty()) {
        return;
    }

    //! NOTE: appends the corresponding mirrored edit(s) - one per other repeat pass - directly to edits
    mirrorEditsToRepeats(key, edits);

    m_score->undo(new EditAutomationPoints(m_score, m_automationData, key, edits));
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

void ScoreAutomationController::init(Score* score)
{
    m_score = score;
    update(0, muse::nidx, muse::nidx);
}

void ScoreAutomationController::insertTime(const Fraction& tick, const Fraction& len)
{
    TRACEFUNC;

    const utick_t diff = len.ticks();
    if (!m_score || !m_automationData || m_automationData->isEmpty() || diff == 0) {
        return;
    }

    const utick_t utick = m_score->repeatList().tick2utick(tick.ticks());
    AutomationCurveMap curves = m_automationData->curves();

    if (diff < 0) {
        removeTicks(utick + diff, utick, curves);
    } else if (diff > 0) {
        moveTicks(utick, diff, curves);
    }

    m_automationData->setCurves(curves);
}

void ScoreAutomationController::update(const ScoreChanges& changes)
{
    if (changes.isTextEditing || !isRelevantChange(changes)) {
        return;
    }

    const int tickFrom = std::max(changes.tickFrom, 0);

    // VoiceAssignment change shifts which staves a dynamic/hairpin covers,
    // so the old points on staves outside the new assignment must also be cleared
    const bool voiceAssignmentChanged = muse::contains(changes.changedPropertyIdSet, Pid::VOICE_ASSIGNMENT);

    if (voiceAssignmentChanged) {
        update(tickFrom, muse::nidx, muse::nidx);
    } else {
        update(tickFrom, changes.staffIdxFrom, changes.staffIdxTo);
    }
}

void ScoreAutomationController::update(int tickFrom, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo)
{
    TRACEFUNC;

    if (!m_score) {
        return;
    }

    const RepeatList& repeatList = m_score->repeatList();
    if (repeatList.empty()) {
        return;
    }

    const auto repeatFromIt = std::find_if(repeatList.cbegin(), repeatList.cend(),
                                           [tickFrom](const RepeatSegment* seg) {
        return seg->endTick() > tickFrom;
    });

    IF_ASSERT_FAILED(repeatFromIt != repeatList.cend()) {
        return;
    }

    if (!m_automationData) {
        m_automationData = std::make_shared<AutomationData>();
    }

    const StaffRange range(m_score, staffIdxFrom, staffIdxTo);
    const RepeatSegment* firstSeg = *repeatFromIt;

    UpdateContext ctx;

    // The first utick the rebuild will regenerate; everything before it is kept as is
    ctx.clearFromUTick = std::max(firstSeg->utick, tickFrom + (firstSeg->utick - firstSeg->tick));

    // Copy only the curves this update can affect, dropping the generated points that the rebuild
    // below re-creates; all other curves stay untouched in m_automationData
    copyCurvesForRebuild(m_automationData->curves(), range, ctx.clearFromUTick, ctx.curves);

    // Step 1: segment dynamics: populates ctx.dynamicPriorities
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
        addSpannerPoints(m_score, seg->tick, seg->endTick(), seg->utick - seg->tick, range, ctx);
    }

    // Step 3: fill each voice curve with any points from the base (all-voice) curve it doesn't already have
    fillVoiceCurvesFromBase(ctx);

    // Step 4: measure repeats
    addMeasureRepeatPoints(ctx);

    // Step 5: the new curves are fully built; merge them back, replacing only the affected ones
    m_automationData->replaceCurves(ctx.curves);
}

//! NOTE: moves all points with tick >= tickFrom by diff ticks
void ScoreAutomationController::moveTicks(utick_t tickFrom, utick_t diff, AutomationCurveMap& curves)
{
    for (auto& entry : curves) {
        AutomationCurve& curve = entry.second;

        const auto startIt = curve.lower_bound(tickFrom);
        if (startIt == curve.end()) {
            continue;
        }

        std::vector<std::pair<utick_t, AutomationPoint> > toMove;
        for (auto it = startIt; it != curve.end(); ++it) {
            toMove.emplace_back(it->first + diff, it->second);
        }

        curve.erase(startIt, curve.end());
        for (auto& pair : toMove) {
            curve.insert(curve.end(), std::move(pair));
        }
    }
}

//! NOTE: removes points in [tickFrom, tickTo], shifts later points back to close the gap
void ScoreAutomationController::removeTicks(utick_t tickFrom, utick_t tickTo, AutomationCurveMap& curves)
{
    IF_ASSERT_FAILED(tickFrom <= tickTo) {
        return;
    }

    const utick_t diff = tickFrom - tickTo;

    for (auto& entry : curves) {
        AutomationCurve& curve = entry.second;

        const auto eraseFromIt = curve.lower_bound(tickFrom);
        if (eraseFromIt == curve.end()) {
            continue;
        }

        curve.erase(eraseFromIt, curve.upper_bound(tickTo));

        const auto startIt = curve.lower_bound(tickTo);
        std::vector<std::pair<utick_t, AutomationPoint> > toMove;
        for (auto it = startIt; it != curve.end(); ++it) {
            toMove.emplace_back(it->first + diff, it->second);
        }

        curve.erase(startIt, curve.end());
        for (auto& pair : toMove) {
            curve.insert(curve.end(), std::move(pair));
        }
    }

    for (auto it = curves.begin(); it != curves.end();) {
        it = it->second.empty() ? curves.erase(it) : std::next(it);
    }
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

    const DynamicType dynamicType = dynamic->dynamicType();
    const utick_t tick = dynamic->tick().ticks() + tickOffset;

    DynamicInfo info;
    info.tick = tick;

    if (auto ordinaryIt = ORDINARY_DYNAMIC_VALUES.find(dynamicType); ordinaryIt != ORDINARY_DYNAMIC_VALUES.end()) {
        info.kind = DynamicInfo::Ordinary { ordinaryIt->second };
    } else if (auto singleNoteIt = SINGLE_NOTE_DYNAMIC_VALUES.find(dynamicType); singleNoteIt != SINGLE_NOTE_DYNAMIC_VALUES.end()) {
        DynamicInfo::SingleNote singleNote { singleNoteIt->second };
        if (const Segment* nextSeg = dynamic->segment()->next()) {
            singleNote.nextTick = nextSeg->tick().ticks() + tickOffset;
        }
        info.kind = singleNote;
    } else if (auto compoundIt = COMPOUND_DYNAMIC_VALUES.find(dynamicType); compoundIt != COMPOUND_DYNAMIC_VALUES.end()) {
        info.kind = DynamicInfo::Compound { compoundIt->second.first, compoundIt->second.second,
                                            tick + dynamic->velocityChangeLength().ticks() };
    } else {
        NOT_SUPPORTED;
        return;
    }

    const std::vector<AutomationCurveKey> keys = resolveKeys(dynamic, AutomationType::Dynamics, range);
    if (keys.empty()) {
        return;
    }

    info.priority = dynamicPriority(dynamic);

    info.eid = dynamic->eid();
    if (!info.eid.isValid()) {
        info.eid = dynamic->assignNewEID();
    }

    for (const AutomationCurveKey& key : keys) {
        addDynamicPoints(info, key, ctx);
    }
}

void ScoreAutomationController::addDynamicPoints(const DynamicInfo& info, const AutomationCurveKey& key, UpdateContext& ctx)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    if (const auto* ordinary = std::get_if<DynamicInfo::Ordinary>(&info.kind)) {
        AutomationPoint point;
        point.inValue = AutomationPoint::FromPrevious {};
        point.outValue = ordinary->value;
        point.itemId = info.eid;
        point.generated = true;
        addDynamicPoint(key, info.tick, point, info.priority, ctx);
        return;
    }

    if (const auto* singleNote = std::get_if<DynamicInfo::SingleNote>(&info.kind)) {
        const AutomationPoint* prevPoint = singleNote->nextTick ? activePoint(ctx.curves, key, info.tick) : nullptr;

        AutomationPoint point;
        point.inValue = AutomationPoint::FromPrevious {};
        point.outValue = singleNote->value;
        point.itemId = info.eid;
        point.generated = true;
        addDynamicPoint(key, info.tick, point, info.priority, ctx);

        if (singleNote->nextTick) {
            // Recovers to whatever was active before this dynamic
            AutomationPoint nextPoint = prevPoint ? *prevPoint : AutomationPoint{};
            nextPoint.inValue = point.outValue;
            nextPoint.generated = true;
            tryAddDynamicPoint(key, *singleNote->nextTick, nextPoint, info.priority, ctx);
        }

        return;
    }

    if (const auto* compound = std::get_if<DynamicInfo::Compound>(&info.kind)) {
        AutomationPoint startPoint;
        startPoint.inValue = AutomationPoint::FromPrevious {};
        startPoint.outValue = compound->startValue;
        startPoint.itemId = info.eid;
        startPoint.generated = true;
        addDynamicPoint(key, info.tick, startPoint, info.priority, ctx);

        AutomationPoint endPoint;
        endPoint.inValue = AutomationPoint::SameAsOut {};
        endPoint.outValue = compound->endValue;
        endPoint.itemId = info.eid;
        endPoint.generated = true;
        tryAddDynamicPoint(key, compound->endPointTick, endPoint, info.priority, ctx);
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
        startPoint.inValue = AutomationPoint::FromPrevious {};
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
        }
        return;
    }

    if (info.from < info.to) {
        AutomationPoint point;
        point.inValue = AutomationPoint::SameAsOut {};
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

        curve.insert(baseIt->second.cbegin(), baseIt->second.cend());
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
                for (auto it = curve->lower_bound(srcFrom), end = curve->lower_bound(srcTo); it != end; ++it) {
                    if (it->second.generated) {
                        curve->insert({ it->first + tickShift, it->second });
                    } else {
                        AutomationPoint mirroredPoint = it->second;
                        mirroredPoint.generated = true;
                        curve->insert({ it->first + tickShift, mirroredPoint });
                    }
                }
            }
        }
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

void ScoreAutomationController::addDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point,
                                                int priority, UpdateContext& ctx)
{
    // If a point with the same priority already exists at this tick, merge them:
    // keep the existing arrival value and use this point's departure value
    const auto prioKeyIt = ctx.dynamicPriorities.find(key);
    if (prioKeyIt != ctx.dynamicPriorities.end()) {
        const auto prioIt = prioKeyIt->second.find(tick);
        if (prioIt != prioKeyIt->second.end() && prioIt->second == priority) {
            AutomationPoint& existing = ctx.curves[key][tick];
            const AutomationPoint::InValue arrivalValue = existing.inValue;
            existing = point;
            existing.inValue = arrivalValue;
            return;
        }
    }

    tryAddDynamicPoint(key, tick, point, priority, ctx);
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

void ScoreAutomationController::mirrorEditsToRepeats(const AutomationCurveKey& key, AutomationPointEdits& edits)
{
    TRACEFUNC;

    const RepeatList& repeatList = m_score->repeatList();
    if (repeatList.size() <= 1) {
        return;
    }

    const Staff* staff = m_score->staffById(key.staffId);
    IF_ASSERT_FAILED(staff) {
        return;
    }

    const StaffRange range(m_score, staff->idx(), staff->idx());

    // Bounded by the original size, since mirrored edits are appended to the same vector below
    const size_t originalEditCount = edits.size();
    for (size_t i = 0; i < originalEditCount; ++i) {
        // Copied by value: push_back below may reallocate edits
        AutomationPointEdit localEdit = edits[i];
        const auto sourceIt = repeatList.findRepeatSegmentFromUTick(localEdit.tick);
        IF_ASSERT_FAILED(sourceIt != repeatList.cend()) {
            continue;
        }

        const RepeatSegment* sourceSeg = *sourceIt;
        const int sourceTickOffset = sourceSeg->utick - sourceSeg->tick;

        // Re-express the edit in tick coordinates local to sourceSeg
        localEdit.tick -= sourceTickOffset;
        if (auto* movePoint = std::get_if<AutomationPointEdit::MovePoint>(&localEdit.change)) {
            movePoint->from -= sourceTickOffset;
        }

        for (const RepeatSegment* targetSeg : repeatList) {
            if (targetSeg != sourceSeg) {
                const MirrorRange targetRange { targetSeg->tick, targetSeg->endTick(), targetSeg->utick - targetSeg->tick };
                mirrorPointIfInRange(localEdit, targetRange, edits);
            }
            mirrorToMeasureRepeats(targetSeg, range, localEdit, edits);
        }
    }
}

void ScoreAutomationController::mirrorPointIfInRange(const AutomationPointEdit& localEdit, const MirrorRange& range,
                                                     AutomationPointEdits& allEdits)
{
    const int localTick = static_cast<int>(localEdit.tick);
    if (localTick < range.from || localTick >= range.toExclusive) {
        return;
    }

    const utick_t mirroredTick = static_cast<utick_t>(localTick + range.tickOffset);

    if (std::holds_alternative<AutomationPointEdit::ErasePoint>(localEdit.change)) {
        allEdits.push_back({ mirroredTick, AutomationPointEdit::ErasePoint {} });
        return;
    }

    const auto* movePoint = std::get_if<AutomationPointEdit::MovePoint>(&localEdit.change);

    // Mirrored points are derived from the user's edit, not edited directly, so they are marked generated
    AutomationPoint mirroredPoint = movePoint ? movePoint->point : std::get<AutomationPointEdit::SetPoint>(localEdit.change).point;
    mirroredPoint.generated = true;

    if (movePoint && movePoint->from >= range.from && movePoint->from < range.toExclusive) {
        const utick_t mirroredMoveFrom = static_cast<utick_t>(movePoint->from + range.tickOffset);
        allEdits.push_back({ mirroredTick, AutomationPointEdit::MovePoint { mirroredPoint, mirroredMoveFrom } });
    } else {
        allEdits.push_back({ mirroredTick, AutomationPointEdit::SetPoint { mirroredPoint } });
    }
}

void ScoreAutomationController::mirrorToMeasureRepeats(const RepeatSegment* targetSeg, const StaffRange& range,
                                                       const AutomationPointEdit& localEdit, AutomationPointEdits& allEdits)
{
    const int tickOffset = targetSeg->utick - targetSeg->tick;

    MeasureRepeats measureRepeats;
    for (const Measure* measure : targetSeg->measureList()) {
        for (const Segment* segment = measure->first(RELEVANT_SEGMENT_TYPES); segment; segment = segment->next(RELEVANT_SEGMENT_TYPES)) {
            collectMeasureRepeats(segment, tickOffset, range, measureRepeats);
        }
    }

    for (const auto& [mr, mrTickOffset] : measureRepeats) {
        const Measure* currMeasure = mr->firstMeasureOfGroup();
        for (int num = 0; currMeasure && num < mr->numMeasures(); ++num, currMeasure = currMeasure->nextMeasure()) {
            const Measure* referringMeasure = mr->referringMeasure(currMeasure);
            IF_ASSERT_FAILED(referringMeasure && referringMeasure != currMeasure) {
                continue;
            }

            const int measureFrom = referringMeasure->tick().ticks();
            const int measureToExclusive = referringMeasure->endTick().ticks();
            const int tickShift = currMeasure->tick().ticks() - measureFrom;

            const MirrorRange measureRange { measureFrom, measureToExclusive, tickShift + mrTickOffset };
            mirrorPointIfInRange(localEdit, measureRange, allEdits);
        }
    }
}
