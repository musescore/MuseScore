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

namespace mu::engraving {
class IAutomation;
class Score;
class Fraction;
class Segment;
class Dynamic;
class Hairpin;
class RepeatSegment;
struct AutomationCurveKey;
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
    void update(const Score* score, int tickFrom, size_t staffIdxFrom, size_t staffIdxTo);

    void removeGeneratedPoints(const Score* score, const RepeatSegment* seg, int tickFrom, size_t staffIdxFrom, size_t staffIdxTo);

    void addSegmentPoints(const Segment* segment, int tickOffset, size_t staffIdxFrom, size_t staffIdxTo);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset, size_t staffIdxFrom, size_t staffIdxTo);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key);

    void addSpannerPoints(const Score* score, int repeatStartTick, int repeatEndTick, int tickOffset, size_t staffIdxFrom,
                          size_t staffIdxTo);
    void addHairpinPoints(const Hairpin* hairpin, int tickOffset, const AutomationCurveKey& key);

    IAutomation* m_automation = nullptr;
};
}
