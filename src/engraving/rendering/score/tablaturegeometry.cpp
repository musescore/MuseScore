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

#include "tablaturegeometry.h"

#include <cmath>
#include <algorithm>

using namespace muse;

namespace mu::engraving::rendering::score {
std::vector<ArcRange> computeVisibleArcRanges(
    const RectF& mainRect,
    const std::vector<RectF>& adjacentRects,
    double lineWidth)
{
    std::vector<ArcRange> result;

    if (mainRect.width() <= 0.0 || mainRect.height() <= 0.0) {
        return result;
    }

    const double cx = mainRect.x() + mainRect.width() / 2.0;
    const double cy = mainRect.y() + mainRect.height() / 2.0;
    const double a = mainRect.width() / 2.0;
    const double b = mainRect.height() / 2.0;

    const double inflation = lineWidth * 0.25;

    struct Blocker {
        double cx, cy, a, b;
    };
    std::vector<Blocker> blockers;
    for (const auto& adj : adjacentRects) {
        if (adj.width() > 0.0 && adj.height() > 0.0) {
            blockers.push_back({
                    adj.x() + adj.width() / 2.0,
                    adj.y() + adj.height() / 2.0,
                    adj.width() / 2.0 + inflation,
                    adj.height() / 2.0 + inflation
                });
        }
    }

    if (blockers.empty()) {
        result.push_back({ 0.0, 2.0 * M_PI });
        return result;
    }

    const int N = std::clamp(static_cast<int>(std::max(a, b) * 4), 180, 720);
    const double dt = 2.0 * M_PI / N;

    auto isVisible = [&](double t) -> bool {
        double px = cx + a * std::cos(t);
        double py = cy - b * std::sin(t);  // Y negated for screen coordinates
        for (const auto& bl : blockers) {
            double dx = (px - bl.cx) / bl.a;
            double dy = (py - bl.cy) / bl.b;
            if (dx * dx + dy * dy <= 1.0) {
                return false;
            }
        }
        return true;
    };

    bool inArc = false;
    double arcStart = 0.0;

    for (int i = 0; i <= N; ++i) {
        double t = (i < N) ? dt * i : 0.0;
        bool vis = isVisible(t);

        if (vis && !inArc) {
            arcStart = t;
            inArc = true;
        } else if (!vis && inArc) {
            result.push_back({ arcStart, dt * i });
            inArc = false;
        }
    }

    if (inArc) {
        double arcEnd = 2.0 * M_PI;
        if (isVisible(0.0)) {
            for (int i = 0; i < N; ++i) {
                if (!isVisible(dt * i)) {
                    arcEnd = 2.0 * M_PI + dt * i;
                    break;
                }
            }
        }
        result.push_back({ arcStart, arcEnd });
    }

    return result;
}
}
