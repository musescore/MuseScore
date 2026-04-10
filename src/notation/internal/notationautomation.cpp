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

    if (enabled) {
        // TODO: Optimization - we don't need to init all the lines every time. Only the bits that
        // changed since the last time automation was enabled...
        initAutomationLinesData();
    }

    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

QVariant NotationAutomation::automationLinesData() const
{
    return m_automationLinesData;
}

void NotationAutomation::initAutomationLinesData()
{
    mu::engraving::MasterScore* masterScore = score() ? score()->masterScore() : nullptr;
    IF_ASSERT_FAILED(masterScore) {
        return;
    }

    if (m_automationLinesData.isEmpty()) {
        // TODO: Placeholder - init this elsewhere. Also note that automation data doesn't currently
        // update through score interactions (such as deleting dynamics, etc)...
        masterScore->initAutomation();
    }

    QVariantList automationLinesData;
    for (const System* system : score()->systems()) {
        const QVariantList linesData = linesDataForSystem(system);
        if (!linesData.empty()) {
            automationLinesData << linesData;
        }
    }
    m_automationLinesData = automationLinesData;
    m_automationLinesDataChanged.notify();
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
        lineData["staffIdx"] = static_cast<int>(staffIdx);
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
    const auto addPoint = [&points](double x, double y, int tick, PointType pointType) {
        QVariantMap pointData;
        pointData["x"] = x;
        pointData["y"] = y;
        pointData["tick"] = tick;
        pointData["pointType"] = static_cast<int>(pointType);
        points << pointData;
    };

    IF_ASSERT_FAILED(automation()) {
        return points;
    }

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

        // Point in/out values are always between 0 and 1 - higher value == lower Y...
        const mu::engraving::AutomationPoint& autoPoint = point.second;
        if (muse::RealIsEqual(autoPoint.inValue, autoPoint.outValue)) {
            addPoint(pointX, 1 - autoPoint.inValue, tick, PointType::BOTH);
            continue;
        }
        addPoint(pointX, 1 - autoPoint.inValue, tick, PointType::IN);
        addPoint(pointX, 1 - autoPoint.outValue, tick, PointType::OUT);
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

void NotationAutomation::requestChangeAutomationPoint(qsizetype lineIdx, qsizetype pointIdx, qreal x, qreal y)
{
    IF_ASSERT_FAILED(automation() && lineIdx < m_automationLinesData.size()) {
        return;
    }

    const QVariantMap lineData = m_automationLinesData.at(lineIdx).toMap();
    IF_ASSERT_FAILED(!lineData.isEmpty()) {
        return;
    }

    bool ok;
    const staff_idx_t staffIdx = static_cast<staff_idx_t>(lineData.value("staffIdx").toInt(&ok));
    IF_ASSERT_FAILED(ok && staffIdx != muse::nidx) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    const QVariantList pointsList = lineData.value("points").toList();
    IF_ASSERT_FAILED(staff && !pointsList.isEmpty() && pointIdx < pointsList.size()) {
        return;
    }

    const QVariantMap point = pointsList.at(pointIdx).toMap();
    IF_ASSERT_FAILED(!point.isEmpty()) {
        return;
    }

    const int tick = point.value("tick").toInt(&ok);
    const int pointTypeRaw = ok ? point.value("pointType").toInt(&ok) : 0;
    IF_ASSERT_FAILED(ok) {
        return;
    }

    const PointType pointType = static_cast<PointType>(pointTypeRaw);
    const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staff->id(), std::nullopt };

    // Point in/out values are always between 0 and 1 - higher value == lower Y...
    if (pointType == PointType::IN || pointType == PointType::BOTH) {
        automation()->setPointInValue(key, tick, 1 - y);
    }
    if (pointType == PointType::OUT || pointType == PointType::BOTH) {
        automation()->setPointOutValue(key, tick, 1 - y);
    }

    UNUSED(x);
    // TODO: use x to calculate newTick...
    // automation()->movePoint(key, tick, newTick) = 0;
}
