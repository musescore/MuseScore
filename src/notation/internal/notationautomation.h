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
    muse::async::Notification automationLinesDataChanged() const override; // TODO: probably a channel specifying indices

    void requestChangeAutomationPoint(qsizetype lineIdx, qsizetype pointIdx, qreal x, qreal y) override;

private:
    QVariantList linesDataForSystem(const System* system) const;
    QVariantList linesDataForSysStaff(const Staff* staff, const muse::RectF& sysStaffCanvasRect, int startTick, int endTick) const;

    mu::engraving::Score* score() const;
    mu::engraving::IAutomation* automation() const;

    IGetScore* m_getScore = nullptr;
    muse::async::Channel<muse::RectF> m_notationChanged;

    bool m_isAutomationModeEnabled = false;
    muse::async::Notification m_automationModeEnabledChanged;

    muse::async::Notification m_automationLinesDataChanged;
};
}
