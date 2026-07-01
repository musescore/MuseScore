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

#include "engraving/automation/automationtypes.h"

namespace mu::engraving {
class IAutomation;
class Score;
class Fraction;
class Segment;
class Dynamic;
class Hairpin;
class MeasureRepeat;
class RepeatSegment;
struct ScoreChanges;

class ScoreAutomationController
{
public:
    ScoreAutomationController();
    ~ScoreAutomationController();

    void init(const Score* score);

    void insertTime(const Score* score, const Fraction& tick, const Fraction& len);
    void update(const Score* score, const ScoreChanges& changes);

    IAutomation* automation() const { return m_automation; }

private:
    using DynamicPriorities = std::map<AutomationCurveKey, std::map<utick_t, int> >;
    using DeferredInValuePoints = std::set<std::pair<AutomationCurveKey, utick_t> >;
    using PendingAnchorDynamics = std::vector<std::pair<const Dynamic*, int> >;

    void update(const Score* score, int tickFrom, size_t staffIdxFrom, size_t staffIdxTo);

    void removeGeneratedPoints(const Score* score, const RepeatSegment* seg, int tickFrom, size_t staffIdxFrom, size_t staffIdxTo);

    void addSegmentPoints(const Segment* segment, int tickOffset, size_t staffIdxFrom, size_t staffIdxTo,
                          DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints,
                          PendingAnchorDynamics& pendingAnchorDynamics);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset, size_t staffIdxFrom, size_t staffIdxTo,
                          DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key,
                          DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints);

    void addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset, size_t staffIdxFrom,
                          size_t staffIdxTo, DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints);
    void addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key,
                          DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints);

    void copyVoiceCurves(const Score* score, size_t staffIdxFrom, size_t staffIdxTo);

    void collectMeasureRepeats(const Score* score, const Segment* segment, std::vector<std::pair<const MeasureRepeat*, int> >& result,
                               int tickOffset, size_t staffIdxFrom, size_t staffIdxTo) const;
    void addMeasureRepeatPoints(const std::vector<std::pair<const MeasureRepeat*, int> >& measureRepeats,
                                DynamicPriorities& dynamicPriorities);

    bool tryAddDynamicPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point, int priority,
                            DynamicPriorities& dynamicPriorities);
    void addDeferredPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& point, int priority,
                          DynamicPriorities& dynamicPriorities, DeferredInValuePoints& deferredInValuePoints);

    void resolveDeferredInValues(DeferredInValuePoints& deferredInValuePoints);

    IAutomation* m_automation = nullptr;
};
}
