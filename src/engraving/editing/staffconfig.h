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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "../dom/staff.h"
#include "../dom/stafftype.h"

namespace mu::engraving {
struct StaffConfig
{
    bool visible = false;
    Spatium userDistance = Spatium(0.0);
    bool cutaway = false;
    bool hideSystemBarline = false;
    AutoOnOff mergeMatchingRests = AutoOnOff::AUTO;
    bool reflectTranspositionInLinkedTab = false;
    ClefTypeList clefTypeList;
    StaffType staffType;

    bool operator==(const StaffConfig& conf) const
    {
        bool equal = visible == conf.visible;
        equal &= userDistance == conf.userDistance;
        equal &= cutaway == conf.cutaway;
        equal &= hideSystemBarline == conf.hideSystemBarline;
        equal &= mergeMatchingRests == conf.mergeMatchingRests;
        equal &= clefTypeList == conf.clefTypeList;
        equal &= staffType == conf.staffType;
        equal &= reflectTranspositionInLinkedTab == conf.reflectTranspositionInLinkedTab;

        return equal;
    }

    bool operator!=(const StaffConfig& conf) const
    {
        return !(*this == conf);
    }
};
}
