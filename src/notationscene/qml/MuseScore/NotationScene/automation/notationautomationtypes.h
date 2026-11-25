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

#include <map>

#include <QPointF>
#include <QVector>

#include "notation/notationtypes.h"

#include "log.h"

namespace muse::uicomponents {
class PolylinePlot;
}

namespace mu::notation {
// A polyline can be uniquely identified using the system, staff, and startTick...
struct PolylineKey {
    const System* system = nullptr;
    const staff_idx_t staffIdx = muse::nidx;
    const int startTick = -1;

    PolylineKey() = default;
    PolylineKey(const System* system, staff_idx_t staffIdx, int startTick)
        : system(system), staffIdx(staffIdx), startTick(startTick) {}

    bool isValid() const
    {
        return system && !system->measures().empty() && staffIdx != muse::nidx && startTick != -1;
    }

    bool operator==(const PolylineKey& k) const
    {
        IF_ASSERT_FAILED(isValid() && k.isValid()) {
            return false;
        }
        return system == k.system && staffIdx == k.staffIdx && startTick == k.startTick;
    }

    bool operator<(const PolylineKey& k) const
    {
        IF_ASSERT_FAILED(isValid() && k.isValid()) {
            return false;
        }
        if (system != k.system) {
            // On different systems...
            return system->first()->index() < k.system->first()->index();
        }
        if (staffIdx != k.staffIdx) {
            // On different staves...
            return staffIdx < k.staffIdx;
        }
        // On same staff...
        return startTick < k.startTick;
    }
};

struct PointData {
    enum class PointType : unsigned char {
        UNKNOWN,
        IN,
        OUT,
        BOTH
    };
    int polylinePointIndex = -1;
    int tick = -1;
    QPointF qPointF;
    PointType pointType = PointType::UNKNOWN;
    bool isPending = false; // true while the user is adding/dragging the point (before committing to the model)

    PointData() = default;
    PointData(int polylinePointIndex, int tick, const QPointF& qPointF, PointType pointType, bool isPending = false)
        : polylinePointIndex(polylinePointIndex), tick(tick), qPointF(qPointF), pointType(pointType), isPending(isPending) {}
};

struct PolylineData {
    muse::uicomponents::PolylinePlot* polyline = nullptr;
    QVector<PointData> pointsData;
    //! NOTE: Horizontal boxes can break up the staves of a system, so a polyline covers a "region" - a run
    //! of measures bounded by horizontal boxes and/or the system itself...
    const Measure* startMeasure = nullptr;
    const Measure* endMeasure = nullptr;

    PolylineData() = default;
    PolylineData(muse::uicomponents::PolylinePlot* polyline, const QVector<PointData>& pointsData, const Measure* startMeasure,
                 const Measure* endMeasure)
        : polyline(polyline), pointsData(pointsData), startMeasure(startMeasure), endMeasure(endMeasure) {}
};

using PolylineMap = std::map<PolylineKey, PolylineData>;
}
