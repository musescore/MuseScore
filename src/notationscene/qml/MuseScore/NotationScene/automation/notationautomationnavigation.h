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
#include <vector>

#include "notation/notationtypes.h"

#include "notationautomationtypes.h"

namespace mu::notation {
class NotationAutomationNavigation
{
public:
    struct Target {
        PolylineKey key;
        int pointIdx = -1;
    };

    //! NOTE: selection must be ordered by position: by polyline (in the order of `polylines`), then by point index. With an empty
    //! selection, every direction targets the first point of the first polyline that has points...
    static std::optional<Target> findTarget(const PolylineMap& polylines, const std::vector<Target>& selection, MoveDirection direction,
                                            bool toTopBottom);

private:
    struct TickRange {
        int from = 0;
        int to = 0;
    };

    static bool hasNavigablePoints(const PolylineData& data);
    static std::optional<Target> firstNavigablePoint(const PolylineMap& polylines);
    static std::optional<Target> stepAlongStaff(const PolylineMap& polylines, const Target& from, bool forwards);
    static std::optional<Target> closestPointInPolyline(const PolylineMap& polylines, const PolylineKey& key, const TickRange& range);
    static std::optional<PolylineKey> nextInColumn(const PolylineMap& polylines, const PolylineKey& from, bool down);
    static std::optional<Target> stepAcrossStaves(const PolylineMap& polylines, const std::vector<Target>& selection, bool down,
                                                  bool toTopBottom);
};
}
