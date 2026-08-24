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
#pragma once

#include <map>
#include <optional>
#include <set>
#include <variant>
#include <vector>

#include "engraving/automation/automationdata.h"
#include "engraving/automation/automationtypes.h"
#include "engraving/dom/tempotimeline.h"
#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
class Fraction;
class EngravingItem;
class Measure;
class Segment;
class Dynamic;
class Fermata;
class Hairpin;
class TempoText;
class GradualTempoChange;
class Volta;
class MeasureRepeat;
class RepeatList;
class RepeatSegment;
struct RepeatSegmentInfo;
struct ScoreChanges;

class ScoreAutomationController
{
public:
    void init(Score* score);
    void ensureInitialized(Score* score);

    void insertTime(const Fraction& tick, const Fraction& len, const std::vector<RepeatSegmentInfo>& oldSegments);
    void update(const ScoreChanges& changes);

    void editPoints(const AutomationCurveKey& key, AutomationPointEdits& edits);

    AutomationDataConstPtr automationData() const { return m_automationData; }
    void setAutomationData(AutomationDataPtr data) { m_automationData = std::move(data); }

    const TempoTimeline& tempoTimeline(bool expandRepeats = true) const;
    void setTempoMultiplier(const BeatsPerSecond& bps);

    void setTempoTimelineOverride(std::optional<TempoTimeline> timeline);

private:
    struct UpdateRequest {
        int tickFrom = 0;
        staff_idx_t staffIdxFrom = muse::nidx;
        staff_idx_t staffIdxTo = muse::nidx;
        bool includeTempo = false;
        bool includeDynamics = false;
    };

    struct StaffRange {
        StaffRange(const Score* score, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo);

        staff_idx_t from = 0;
        staff_idx_t to = 0;
        bool isFull = true;
        std::set<muse::ID> staffIds; // only populated when !isFull

        bool contains(staff_idx_t staffIdx) const;
        bool contains(const muse::ID& staffId) const;
    };

    struct MirrorRange {
        int from = 0;
        int toExclusive = 0;
        int tickOffset = 0;
    };

    struct AnacrusisMeasureInfo {
        const Measure* measure = nullptr;
        int tickOffset = 0;
        std::optional<utick_t> nextMeasureUTick; // next measure in the same repeat pass, if any
    };

    using DynamicPriorities = std::map<AutomationCurveKey, std::map<utick_t, int> >;
    using MeasureRepeats = std::vector<std::pair<const MeasureRepeat*, int> >;

    struct UpdateContext {
        UpdateRequest request;
        AutomationCurveMap curves;
        utick_t clearFromUTick = 0;
        DynamicPriorities dynamicPriorities;
        MeasureRepeats measureRepeats;
        std::optional<BeatsPerSecond> tempoPrimo;
        PausesMap pauses;
        PausesMap noRepeatPauses; // repeat-agnostic twin of pauses
        std::vector<AnacrusisMeasureInfo> anacrusisMeasures;
        AutomationCurve* tempoCurve = nullptr;
        AutomationCurve noRepeatTempoCurve; // repeat-agnostic Tempo curve
        std::vector<std::pair<utick_t, int> > pendingTempoResets;
        BeatsPerSecond tempoMultiplier = 1.0;
    };

    struct DynamicInfo {
        struct Ordinary {
            real_t value = 0;
        };
        struct SingleNote {
            real_t value = 0;
            std::optional<utick_t> nextTick; // tick of the next segment, if any (recovery point)
        };
        struct Compound {
            real_t startValue = 0;
            real_t endValue = 0;
            utick_t endPointTick = 0; // tick + velocityChangeLength (arrival point)
        };

        utick_t tick = 0;
        EID eid = EID::invalid();
        int priority = 0;
        std::variant<Ordinary, SingleNote, Compound> kind = Ordinary {};
    };

    struct HairpinInfo {
        utick_t from = 0;
        utick_t to = 0;
        EID eid = EID::invalid();
        int priority = 0;
        bool isCrescendo = false;
        std::optional<real_t> nominalValueFrom;
        std::optional<real_t> nominalValueTo;
    };

