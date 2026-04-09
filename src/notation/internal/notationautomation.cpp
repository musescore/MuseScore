/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "notationautomation.h"
#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"

using namespace mu::notation;

NotationAutomation::NotationAutomation(IGetScore* getScore,
                                       muse::async::Channel<muse::RectF> notationChanged)
    : m_getScore(getScore), m_notationChanged(notationChanged)
{
}

bool NotationAutomation::isAutomationModeEnabled() const
{
    return m_isAutomationModeEnabled;
}

void NotationAutomation::setAutomationModeEnabled(bool enabled)
{
    if (m_isAutomationModeEnabled == enabled) {
        return;
    }

    if (enabled) { // TODO: Placeholder - need to init this somewhere...
        if (score() && score()->masterScore()) {
            score()->masterScore()->initAutomation();
        } else {
            ASSERT_X("No score for automation...")
        }
    }

    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
    m_automationLinesDataChanged.notify(); // TODO: Delete once above placeholder has been removed...
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

QVariant NotationAutomation::automationLinesData() const
{
    QVariantList automationLinesData;

    IF_ASSERT_FAILED(automation()) {
        return automationLinesData;
    }

    for (const System* system : score()->systems()) {
        const QVariantList linesData = linesDataForSystem(system);
        if (!linesData.empty()) {
            automationLinesData << linesData;
        }
    }

    return automationLinesData;
}

QVariantList NotationAutomation::linesDataForSystem(const System* system) const
{
    QVariantList lines;

    const int systemStartTick = system->first()->tick().ticks();
    const int systemEndTick = system->last()->endTick().ticks();

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        const Staff* staff = score()->staff(staffIdx);
        const SysStaff* sysStaff = system->staff(staffIdx);
        IF_ASSERT_FAILED(staff && sysStaff) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        if (!staff->isPrimaryStaff()) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
        const QVariantList staffLinesData = linesDataForSysStaff(staff, staffCanvasRect, systemStartTick, systemEndTick);
        if (staffLinesData.isEmpty()) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        QVariantMap lineData;
        lineData["x"] = staffCanvasRect.x();
        lineData["y"] = staffCanvasRect.y();
        lineData["width"] = staffCanvasRect.width();
        lineData["height"] = staffCanvasRect.height();
        lineData["points"] = staffLinesData;

        lines << lineData;

        staffIdx = system->nextVisibleStaff(staffIdx);
    }

    return lines;
}

QVariantList NotationAutomation::linesDataForSysStaff(const Staff* staff, const muse::RectF& sysStaffCanvasRect,
                                                      int startTick, int endTick) const
{
    QVariantList points;
    const auto addPoint = [&points](double x, double y) {
        QVariantMap pointData;
        pointData["x"] = x;
        pointData["y"] = y;
        points << pointData;
    };

    const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staff->id(), std::nullopt };
    for (auto& point : automation()->curve(key)) {
        const int tick = point.first;
        if (tick < startTick || tick > endTick) {
            continue;
        }

        const Fraction frac = Fraction::fromTicks(tick);
        const Segment* seg = score()->tick2segmentMM(frac);
        IF_ASSERT_FAILED(seg) {
            continue;
        }

        // TODO: The following won't work for dynamics at time ticks...

        // x positions scaled between 0 and 1 (where 0 is the staff start and 1 is the staff end)
        const double pointX = (seg->canvasX() - sysStaffCanvasRect.x()) / sysStaffCanvasRect.width();

        const mu::engraving::AutomationPoint& autoPoint = point.second;
        if (!muse::RealIsEqual(autoPoint.inValue, autoPoint.outValue)) {
            // inValue is always between 0 and 1 - higher value == lower Y...
            addPoint(pointX, 1 - autoPoint.inValue);
        }

        // outValue is always between 0 and 1 - higher value == lower Y...
        addPoint(pointX, 1 - autoPoint.outValue);
    }

    return points;
}

muse::async::Notification NotationAutomation::automationLinesDataChanged() const
{
    return m_automationLinesDataChanged;
}

mu::engraving::Score* NotationAutomation::score() const
{
    return m_getScore ? m_getScore->score() : nullptr;
}

mu::engraving::IAutomation* NotationAutomation::automation() const
{
    return score() ? score()->automation() : nullptr;
}
