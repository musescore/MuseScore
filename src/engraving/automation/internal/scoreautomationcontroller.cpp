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

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "engraving/dom/score.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/volta.h"

#include "engraving/automation/automationdata.h"
#include "engraving/automation/dynamicvalues.h"
#include "engraving/automation/tempovalues.h"

#include "engraving/types/constants.h"

#include "global/containers.h"
#include "global/realfn.h"
#include "global/log.h"

using namespace mu::engraving;

static constexpr real_t DYNAMIC_STEP = real_t::make(0.05);

// Dynamics, measure repeats, fermatas, and tempo texts can only appear on these segment types
static constexpr SegmentType RELEVANT_SEGMENT_TYPES = SegmentType::ChordRest | SegmentType::TimeTick;

static BeatsPerSecond roundTempo(const BeatsPerSecond& bps)
{
    return muse::RealRound(bps.val, Constants::TEMPO_PRECISION);
}

static AutomationPoint makeTempoPoint(real_t normalizedBps, std::optional<EID> itemId)
{
    AutomationPoint point;
    point.value.outValue = std::clamp(normalizedBps, MIN_NORMALIZED_TEMPO, MAX_NORMALIZED_TEMPO);
    point.itemId = itemId;
    point.generated = true;
    return point;
}

//! TODO: map EXPONENTIAL/EASE_IN/EASE_OUT/EASE_IN_OUT to a real ease shape; every method is linear for now
static AutomationPoint::Ease changeMethodToEase(ChangeMethod)
{
    return AutomationPoint::Ease::none();
}

