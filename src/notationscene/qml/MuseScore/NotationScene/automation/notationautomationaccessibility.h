/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <optional>

#include <QString>

#include "global/types/number.h"

#include "engraving/automation/automationtypes.h"

#include "notation/notationtypes.h"
#include "notation/types/automationlane.h"

#include "notationautomationtypes.h"

namespace mu::engraving {
class Score;
class Staff;
}

namespace mu::notation {
class NotationAutomationAccessibility
{
public:
    struct PointInfo {
        AutomationLane lane = AutomationLane::Dynamics;
        mu::engraving::AutomationCurveKey curveKey;
        const mu::engraving::Staff* staff = nullptr;
        int tick = 0;
        PointData::PointType pointType = PointData::PointType::BOTH;
        std::optional<muse::real_t> value;
    };

    //! NOTE: As with selected score elements, the staff/instrument is only spoken when asked for (i.e. when it's
    //! different from that of the previous point), so that stepping along a staff doesn't repeat it every time
    static QString pointDescription(const PointInfo& info, bool includeScope);

    static QString noTargetDescription(MoveDirection direction, bool hadSelection);

private:
    static QString laneTitle(AutomationLane lane);
    static QString scopeText(const PointInfo& info);
    static QString barsAndBeatsText(const mu::engraving::Score* score, int tick);
};
}
