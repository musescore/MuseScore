/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <vector>

namespace muse::interpolation {
struct Point {
    double x = 0.0;
    double y = 0.0;
};

inline std::vector<Point> lerp(const Point& start, const Point& end, std::size_t steps)
{
    if (steps == 0) {
        return {};
    }

    std::vector<Point> result;
    result.reserve(steps + 1);

    const double dx = end.x - start.x;
    const double dy = end.y - start.y;

    for (std::size_t i = 0; i <= steps; ++i) {
        const double t = static_cast<double>(i) / steps;

        result.emplace_back(Point {
                start.x + dx * t,
                start.y + dy * t
            });
    }

    return result;
}
}
