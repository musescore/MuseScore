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
#include "notationtypes.h"

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

    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

QVariant NotationAutomation::automationLinesData() const
{
    // TODO: Entire method is a dummy - here we'll construct from actual automation data...
    QVariantList dummyAutomationData;

    // Hack - using second system because first is normally the title...
    const System* systemOne = m_getScore ? m_getScore->score()->systems().at(1) : nullptr;
    if (!systemOne) {
        return QVariantList();
    }

    const muse::PointF systemOnePos = systemOne->canvasPos();
    for (const SysStaff* staff : systemOne->staves()) {
        QVariantMap lineData;

        const muse::RectF staffRect = staff->bbox().translated(systemOnePos);

        lineData["x"] = staffRect.x();
        lineData["y"] = staffRect.y();
        lineData["width"] = staffRect.width();
        lineData["height"] = staffRect.height();

        QVariantList points;

        // Dummy points...
        QVariantMap point1;
        point1["x"] = 0.10;
        point1["y"] = 0.50;
        points << point1;

        QVariantMap point2;
        point2["x"] = 0.50;
        point2["y"] = 0.10;
        points << point2;

        QVariantMap point3;
        point3["x"] = 0.66;
        point3["y"] = 0.90;
        points << point3;

        lineData["points"] = points;

        dummyAutomationData << lineData;
    }

    return dummyAutomationData;
}

muse::async::Notification NotationAutomation::automationLinesDataChanged() const
{
    return m_automationLinesDataChanged;
}
