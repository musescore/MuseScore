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
#pragma once

#include "../inotationautomation.h"

#include "notationtypes.h"
#include "engraving/automation/automationtypes.h"

#include "async/channel.h"

#include "igetscore.h"
#include "draw/types/geometry.h"

namespace mu::engraving {
class IAutomation;
}

namespace mu::notation {
class NotationAutomation : public INotationAutomation
{
public:
    NotationAutomation(IGetScore* getScore, muse::async::Channel<muse::RectF> notationChanged);

    bool isAutomationModeEnabled() const override;
    void setAutomationModeEnabled(bool enabled) override;
    muse::async::Notification automationModeEnabledChanged() const override;

    QVariant automationLinesData() const override;
    muse::async::Notification automationLinesDataChanged() const override;
    void refreshAutomationView() override;

    void requestChangeAutomationPoint(qsizetype lineIdx, qsizetype pointIdx, qreal x, qreal y) override;
    void requestAddAutomationPoint(qsizetype lineIdx, qreal x, qreal y) override;
    void requestRemoveAutomationPoint(qsizetype lineIdx, qsizetype pointIdx) override;

private:
    enum class PointType : unsigned char {
        UNKNOWN = 0,
        IN,
        OUT,
        BOTH
    };

    void initAutomationLinesData();

    QVariantList linesDataForSystem(const System* system, size_t systemIdx) const;
    QVariantMap lineDataForSysStaff(staff_idx_t staffIdx, size_t systemIdx, const System* system) const;
    QVariantList pointsDataForSysStaff(const Staff* staff, const System* system, const muse::RectF& sysStaffCanvasRect,
                                       int startTick, int endTick) const;
    int tickForLineX(const QVariantMap& lineData, staff_idx_t staffIdx, qreal x, bool* ok);
    bool refreshAutomationLinesForStaff(staff_idx_t staffIdx);
    void notifyAutomationChanged(staff_idx_t staffIdx);

    mu::engraving::Score* score() const;
    mu::engraving::IAutomation* automation() const;

    IGetScore* m_getScore = nullptr;
    muse::async::Channel<muse::RectF> m_notationChanged;

    bool m_isAutomationModeEnabled = false;
    muse::async::Notification m_automationModeEnabledChanged;

    QVariantList m_automationLinesData;
    muse::async::Notification m_automationLinesDataChanged;
};
}
