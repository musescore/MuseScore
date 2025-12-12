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

#include "engraving/automation/iautomation.h"

namespace mu::engraving {
class Score;
class Segment;
class Dynamic;

class Automation : public IAutomation
{
public:
    void init(Score* score);

    const AutomationCurve& curve(const AutomationCurveKey& key) const override;

    void addPoint(const AutomationCurveKey& key, int utick, const AutomationPoint& p) override;
    void removePoint(const AutomationCurveKey& key, int utick) override;
    void movePoint(const AutomationCurveKey& key, int srcUtick, int dstUtick) override;

    void setPointInValue(const AutomationCurveKey& key, int utick, double value) override;
    void setPointOutValue(const AutomationCurveKey& key, int utick, double value) override;

    void moveTicks(int utickFrom, int diff) override;
    void removeTicks(int utickFrom, int utickTo) override;

    void read(const muse::ByteArray& json) override;
    muse::ByteArray toJson() const override;

private:
    void handleSegmentAnnotations(const Segment* segment, int tickOffset);
    void handleDynamic(const Dynamic* dynamic, const Segment* segment, int tickOffset);

    const AutomationPoint& activePoint(const AutomationCurveKey& key, int utick) const;

    std::map<AutomationCurveKey, AutomationCurve> m_curveMap;
};
}
