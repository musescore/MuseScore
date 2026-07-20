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
#include <set>
#include <vector>

#include "engraving/automation/automationdata.h"
#include "engraving/automation/automationtypes.h"
#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
class Fraction;
class EngravingItem;
class Segment;
class Dynamic;
class Hairpin;
class MeasureRepeat;
class RepeatSegment;
struct ScoreChanges;

class ScoreAutomationController
{
public:
    void init(const Score* score);

    void insertTime(const Fraction& tick, const Fraction& len);
    void update(const ScoreChanges& changes);

    AutomationDataConstPtr automationData() const { return m_automationData; }
    void setAutomationData(AutomationDataPtr data);

    void editPoints(const AutomationCurveKey& key, AutomationPointEdits& edits);

private:
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

    using DynamicPriorities = std::map<AutomationCurveKey, std::map<utick_t, int> >;
    using MeasureRepeats = std::vector<std::pair<const MeasureRepeat*, int> >;

    struct UpdateContext {
        AutomationCurveMap curves;
        utick_t clearFromUTick = 0;
        DynamicPriorities dynamicPriorities;
        MeasureRepeats measureRepeats;
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

    void update(int tickFrom, staff_idx_t staffIdxFrom, staff_idx_t staffIdxTo);

    static void moveTicks(utick_t tickFrom, utick_t diff, AutomationCurveMap& curves);
    static void removeTicks(utick_t tickFrom, utick_t tickTo, AutomationCurveMap& curves);

    static void copyCurvesForRebuild(const AutomationCurveMap& curves, const StaffRange& range, utick_t clearFromUTick,
                                     AutomationCurveMap& destCurves);

    static void addSegmentPoints(const Segment* segment, int tickOffset, const StaffRange& range, UpdateContext& ctx);
    static void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const StaffRange& range, UpdateContext& ctx);
    static void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key, UpdateContext& ctx);

    static void addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset, const StaffRange& range,
                                 UpdateContext& ctx);
    static void addHairpinPoints(const Hairpin* hairpin, int tickOffset, const std::vector<AutomationCurveKey>& keys, UpdateContext& ctx);
    static void addHairpinPoints(const HairpinInfo& info, const AutomationCurveKey& key, UpdateContext& ctx);

    static void fillVoiceCurvesFromBase(UpdateContext& ctx);

    static void collectMeasureRepeats(const Segment* segment, int tickOffset, const StaffRange& range, MeasureRepeats& result);
    static void addMeasureRepeatPoints(UpdateContext& ctx);

    static bool tryAddDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point, int priority,
                                   UpdateContext& ctx);
    static void addDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point, int priority,
                                UpdateContext& ctx);
    static void tryAddStaffKey(const Score* score, staff_idx_t staffIdx, const StaffRange& range, AutomationCurveKey key,
                               std::vector<AutomationCurveKey>& result);
    static std::vector<AutomationCurveKey> resolveKeys(const EngravingItem* item, AutomationType type, const StaffRange& range);

    void mirrorEditsToRepeats(const AutomationCurveKey& key, AutomationPointEdits& edits);

    static void mirrorPointIfInRange(int localTick, const std::optional<int>& localMoveFrom, const AutomationPoint& point,
                                     const MirrorRange& range, AutomationPointEdits& allEdits);

    static void mirrorToMeasureRepeats(const RepeatSegment* targetSeg, const StaffRange& range, int localTick,
                                       const std::optional<int>& localMoveFrom, const AutomationPoint& point,
                                       AutomationPointEdits& allEdits);

    const Score* m_score = nullptr;
    AutomationDataPtr m_automationData;
};
}