//! NOTE: segments are ordered by utick; returns the index of the last one starting at or before utick
static std::optional<size_t> findSegmentIndex(const std::vector<RepeatSegmentInfo>& segments, utick_t utick)
{
    const auto it = std::upper_bound(segments.begin(), segments.end(), utick,
                                     [](utick_t u, const RepeatSegmentInfo& seg) { return u < seg.utick; });
    if (it == segments.begin()) {
        return std::nullopt;
    }

    return size_t(std::prev(it) - segments.begin());
}

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

    if (key.voiceIdx().has_value()) {
        const AutomationCurveKey sharedKey = key.withoutVoice();
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

static real_t currentTempoAt(const AutomationCurve& tempoCurve, utick_t tick)
{
    const auto it = muse::findLessOrEqual(tempoCurve, tick);
    return it != tempoCurve.cend() ? it->second.value.outValue : DEFAULT_NORMALIZED_TEMPO;
}

struct ChangeRelevance {
    bool dynamics = false;
    bool tempo = false;
};

static ChangeRelevance classifyChanges(const ScoreChanges& changes)
{
    ChangeRelevance result;

    for (const ElementType type : changes.changedTypes) {
        switch (type) {
        case ElementType::DYNAMIC:
        case ElementType::HAIRPIN:
        case ElementType::HAIRPIN_SEGMENT:
        case ElementType::MEASURE_REPEAT:
            result.dynamics = true;
            break;
        case ElementType::BREATH:
        case ElementType::LAYOUT_BREAK:
        case ElementType::FERMATA:
        case ElementType::GRADUAL_TEMPO_CHANGE:
        case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
        case ElementType::TEMPO_TEXT:
            result.tempo = true;
            break;
        case ElementType::VOLTA:
        case ElementType::VOLTA_SEGMENT:
        case ElementType::MARKER:
        case ElementType::JUMP:
            return { true, true };
        default:
            break;
        }

        if (result.dynamics && result.tempo) {
            break;
        }
    }

    // VoiceAssignment change shifts which staves a dynamic/hairpin covers,
    // so the old points on staves outside the new assignment must also be cleared
    result.dynamics |= muse::contains(changes.changedPropertyIdSet, Pid::VOICE_ASSIGNMENT);

    static const std::unordered_set<Pid> REPEAT_PROPERTIES {
        Pid::REPEAT_START,
        Pid::REPEAT_END,
        Pid::REPEAT_JUMP,
        Pid::REPEAT_COUNT,
    };

    bool repeatPropertyChanged = false;
    for (const Pid pid : REPEAT_PROPERTIES) {
        if (muse::contains(changes.changedPropertyIdSet, pid)) {
            repeatPropertyChanged = true;
            break;
        }
    }

    if (repeatPropertyChanged) {
        result.dynamics = true;
        result.tempo = true;
    }

    return result;
}

const TempoTimeline& ScoreAutomationController::tempoTimeline(bool expandRepeats) const
{
    if (m_tempoTimelineOverride) {
        return *m_tempoTimelineOverride;
    }

    return expandRepeats ? m_tempoTimeline : m_flattenedTempoTimeline;
}

void ScoreAutomationController::setTempoMultiplier(const BeatsPerSecond& bps)
{
    m_tempoTimeline.setTempoMultiplier(bps);
    m_flattenedTempoTimeline.setTempoMultiplier(bps);
}

void ScoreAutomationController::setTempoTimelineOverride(std::optional<TempoTimeline> timeline)
{
    m_tempoTimelineOverride = std::move(timeline);
}

void ScoreAutomationController::init(Score* score)
{
    m_score = score;

    if (!m_automationData) {
        m_automationData = std::make_shared<AutomationData>();
    }

    AutomationCurveMap curves = m_automationData->curves();
    fullRebuild(curves);
}

void ScoreAutomationController::fullRebuild(AutomationCurveMap& curves, bool includeTempo, bool includeDynamics)
{
    const UpdateRequest request { 0, muse::nidx, muse::nidx, includeTempo, includeDynamics };
    update(request, curves);
}

void ScoreAutomationController::ensureInitialized(Score* score)
{
    if (m_score) {
        return;
    }
    init(score);
}

void ScoreAutomationController::insertTime(const Fraction& tick, const Fraction& len, const std::vector<RepeatSegmentInfo>& oldSegments)
{
    TRACEFUNC;

    const int diff = len.ticks();
    if (!m_score || !m_automationData || m_automationData->isEmpty() || diff == 0) {
        return;
    }

    const int rawInsertTick = tick.ticks();
    const std::vector<RepeatSegmentInfo> newSegments = m_score->expandedRepeatList().segmentInfoList();

    AutomationCurveMap curves = m_automationData->curves();

    for (auto& entry : curves) {
        AutomationCurve& curve = entry.second;
        AutomationCurve shifted;

        for (const auto& [utick, point] : curve) {
            // Generated points are regenerated by the full update below - only authored points need relocating
            if (point.generated) {
                continue;
            }

            // Re-express the point in raw-tick coordinates local to its repeat pass,
            // so the shift below is correct even for a pass other than the first
            const std::optional<size_t> oldIdx = findSegmentIndex(oldSegments, utick);
            if (!oldIdx || *oldIdx >= newSegments.size()) {
                continue; // no corresponding pass anymore (edit changed the repeat structure itself)
            }

            const RepeatSegmentInfo& oldSeg = oldSegments[*oldIdx];
            const int oldRawTick = utick - (oldSeg.utick - oldSeg.tick);

            if (diff < 0 && oldRawTick >= rawInsertTick && oldRawTick < rawInsertTick - diff) {
                continue; // point sat inside the removed range
            }

            const int newRawTick = oldRawTick >= rawInsertTick ? oldRawTick + diff : oldRawTick;

            // Re-derive the utick for the *same* repeat pass under the new (post-edit) RepeatList
            const RepeatSegmentInfo& newSeg = newSegments[*oldIdx];
            const utick_t newUtick = utick_t(newRawTick + (newSeg.utick - newSeg.tick));

            shifted.emplace_hint(shifted.end(), newUtick, point);
        }

        curve = std::move(shifted);
    }

    // Kept as empty entries, not erased: replaceCurves() only clears a key when given it with an
    // empty curve - a key absent from the map below would leave stale data untouched instead
    fullRebuild(curves);
}

void ScoreAutomationController::update(const ScoreChanges& changes)
{
    if (changes.isTextEditing) {
        return;
    }

    const ChangeRelevance relevance = classifyChanges(changes);
    if (!relevance.dynamics && !relevance.tempo) {
        return;
    }

    UpdateRequest request;
    request.includeTempo = relevance.tempo;
    request.includeDynamics = relevance.dynamics;

    // When tempo is relevant, the whole score must be reparsed in one full pass,
    // so tickFrom/staffIdxFrom/staffIdxTo are left at their defaults (0/nidx/nidx)
    if (!relevance.tempo) {
        request.tickFrom = std::max(changes.tickFrom, 0);

        // VoiceAssignment change shifts which staves a dynamic/hairpin covers,
        // so the old points on staves outside the new assignment must also be cleared
        const bool voiceAssignmentChanged = muse::contains(changes.changedPropertyIdSet, Pid::VOICE_ASSIGNMENT);
        if (!voiceAssignmentChanged) {
            request.staffIdxFrom = changes.staffIdxFrom;
            request.staffIdxTo = changes.staffIdxTo;
        }
    }

    if (!m_automationData) {
        m_automationData = std::make_shared<AutomationData>();
    }

    update(request, m_automationData->curves());
}

void ScoreAutomationController::editPoints(const AutomationCurveKey& key, AutomationPointEdits& edits)
{
    IF_ASSERT_FAILED(m_automationData && m_score) {
        return;
    }

    // Tempo edits can affect later relative tempo markings, so we need a full rescan
    if (key.type == AutomationType::Tempo) {
        AutomationCurveMap curves = m_automationData->curves();
        AutomationCurve& tempoCurve = curves[key];

        for (const AutomationPointEdit& edit : edits) {
            if (const auto* setPoint = std::get_if<AutomationPointEdit::SetPoint>(&edit.change)) {
                tempoCurve[edit.tick] = setPoint->point;
            } else if (const auto* movePoint = std::get_if<AutomationPointEdit::MovePoint>(&edit.change)) {
                if (movePoint->from != edit.tick) {
                    tempoCurve.erase(movePoint->from);
                }
                tempoCurve[edit.tick] = movePoint->point;
            } else {
                tempoCurve.erase(edit.tick);
            }
        }

        fullRebuild(curves, /*includeTempo*/ true, /*includeDynamics*/ false);
        return;
    }

    // Dynamics/Volume/Pan edits don't affect other points, so no rescan is needed
    mirrorEditsToRepeats(key, edits);
    m_automationData->editPoints(key, edits);
}

void ScoreAutomationController::update(const UpdateRequest& request, const AutomationCurveMap& curves)
{
    TRACEFUNC;

    if (!m_score) {
        return;
    }

    const RepeatList& repeatList = m_score->expandedRepeatList();
    if (repeatList.empty()) {
        return;
    }

    const auto repeatFromIt = std::find_if(repeatList.cbegin(), repeatList.cend(),
                                           [tickFrom = request.tickFrom](const RepeatSegment* seg) {
        return seg->endTick() > tickFrom;
    });

    IF_ASSERT_FAILED(repeatFromIt != repeatList.cend()) {
        return;
    }

    const StaffRange range(m_score, request.staffIdxFrom, request.staffIdxTo);
    const RepeatSegment* firstSeg = *repeatFromIt;

    UpdateContext ctx;
    ctx.request = request;
    ctx.tempoMultiplier = m_tempoTimeline.tempoMultiplier();

    // The first utick the rebuild will regenerate; everything before it is kept as is
    ctx.clearFromUTick = std::max(firstSeg->utick, request.tickFrom + (firstSeg->utick - firstSeg->tick));

    // Copy only the curves this update can affect, dropping the generated points that the rebuild
    // below re-creates; all other curves stay untouched
    copyCurvesForRebuild(curves, range, ctx.clearFromUTick, request.includeTempo, request.includeDynamics, ctx.curves);

    AutomationCurve emptyTempoCurve;

    if (request.includeTempo) {
        ctx.tempoCurve = &ctx.curves[TEMPO_KEY];
        fillNoRepeatTempoCurve(*ctx.tempoCurve, ctx.noRepeatTempoCurve);
    } else {
        // Read-only access for e.g. compound dynamics,
        // which need the current tempo even when this update doesn't rebuild it
        const auto tempoIt = ctx.curves.find(TEMPO_KEY);
        ctx.tempoCurve = tempoIt != ctx.curves.end() ? &tempoIt->second : &emptyTempoCurve;
    }

    // Step 1: segment dynamics (+ tempo/pause when relevant): populates ctx.dynamicPriorities
    int prevSegRawEndTick = -1;
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        const int tickOffset = seg->utick - seg->tick;

        if (request.includeTempo && prevSegRawEndTick != -1 && seg->tick != prevSegRawEndTick) {
            // Playback jumped elsewhere in the score (repeat loop-back / volta switch) - this
            // pass's start needs its tempo reset from raw tick order; resolved after Step 2, once
            // any volta reset affecting the lookup has landed in noRepeatTempoCurve
            ctx.pendingTempoResets.push_back({ seg->utick, seg->tick });
        }
        prevSegRawEndTick = seg->endTick();

        // tickFrom limits only the first affected repeat segment: later segments may replay
        // measures from before tickFrom, and all their points were cleared, so rebuild them in full
        const int measureFrom = (it == repeatFromIt) ? std::max(seg->tick, request.tickFrom) : seg->tick;

        const std::vector<const Measure*>& segMeasures = seg->measureList();
        for (size_t mi = 0; mi < segMeasures.size(); ++mi) {
            const Measure* measure = segMeasures[mi];
            if (measure->endTick().ticks() <= measureFrom) {
                continue;
            }

            if (request.includeTempo) {
                collectPauses(measure, tickOffset, ctx.pauses, ctx.noRepeatPauses);
                if (measure->isAnacrusis()) {
                    std::optional<utick_t> nextMeasureUTick;
                    if (mi + 1 < segMeasures.size()) {
                        nextMeasureUTick = segMeasures[mi + 1]->tick().ticks() + tickOffset;
                    }
                    ctx.anacrusisMeasures.push_back({ measure, tickOffset, nextMeasureUTick });
                }
            }

            // A MeasureRepeat can only ever sit on the measure's first ChordRest segment
            const Segment* firstChordRestSegment = measure->first(SegmentType::ChordRest);
            if (firstChordRestSegment && firstChordRestSegment->tick().ticks() >= measureFrom) {
                collectMeasureRepeats(firstChordRestSegment, tickOffset, range, ctx.measureRepeats);
            }

            for (const Segment* segment = measure->first(RELEVANT_SEGMENT_TYPES); segment;
                 segment = segment->next(RELEVANT_SEGMENT_TYPES)) {
                if (segment->annotations().empty() || segment->tick().ticks() < measureFrom) {
                    continue;
                }

                addSegmentPoints(segment, tickOffset, range, ctx);
            }
        }
    }

    if (!ctx.anacrusisMeasures.empty()) {
        fixAnacrusisTempo(ctx);
    }

    // Step 2: spanner points (+ gradual tempo change when relevant):
    // ctx.dynamicPriorities fully populated; sets inValues on hairpin-end dynamics
    for (auto it = repeatFromIt; it != repeatList.cend(); ++it) {
        const RepeatSegment* seg = *it;
        addSpannerPoints(m_score, seg->tick, seg->endTick(), seg->utick - seg->tick, range, ctx);
    }

    // Step 3: resolve pending tempo resets now that noRepeatTempoCurve is fully built
    resolvePendingTempoResets(ctx);

    // Step 4: fill each voice curve with any points from the base (all-voice) curve it doesn't already have
    fillVoiceCurvesFromBase(ctx);

    // Step 5: measure repeats
    addMeasureRepeatPoints(ctx);

    // Step 6: Mirror directly-authored points to repeats
    mirrorAuthoredPointsToRepeats(ctx);

    if (request.includeTempo) {
        // TempoTimeline falls back to the default tempo when the curve has no points
        m_tempoTimeline.rebuild(*ctx.tempoCurve, ctx.pauses);
        m_flattenedTempoTimeline.rebuild(ctx.noRepeatTempoCurve, ctx.noRepeatPauses);
    }

    // Passed through as-is: replaceCurves() clears a key when given an empty curve for it
    m_automationData->replaceCurves(ctx.curves);
}

