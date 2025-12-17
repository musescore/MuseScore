/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
class Segment;
class Dynamic;
struct AutomationCurveKey;

class AutomationController
{
public:
    AutomationController();
    ~AutomationController();

    void init(Score* score);

    IAutomation* automation() const { return m_automation; }

private:
    void addSegmentPoints(const Segment* segment, int tickOffset);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset);
    void addDynamicPoints(const Dynamic* dynamic, int tickOffset, const AutomationCurveKey& key);

    IAutomation* m_automation = nullptr;
};
}