    void update(const UpdateRequest& request, const AutomationCurveMap& curves);
    void fullRebuild(AutomationCurveMap& curves, bool includeTempo = true, bool includeDynamics = true);

    static void copyCurvesForRebuild(const AutomationCurveMap& curves, const StaffRange& range, utick_t clearFromUTick, bool includeTempo,
                                     bool includeDynamics, AutomationCurveMap& destCurves);

    static void addSegmentPoints(const Segment* segment, int tickOffset, const StaffRange& range, UpdateContext& ctx);
    static void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const StaffRange& range, UpdateContext& ctx);
    static void addDynamicPoints(const DynamicInfo& info, const AutomationCurveKey& key, UpdateContext& ctx);

    static void addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset, const StaffRange& range,
                                 UpdateContext& ctx);
    static void addHairpinPoints(const Hairpin* hairpin, int tickOffset, const std::vector<AutomationCurveKey>& keys, UpdateContext& ctx);
    static void addHairpinPoints(const HairpinInfo& info, const AutomationCurveKey& key, UpdateContext& ctx);

    static void fillVoiceCurvesFromBase(UpdateContext& ctx);

    static void collectMeasureRepeats(const Segment* segment, int tickOffset, const StaffRange& range, MeasureRepeats& result);
    static void addMeasureRepeatPoints(UpdateContext& ctx);

    void mirrorAuthoredPointsToRepeats(UpdateContext& ctx);
    void fillNoRepeatTempoCurve(const AutomationCurve& tempoCurve, AutomationCurve& noRepeatTempoCurve);

    static void collectPauses(const Measure* measure, int tickOffset, PausesMap& pauses, PausesMap& noRepeatPauses);
    static void addTempoTextPoint(const TempoText* tt, int tickOffset, UpdateContext& ctx);
    static void addFermataStretchPoints(const Fermata* fermata, int tickOffset, double stretch, UpdateContext& ctx);
    static void addGradualTempoChangePoints(const GradualTempoChange* tempoChange, int tickOffset, UpdateContext& ctx);
    static void addVoltaTempoResetPoint(const Volta* volta, UpdateContext& ctx);
    static void resolvePendingTempoResets(UpdateContext& ctx);
    static void fixAnacrusisTempo(UpdateContext& ctx);

    static bool tryAddDynamicPoint(AutomationCurve& curve, std::map<utick_t, int>& tickPrioMap, utick_t tick, const AutomationPoint& point,
                                   int priority);
    static void addDynamicPoint(AutomationCurve& curve, std::map<utick_t, int>& tickPrioMap, utick_t tick, const AutomationPoint& point,
                                int priority);
    static void setTempoPoint(utick_t tick, int tickOffset, real_t normalizedBps, UpdateContext& ctx,
                              std::optional<EID> itemId = std::nullopt);
    //! NOTE: for callers that already know no point exists at tick (e.g. via lower_bound) - skips the redundant lookup
    static void setTempoPoint(AutomationCurve::iterator hint, utick_t tick, int tickOffset, real_t normalizedBps, UpdateContext& ctx,
                              std::optional<EID> itemId = std::nullopt);
    static void setNoRepeatTempoPoint(utick_t rawTick, const AutomationPoint& point, UpdateContext& ctx);

    static std::vector<AutomationCurveKey> resolveKeys(const EngravingItem* item, AutomationType type, const StaffRange& range);

    void mirrorEditsToRepeats(const AutomationCurveKey& key, AutomationPointEdits& edits);

    static void mirrorPointIfInRange(const AutomationPointEdit& localEdit, const MirrorRange& range, AutomationPointEdits& allEdits);

    static void mirrorToMeasureRepeats(const RepeatSegment* targetSeg, const StaffRange& range, const AutomationPointEdit& localEdit,
                                       AutomationPointEdits& allEdits);

    Score* m_score = nullptr;
    AutomationDataPtr m_automationData;
    TempoTimeline m_tempoTimeline;
    TempoTimeline m_flattenedTempoTimeline;
    std::optional<TempoTimeline> m_tempoTimelineOverride;
};
}