void ScoreAutomationController::fillNoRepeatTempoCurve(const AutomationCurve& tempoCurve, AutomationCurve& noRepeatTempoCurve)
{
    TRACEFUNC;

    const RepeatList& repeatList = m_score->expandedRepeatList();

    // tempoCurve is expected to be authored-only
    for (const auto& [utick, point] : tempoCurve) {
        const auto segIt = repeatList.findRepeatSegmentFromUTick(utick);
        IF_ASSERT_FAILED(segIt != repeatList.cend()) {
            continue;
        }

        const int segTickOffset = (*segIt)->utick - (*segIt)->tick;
        noRepeatTempoCurve.emplace(utick - segTickOffset, point);
    }
}

void ScoreAutomationController::mirrorAuthoredPointsToRepeats(UpdateContext& ctx)
{
    TRACEFUNC;

    for (auto& [key, curve] : ctx.curves) {
        if (curve.empty()) {
            continue;
        }

        AutomationPointEdits edits;
        for (const auto& [tick, point] : curve) {
            if (!point.generated) {
                edits.push_back({ tick, AutomationPointEdit::SetPoint { point } });
            }
        }

        if (edits.empty()) {
            continue;
        }

        //! NOTE: appends the corresponding mirrored edit(s) - one per other repeat pass - directly to edits
        mirrorEditsToRepeats(key, edits);

        for (const AutomationPointEdit& edit : edits) {
            const auto* setPoint = std::get_if<AutomationPointEdit::SetPoint>(&edit.change);
            IF_ASSERT_FAILED(setPoint) {
                continue;
            }

            curve.insert_or_assign(edit.tick, setPoint->point);
        }
    }
}

