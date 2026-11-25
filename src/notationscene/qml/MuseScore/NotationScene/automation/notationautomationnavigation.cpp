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

#include "notationautomationnavigation.h"

#include <algorithm>
#include <limits>

using namespace mu::notation;

bool NotationAutomationNavigation::hasNavigablePoints(const PolylineData& data)
{
    return data.polyline && !data.pointsData.isEmpty();
}

std::optional<NotationAutomationNavigation::Target> NotationAutomationNavigation::firstNavigablePoint(const PolylineMap& polylines)
{
    for (const auto& [key, data] : polylines) {
        if (hasNavigablePoints(data)) {
            return Target { key, 0 };
        }
    }
    return std::nullopt;
}

std::optional<NotationAutomationNavigation::Target> NotationAutomationNavigation::stepAlongStaff(const PolylineMap& polylines,
                                                                                                 const Target& from, bool forwards)
{
    // Step within the same polyline...
    const int idx = from.pointIdx + (forwards ? 1 : -1);
    if (idx >= 0 && idx < polylines.at(from.key).pointsData.size()) {
        return Target { from.key, idx };
    }

    if (forwards) {
        // At the end of the polyline, go to the first point of the next polyline on this staff...
        bool passedCurrent = false;
        for (const auto& [key, data] : polylines) {
            if (key == from.key) {
                passedCurrent = true;
            } else if (passedCurrent && key.staffIdx == from.key.staffIdx && hasNavigablePoints(data)) {
                return Target { key, 0 };
            }
        }

        return std::nullopt;
    }

    // At the start of the polyline, go to the last point of the previous polyline on this staff...
    std::optional<Target> previous;
    for (const auto& [key, data] : polylines) {
        if (key == from.key) {
            return previous;
        }
        if (key.staffIdx == from.key.staffIdx && hasNavigablePoints(data)) {
            previous.emplace(key, data.pointsData.size() - 1);
        }
    }

    return std::nullopt;
}

//! NOTE: A point of the polyline whose tick is nearest to the range (the first of them if several are equally near)
std::optional<NotationAutomationNavigation::Target> NotationAutomationNavigation::closestPointInPolyline(
    const PolylineMap& polylines, const PolylineKey& key, const TickRange& range)
{
    const QVector<PointData>& pointsData = polylines.at(key).pointsData;

    std::optional<Target> closest;
    int closestDistance = std::numeric_limits<int>::max();
    for (int i = 0; i < pointsData.size(); ++i) {
        const int tick = pointsData.at(i).tick;
        const int distance = std::max({ 0, range.from - tick, tick - range.to });
        if (distance < closestDistance) {
            closestDistance = distance;
            closest.emplace(key, i);
        }
    }

    return closest;
}

std::optional<PolylineKey> NotationAutomationNavigation::nextInColumn(const PolylineMap& polylines, const PolylineKey& from, bool down)
{
    if (down) {
        // Return the first matching polyline after from...
        bool passedCurrent = false;
        for (const auto& [key, data] : polylines) {
            if (key == from) {
                passedCurrent = true;
            } else if (passedCurrent && key.system == from.system && key.startTick == from.startTick && hasNavigablePoints(data)) {
                return key;
            }
        }

        return std::nullopt;
    }

    // Return the last matching polyline before from...
    std::optional<PolylineKey> previous;
    for (const auto& [key, data] : polylines) {
        if (key == from) {
            return previous;
        }

        if (key.system == from.system && key.startTick == from.startTick && hasNavigablePoints(data)) {
            previous.emplace(key);
        }
    }

    return std::nullopt;
}

std::optional<NotationAutomationNavigation::Target> NotationAutomationNavigation::stepAcrossStaves(const PolylineMap& polylines,
                                                                                                   const std::vector<Target>& selection,
                                                                                                   bool down, bool toTopBottom)
{
    const Target& anchor = down ? selection.back() : selection.front();

    // Find the tick range covered by the selected points in the anchor polyline...
    TickRange range { std::numeric_limits<int>::max(), std::numeric_limits<int>::min() };
    for (const Target& selected : selection) {
        if (selected.key == anchor.key) {
            const int tick = polylines.at(selected.key).pointsData.at(selected.pointIdx).tick;
            range.from = std::min(range.from, tick);
            range.to = std::max(range.to, tick);
        }
    }

    // Find the polyline above/below the anchor...
    std::optional<PolylineKey> target = nextInColumn(polylines, anchor.key, down);
    if (!target) {
        return std::nullopt;
    }

    // For top/bottom, keep going until the last polyline in that direction...
    if (toTopBottom) {
        while (std::optional<PolylineKey> next = nextInColumn(polylines, target.value(), down)) {
            target.emplace(next.value());
        }
    }

    // Select the point in that polyline closest (in ticks) to the selected range...
    return closestPointInPolyline(polylines, target.value(), range);
}

std::optional<NotationAutomationNavigation::Target> NotationAutomationNavigation::findTarget(const PolylineMap& polylines,
                                                                                             const std::vector<Target>& selection,
                                                                                             MoveDirection direction, bool toTopBottom)
{
    if (selection.empty()) {
        return firstNavigablePoint(polylines);
    }

    switch (direction) {
    case MoveDirection::Right:
        return stepAlongStaff(polylines, selection.back(), true);
    case MoveDirection::Left:
        return stepAlongStaff(polylines, selection.front(), false);
    case MoveDirection::Up:
    case MoveDirection::Down:
        return stepAcrossStaves(polylines, selection, direction == MoveDirection::Down, toTopBottom);
    case MoveDirection::Undefined:
        break;
    }

    return std::nullopt;
}