void ScoreAutomationController::copyCurvesForRebuild(const AutomationCurveMap& curves, const StaffRange& range, utick_t clearFromUTick,
                                                     bool includeTempo, bool includeDynamics, AutomationCurveMap& destCurves)
{
    TRACEFUNC;

    for (const auto& [key, curve] : curves) {
        const bool isDynamics = key.type == AutomationType::Dynamics;
        const bool isTempo = key.type == AutomationType::Tempo;

        // Instrument curves (Volume, Pan) aren't staff-scoped or score-derived, so only
        // user points survive here - mirrorAuthoredPointsToRepeats() recomputes the rest
        if (isDynamics) {
            if (!includeDynamics) {
                continue;
            }

            const std::optional<muse::ID> staffId = key.staffId();
            if (!staffId || !range.contains(*staffId)) {
                continue;
            }
        }

        AutomationCurve& curveCopy = destCurves.emplace_hint(destCurves.end(), key, AutomationCurve())->second;

        for (const auto& [tick, point] : curve) {
            // Dynamics: keep generated points before clearFromUTick (the incremental rebuild regenerates the rest)
            // Tempo: keep generated points only if this update doesn't include tempo (otherwise the whole curve is regenerated below)
            // Instrument: never keep generated points (there's no generation step for them, only user edits)
            const bool keepGenerated = isDynamics ? tick < clearFromUTick : isTempo && !includeTempo;
            if (!point.generated || keepGenerated) {
                curveCopy.emplace_hint(curveCopy.end(), tick, point);
            }
        }
    }
}

void ScoreAutomationController::addSegmentPoints(const Segment* segment, int tickOffset, const StaffRange& range,
                                                 UpdateContext& ctx)
{
    TRACEFUNC;

    double fermataStretch = 0.0;
    const Fermata* fermata = nullptr;

    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation) {
            continue;
        }

        if (annotation->isDynamic()) {
            if (ctx.request.includeDynamics && range.contains(annotation->staffIdx())) {
                addDynamicPoints(toDynamic(annotation), tickOffset, range, ctx);
            }
        } else if (ctx.request.includeTempo && annotation->isFermata() && toFermata(annotation)->play()) {
            const Fermata* f = toFermata(annotation);
            if (f->timeStretch() > fermataStretch) {
                fermataStretch = f->timeStretch();
                fermata = f;
            }
        } else if (ctx.request.includeTempo && annotation->isTempoText()) {
            addTempoTextPoint(toTempoText(annotation), tickOffset, ctx);
        }
    }

    if (fermata && !muse::RealIsNull(fermataStretch) && !muse::RealIsEqual(fermataStretch, 1.0)) {
        addFermataStretchPoints(fermata, tickOffset, fermataStretch, ctx);
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
        DynamicInfo::SingleNote singleNote { singleNoteIt->second, std::nullopt };
        if (const Segment* nextSeg = dynamic->segment()->next()) {
            singleNote.nextTick = nextSeg->tick().ticks() + tickOffset;
        }
        info.kind = singleNote;
    } else if (auto compoundIt = COMPOUND_DYNAMIC_VALUES.find(dynamicType); compoundIt != COMPOUND_DYNAMIC_VALUES.end()) {
        const BeatsPerSecond tempo = denormalizeTempo(currentTempoAt(*ctx.tempoCurve, tick)) * ctx.tempoMultiplier;
        info.kind = DynamicInfo::Compound { compoundIt->second.first, compoundIt->second.second,
                                            tick + dynamic->velocityChangeLength(tempo).ticks() };
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

    AutomationCurve& curve = ctx.curves[key];
    std::map<utick_t, int>& tickPrioMap = ctx.dynamicPriorities[key];

    if (const auto* ordinary = std::get_if<DynamicInfo::Ordinary>(&info.kind)) {
        AutomationPoint point;
        point.value.inValue = AutomationPoint::ArrivalFromPrevious {};
        point.value.outValue = ordinary->value;
        point.itemId = info.eid;
        point.generated = true;
        addDynamicPoint(curve, tickPrioMap, info.tick, point, info.priority);
        return;
    }

    if (const auto* singleNote = std::get_if<DynamicInfo::SingleNote>(&info.kind)) {
        const AutomationPoint* prevPoint = singleNote->nextTick ? activePoint(ctx.curves, key, info.tick) : nullptr;

        AutomationPoint point;
        point.value.inValue = AutomationPoint::ArrivalFromPrevious {};
        point.value.outValue = singleNote->value;
        point.itemId = info.eid;
        point.generated = true;
        addDynamicPoint(curve, tickPrioMap, info.tick, point, info.priority);

        if (singleNote->nextTick) {
            // Recovers to whatever was active before this dynamic
            AutomationPoint nextPoint = prevPoint ? *prevPoint : AutomationPoint{};
            const AutomationPoint::Ease preservedEase = ease(nextPoint).value_or(AutomationPoint::Ease::none());
            nextPoint.value.inValue = AutomationPoint::ExplicitArrival { point.value.outValue, preservedEase };
            nextPoint.generated = true;
            tryAddDynamicPoint(curve, tickPrioMap, *singleNote->nextTick, nextPoint, info.priority);
        }

        return;
    }

    if (const auto* compound = std::get_if<DynamicInfo::Compound>(&info.kind)) {
        AutomationPoint startPoint;
        startPoint.value.inValue = AutomationPoint::ArrivalFromPrevious {};
        startPoint.value.outValue = compound->startValue;
        startPoint.itemId = info.eid;
        startPoint.generated = true;
        addDynamicPoint(curve, tickPrioMap, info.tick, startPoint, info.priority);

        AutomationPoint endPoint;
        endPoint.value.outValue = compound->endValue;
        endPoint.value.inValue = AutomationPoint::ExplicitArrival { endPoint.value.outValue, AutomationPoint::Ease::none() };
        endPoint.itemId = info.eid;
        endPoint.generated = true;
        tryAddDynamicPoint(curve, tickPrioMap, compound->endPointTick, endPoint, info.priority);
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
        if (!spanner->playSpanner()) {
            continue;
        }

        if (spanner->isHairpin()) {
            if (!ctx.request.includeDynamics) {
                continue;
            }
            const Hairpin* hairpin = toHairpin(spanner);
            const std::vector<AutomationCurveKey> keys = resolveKeys(hairpin, AutomationType::Dynamics, range);
            if (!keys.empty()) {
                addHairpinPoints(hairpin, tickOffset, keys, ctx);
            }
        } else if (ctx.request.includeTempo && spanner->isGradualTempoChange()) {
            addGradualTempoChangePoints(toGradualTempoChange(spanner), tickOffset, ctx);
        } else if (ctx.request.includeTempo && spanner->isVolta()) {
            addVoltaTempoResetPoint(toVolta(spanner), ctx);
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
            const BeatsPerSecond tempo = denormalizeTempo(currentTempoAt(*ctx.tempoCurve, info.from)) * ctx.tempoMultiplier;
            info.from += startDynamic->velocityChangeLength(tempo).ticks();
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
    AutomationCurve& curve = ctx.curves[key];
    std::map<utick_t, int>& tickPrioMap = ctx.dynamicPriorities[key];

    // --- Determine valueFrom
    const AutomationPoint* prevPoint = activePoint(ctx.curves, key, info.from);
    const real_t prevOutValue = prevPoint ? prevPoint->value.outValue : real_t(0.0);

    // If the hairpin has no specific start value, use the currently-applicable value at the start tick of the hairpin
    const real_t valueFrom = info.nominalValueFrom.value_or(prevOutValue);

    {
        AutomationPoint startPoint;
        startPoint.value.outValue = valueFrom;
        startPoint.value.inValue = AutomationPoint::ArrivalFromPrevious {};
        startPoint.itemId = info.eid;
        startPoint.generated = true;
        tryAddDynamicPoint(curve, tickPrioMap, info.from, startPoint, info.priority);
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

    const auto endPointIt = curve.find(info.to);
    const bool hasPointAtEnd = endPointIt != curve.end();

    if (hasPointAtEnd) {
        // A point already exists at the end tick; encode the hairpin's arrival via inValue, but only
        // if this hairpin has at least as much priority as whoever placed that point
        const auto tickIt = tickPrioMap.find(info.to);
        const bool canModify = tickIt == tickPrioMap.end() || info.priority >= tickIt->second;
        if (canModify) {
            const AutomationPoint::Ease preservedEase = ease(endPointIt->second).value_or(AutomationPoint::Ease::none());
            endPointIt->second.value.inValue = AutomationPoint::ExplicitArrival { valueTo, preservedEase };
        }
        return;
    }

    if (info.from < info.to) {
        AutomationPoint point;
        point.value.outValue = valueTo;
        point.value.inValue = AutomationPoint::ExplicitArrival { point.value.outValue, AutomationPoint::Ease::none() };
        point.itemId = info.eid;
        point.generated = true;
        tryAddDynamicPoint(curve, tickPrioMap, info.to, point, info.priority);
    }
}

void ScoreAutomationController::fillVoiceCurvesFromBase(UpdateContext& ctx)
{
    TRACEFUNC;

    for (auto& [key, curve] : ctx.curves) {
        if (!key.voiceIdx().has_value()) {
            continue;
        }

        const AutomationCurveKey baseKey = key.withoutVoice();
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
    if (!segment->isChordRestType()) {
        return;
    }

    TRACEFUNC;

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

    std::unordered_map<uint64_t, std::vector<AutomationCurve*> > curvesByStaff;
    for (auto& [key, curve] : ctx.curves) {
        const std::optional<muse::ID> staffId = key.staffId();
        if (staffId) {
            curvesByStaff[staffId->toUint64()].push_back(&curve);
        }
    }

    if (curvesByStaff.empty()) {
        return;
    }

    struct MeasureRange {
        int tickShift = 0;
        utick_t srcFrom = 0;
        utick_t srcTo = 0;
    };

    for (const auto& [mr, tickOffset] : ctx.measureRepeats) {
        const Staff* staff = mr->staff();
        if (!staff) {
            continue;
        }

        const auto staffCurvesIt = curvesByStaff.find(staff->id().toUint64());
        if (staffCurvesIt == curvesByStaff.end()) {
            continue;
        }

        std::vector<MeasureRange> ranges;
        ranges.reserve(mr->numMeasures());

        const Measure* currMeasure = mr->firstMeasureOfGroup();
        for (int num = 0; currMeasure && num < mr->numMeasures(); ++num, currMeasure = currMeasure->nextMeasure()) {
            const Measure* referringMeasure = mr->referringMeasure(currMeasure);
            IF_ASSERT_FAILED(referringMeasure && referringMeasure != currMeasure) {
                continue;
            }

            const int referringMeasureTick = referringMeasure->tick().ticks();
            ranges.push_back({ currMeasure->tick().ticks() - referringMeasureTick,
                               referringMeasureTick + tickOffset,
                               referringMeasure->endTick().ticks() + tickOffset });
        }

        if (ranges.empty()) {
            continue;
        }

        for (AutomationCurve* curve : staffCurvesIt->second) {
            const MeasureRange& firstRange = ranges.front();
            auto srcIt = curve->lower_bound(firstRange.srcFrom);
            const auto curveEnd = curve->end();
            if (srcIt == curveEnd) {
                continue;
            }

            const auto destHint = curve->lower_bound(firstRange.srcFrom + firstRange.tickShift);

            for (const MeasureRange& range : ranges) {
                // Ranges are contiguous for consecutive measures, but a skipped measure (assert
                // failure above) can leave a gap; catch up if srcIt fell behind this range's start
                if (srcIt != curveEnd && srcIt->first < range.srcFrom) {
                    srcIt = curve->lower_bound(range.srcFrom);
                }

                for (; srcIt != curveEnd && srcIt->first < range.srcTo; ++srcIt) {
                    if (srcIt->second.generated) {
                        curve->insert(destHint, { srcIt->first + range.tickShift, srcIt->second });
                    } else {
                        AutomationPoint mirroredPoint = srcIt->second;
                        mirroredPoint.generated = true;
                        curve->insert(destHint, { srcIt->first + range.tickShift, mirroredPoint });
                    }
                }
            }
        }
    }
}

void ScoreAutomationController::collectPauses(const Measure* measure, int tickOffset, PausesMap& pauses,
                                              PausesMap& noRepeatPauses)
{
    const Fraction& startTick = measure->tick();
    const int startUtick = startTick.ticks() + tickOffset;

    // Implement section break rest
    for (MeasureBase* mb = measure->prev(); mb && mb->endTick() == startTick; mb = mb->prev()) {
        if (mb->pause()) {
            pauses[startUtick] = mb->pause();
            // Identical value across every pass that plays this raw tick, so last-write-wins is fine
            noRepeatPauses[startTick.ticks()] = mb->pause();
        }
    }

    // Add pauses from the end of the previous measure (at measure->tick()):
    for (Segment* s = measure->first(); s && s->tick() == startTick; s = s->prev1()) {
        if (!s->isBreathType()) {
            continue;
        }
        double length = 0.0;
        for (EngravingItem* e : s->elist()) {
            if (e && e->isBreath()) {
                length = std::max(length, toBreath(e)->pause());
            }
        }
        if (!muse::RealIsNull(length)) {
            pauses[startUtick] = length;
            noRepeatPauses[startTick.ticks()] = length;
        }
    }

    for (const Segment& segment : measure->segments()) {
        if (!segment.isBreathType()) {
            continue;
        }

        double length = 0.0;
        Fraction tick = segment.tick();
        // find longest pause
        for (track_idx_t i = 0, n = measure->score()->ntracks(); i < n; ++i) {
            EngravingItem* e = segment.element(i);
            if (e && e->isBreath()) {
                length = std::max(length, toBreath(e)->pause());
            }
        }
        if (!muse::RealIsNull(length)) {
            pauses[tick.ticks() + tickOffset] = length;
            noRepeatPauses[tick.ticks()] = length;
        }
    }
}

void ScoreAutomationController::addTempoTextPoint(const TempoText* tt, int tickOffset, UpdateContext& ctx)
{
    if (!tt->playTempoText()) {
        return;
    }

    if (tt->isNormal() && !tt->isRelative() && !ctx.tempoPrimo) {
        ctx.tempoPrimo = roundTempo(tt->tempo());
    }

    const utick_t tick = tt->segment()->tick().ticks() + tickOffset;

    EID itemId = tt->eid();
    if (!itemId.isValid()) {
        itemId = tt->assignNewEID();
    }

    if (tt->isATempo() && tt->followText()) {
        // This will effectively reset the tempo to the previous one when a progressive change was active
        setTempoPoint(tick, tickOffset, currentTempoAt(*ctx.tempoCurve, tick), ctx, itemId);
    } else if (tt->isTempoPrimo() && tt->followText()) {
        setTempoPoint(tick, tickOffset, normalizeTempo(ctx.tempoPrimo.value_or(Constants::DEFAULT_TEMPO)), ctx, itemId);
    } else if (tt->isRelative()) {
        const real_t before = currentTempoAt(*ctx.tempoCurve, tick);
        setTempoPoint(tick, tickOffset, before * tt->relativeValue(), ctx, itemId);
    } else {
        setTempoPoint(tick, tickOffset, normalizeTempo(roundTempo(tt->tempo())), ctx, itemId);
    }
}

void ScoreAutomationController::addFermataStretchPoints(const Fermata* fermata, int tickOffset, double stretch, UpdateContext& ctx)
{
    const Segment* segment = fermata->segment();
    const utick_t tick = segment->tick().ticks() + tickOffset;

    const auto beforeIt = muse::findLessOrEqual(*ctx.tempoCurve, tick);
    const bool hasBefore = beforeIt != ctx.tempoCurve->cend();
    const real_t before = hasBefore ? beforeIt->second.value.outValue : DEFAULT_NORMALIZED_TEMPO;

    EID itemId = fermata->eid();
    if (!itemId.isValid()) {
        itemId = fermata->assignNewEID();
    }

    setTempoPoint(tick, tickOffset, before / stretch, ctx, itemId);

    const Segment* nextActiveSegment = segment->next1();
    while (nextActiveSegment && !nextActiveSegment->isActive()) {
        nextActiveSegment = nextActiveSegment->next1();
    }

    const Fraction tempoEndTick = nextActiveSegment ? nextActiveSegment->tick() : segment->tick() + segment->ticks();
    const utick_t etick = tempoEndTick.ticks() + tickOffset;

    // Restore to whatever was active before the fermata, so the point belongs to that marking, not the fermata
    const auto etickIt = ctx.tempoCurve->lower_bound(etick);
    if (etickIt == ctx.tempoCurve->end() || etickIt->first != etick) {
        setTempoPoint(etickIt, etick, tickOffset, before, ctx, hasBefore ? beforeIt->second.itemId : std::nullopt);
    }
}

void ScoreAutomationController::addGradualTempoChangePoints(const GradualTempoChange* tempoChange, int tickOffset, UpdateContext& ctx)
{
    const utick_t tickFrom = tempoChange->tick().ticks() + tickOffset;
    const utick_t tickTo = tickFrom + tempoChange->ticks().ticks();
    const real_t normalizedCurrent = currentTempoAt(*ctx.tempoCurve, tickFrom);

    EID itemId = tempoChange->eid();
    if (!itemId.isValid()) {
        itemId = tempoChange->assignNewEID();
    }

    const auto tickFromIt = ctx.tempoCurve->lower_bound(tickFrom);
    if (tickFromIt == ctx.tempoCurve->end() || tickFromIt->first != tickFrom) {
        // Explicit flat arrival, so the ramp is anchored here rather than at whatever
        // unrelated marking last happened to be an arrival
        AutomationPoint startPoint;
        startPoint.value.outValue = normalizedCurrent;
        startPoint.value.inValue = AutomationPoint::ExplicitArrival { normalizedCurrent, AutomationPoint::Ease::none() };
        startPoint.itemId = itemId;
        startPoint.generated = true;
        ctx.tempoCurve->emplace_hint(tickFromIt, tickFrom, startPoint);
        setNoRepeatTempoPoint(tickFrom - tickOffset, startPoint, ctx);
    }

    const real_t normalizedTarget = std::clamp(normalizedCurrent * tempoChange->tempoChangeFactor(),
                                               MIN_NORMALIZED_TEMPO, MAX_NORMALIZED_TEMPO);
    const AutomationPoint::ExplicitArrival arrival { normalizedTarget, changeMethodToEase(tempoChange->easingMethod()) };

    // Don't overwrite a tempo already set at tickTo (e.g. by a tempo marking) - just record the
    // arrival so the ramp leading up to it still interpolates correctly
    // See https://github.com/musescore/MuseScore/issues/12140
    const auto tickToIt = ctx.tempoCurve->lower_bound(tickTo);
    if (tickToIt != ctx.tempoCurve->end() && tickToIt->first == tickTo) {
        tickToIt->second.value.inValue = arrival;

        const auto rawIt = ctx.noRepeatTempoCurve.find(tickTo - tickOffset);
        if (rawIt != ctx.noRepeatTempoCurve.end()) {
            rawIt->second.value.inValue = arrival;
        }

        return;
    }

    AutomationPoint endPoint;
    endPoint.value.outValue = normalizedTarget;
    endPoint.value.inValue = arrival;
    endPoint.itemId = itemId;
    endPoint.generated = true;
    ctx.tempoCurve->emplace_hint(tickToIt, tickTo, endPoint);
    setNoRepeatTempoPoint(tickTo - tickOffset, endPoint, ctx);
}

void ScoreAutomationController::addVoltaTempoResetPoint(const Volta* volta, UpdateContext& ctx)
{
    const Measure* startMeasure = volta->startMeasure();
    const Measure* endMeasure = volta->endMeasure();
    if (!startMeasure || !endMeasure || !endMeasure->repeatEnd()) {
        return;
    }

    // Changes noRepeatTempoCurve only, so later content (e.g. a seconda volta) sees the pre-volta tempo, not
    // the volta's own changes; findLessOrEqual so an earlier volta's own reset at this tick counts
    const auto beforeIt = muse::findLessOrEqual(ctx.noRepeatTempoCurve, startMeasure->tick().ticks());
    const bool hasBefore = beforeIt != ctx.noRepeatTempoCurve.end();
    const real_t tempoBeforeVolta = hasBefore ? beforeIt->second.value.outValue : DEFAULT_NORMALIZED_TEMPO;

    AutomationPoint point;
    point.value.outValue = tempoBeforeVolta;
    // Recovers to whatever was active before the volta, so the point belongs to that marking, not the volta
    point.itemId = hasBefore ? beforeIt->second.itemId : std::nullopt;
    point.generated = true;

    // Don't overwrite an explicit tempo marking already sitting at this exact tick
    // (e.g. the seconda volta's own TempoText, written in Step 1 before this Step 2 pass runs)
    ctx.noRepeatTempoCurve.try_emplace(endMeasure->endTick().ticks(), point);
}

void ScoreAutomationController::resolvePendingTempoResets(UpdateContext& ctx)
{
    for (const auto& [utick, rawTick] : ctx.pendingTempoResets) {
        const auto utickIt = ctx.tempoCurve->lower_bound(utick);
        if (utickIt != ctx.tempoCurve->end() && utickIt->first == utick) {
            continue; // this pass's own marking already sits exactly here - don't overwrite it
        }

        const auto rawIt = muse::findLessOrEqual(ctx.noRepeatTempoCurve, rawTick);
        const real_t tempo = rawIt != ctx.noRepeatTempoCurve.end() ? rawIt->second.value.outValue
                             : DEFAULT_NORMALIZED_TEMPO;

        const real_t currentTempo = currentTempoAt(*ctx.tempoCurve, utick);
        if (RealIsEqual(tempo, currentTempo)) {
            continue; // no actual change - don't emit a redundant point
        }

        setTempoPoint(utickIt, utick, utick - rawTick, tempo, ctx);
    }
}

void ScoreAutomationController::fixAnacrusisTempo(UpdateContext& ctx)
{
    // An anacrusis measure with no tempo marking of its own should start at whatever tempo
    // the next (first full) measure marks, rather than whatever tempo preceded it
    for (const AnacrusisMeasureInfo& info : ctx.anacrusisMeasures) {
        const utick_t measureTick = info.measure->tick().ticks() + info.tickOffset;
        const auto measureIt = ctx.tempoCurve->lower_bound(measureTick);
        if (measureIt != ctx.tempoCurve->end() && measureIt->first == measureTick) {
            continue;
        }

        if (!info.nextMeasureUTick) {
            continue; // anacrusis is the last measure of its repeat pass - no "next" within this pass
        }

        const auto nextIt = ctx.tempoCurve->find(*info.nextMeasureUTick);
        if (nextIt != ctx.tempoCurve->end()) {
            setTempoPoint(measureIt, measureTick, info.tickOffset, nextIt->second.value.outValue, ctx);
        }
    }
}

bool ScoreAutomationController::tryAddDynamicPoint(AutomationCurve& curve, std::map<utick_t, int>& tickPrioMap, utick_t tick,
                                                   const AutomationPoint& point, int priority)
{
    //! See: https://github.com/musescore/MuseScore/issues/23355
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

void ScoreAutomationController::addDynamicPoint(AutomationCurve& curve, std::map<utick_t, int>& tickPrioMap, utick_t tick,
                                                const AutomationPoint& point, int priority)
{
    // If a point with the same priority already exists at this tick, merge them:
    // keep the existing arrival value and use this point's departure value
    const auto prioIt = tickPrioMap.find(tick);
    if (prioIt != tickPrioMap.end() && prioIt->second == priority) {
        AutomationPoint& existing = curve[tick];
        const AutomationPoint::InValue arrivalValue = existing.value.inValue;
        existing = point;
        existing.value.inValue = arrivalValue;
        return;
    }

    tryAddDynamicPoint(curve, tickPrioMap, tick, point, priority);
}

void ScoreAutomationController::setNoRepeatTempoPoint(utick_t rawTick, const AutomationPoint& point, UpdateContext& ctx)
{
    const auto it = ctx.noRepeatTempoCurve.lower_bound(rawTick);
    const bool exists = it != ctx.noRepeatTempoCurve.end() && it->first == rawTick;
    if (exists && !it->second.generated) {
        return; // an authored point at this raw tick wins over the generated one
    }

    if (exists) {
        it->second = point;
    } else {
        ctx.noRepeatTempoCurve.emplace_hint(it, rawTick, point);
    }
}

void ScoreAutomationController::setTempoPoint(utick_t tick, int tickOffset, real_t normalizedBps, UpdateContext& ctx,
                                              std::optional<EID> itemId)
{
    const auto it = ctx.tempoCurve->lower_bound(tick);
    const bool exists = it != ctx.tempoCurve->end() && it->first == tick;
    if (exists && !it->second.generated) {
        return; // an authored point at this tick wins over the generated one
    }

    if (!exists) {
        setTempoPoint(it, tick, tickOffset, normalizedBps, ctx, itemId);
        return;
    }

    const AutomationPoint point = makeTempoPoint(normalizedBps, itemId);
    it->second = point;
    setNoRepeatTempoPoint(tick - tickOffset, point, ctx);
}

void ScoreAutomationController::setTempoPoint(AutomationCurve::iterator hint, utick_t tick, int tickOffset, real_t normalizedBps,
                                              UpdateContext& ctx, std::optional<EID> itemId)
{
    const AutomationPoint point = makeTempoPoint(normalizedBps, itemId);
    ctx.tempoCurve->emplace_hint(hint, tick, point);
    setNoRepeatTempoPoint(tick - tickOffset, point, ctx);
}

std::vector<AutomationCurveKey> ScoreAutomationController::resolveKeys(const EngravingItem* item, AutomationType type,
                                                                       const StaffRange& range)
{
    const Score* score = item->score();
    IF_ASSERT_FAILED(score) {
        return {};
    }

    std::vector<AutomationCurveKey> result;

    auto tryAddStaffKey = [type, score, &range, &result](staff_idx_t staffIdx, std::optional<size_t> voiceIdx = std::nullopt) {
        if (!range.contains(staffIdx)) {
            return;
        }

        const Staff* staff = score->staff(staffIdx);
        IF_ASSERT_FAILED(staff) {
            return;
        }

        if (!staff->isPrimaryStaff()) {
            return; // ignore linked staves
        }

        result.push_back(AutomationCurveKey::staff(type, staff->id(), voiceIdx));
    };

    const VoiceAssignment voiceAssignment = item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
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
            tryAddStaffKey(staffIdx);
        }
    } break;
    case VoiceAssignment::CURRENT_VOICE_ONLY:
        tryAddStaffKey(item->staffIdx(), item->voice());
        break;
    case VoiceAssignment::ALL_VOICE_IN_STAFF:
        tryAddStaffKey(item->staffIdx());
        break;
    }

    return result;
}

void ScoreAutomationController::mirrorEditsToRepeats(const AutomationCurveKey& key, AutomationPointEdits& edits)
{
    TRACEFUNC;

    const RepeatList& repeatList = m_score->expandedRepeatList();
    if (repeatList.size() <= 1) {
        return;
    }

    // Measure repeats are staff-specific; global/instrument-scoped points (e.g. Tempo, Volume, Pan)
    // aren't tied to any staff, so they're mirrored to regular repeats only, not measure repeats
    std::optional<StaffRange> staffRange;
    if (const std::optional<muse::ID> staffId = key.staffId()) {
        const Staff* staff = m_score->staffById(*staffId);
        IF_ASSERT_FAILED(staff) {
            return;
        }

        staffRange.emplace(m_score, staff->idx(), staff->idx());
    }

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
            if (staffRange) {
                mirrorToMeasureRepeats(targetSeg, *staffRange, localEdit, edits);
            }
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

    // A MeasureRepeat can only ever sit on the measure's first ChordRest segment
    MeasureRepeats measureRepeats;
    for (const Measure* measure : targetSeg->measureList()) {
        if (const Segment* firstChordRestSegment = measure->first(SegmentType::ChordRest)) {
            collectMeasureRepeats(firstChordRestSegment, tickOffset, range, measureRepeats);
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
