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
#include <unordered_set>
#include <vector>
#include <QPointF>
#include <QQuickItem>

#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "notation/notationtypes.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationcontextconfiguration.h"

namespace muse::uicomponents {
class PolylinePlot;
}

namespace mu::engraving {
class IAutomation;
}

namespace mu::notation {
class NotationAutomationController : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };
    muse::GlobalInject<INotationConfiguration> notationConfiguration;
    muse::ContextInject<INotationContextConfiguration> notationContextConfiguration = { this };

public:
    NotationAutomationController(QQuickItem* linesParent, const muse::modularity::ContextPtr& iocCtx);

    void init();
    void setViewMatrix(const muse::draw::Transform& viewMatrix);

private:
    // Necessary since SysStaff doesn't hold a reference to its system, which is needed
    // for calculating a SysStaff's relative position...
    struct SysStaffKey {
        const System* system = nullptr;
        const staff_idx_t staffIdx = muse::nidx;

        bool isValid() const
        {
            return system && system->first() && staffIdx != muse::nidx;
        }

        bool operator==(const SysStaffKey& k) const
        {
            IF_ASSERT_FAILED(isValid() && k.isValid()) {
                return false;
            }
            return system == k.system && staffIdx == k.staffIdx;
        }

        bool operator<(const SysStaffKey& k) const
        {
            IF_ASSERT_FAILED(isValid() && k.isValid()) {
                return false;
            }
            if (system == k.system) {
                return staffIdx < k.staffIdx;
            }
            return system->first()->index() < k.system->first()->index();
        }
    };

    using PolylinesSet = std::unordered_set<muse::uicomponents::PolylinePlot*>;
    using SysStaffToPolylinesMap = std::map<const SysStaffKey, const PolylinesSet>;

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
    };

    SysStaffToPolylinesMap createPolylinesForSystem(const System* system);
    QVector<PointData> pointsDataInStaff(const muse::ID& staff, const muse::RectF& sysStaffCanvasRect, int startTick, int endTick) const;

    void applyPolylineStyle(muse::uicomponents::PolylinePlot* polyline) const;
    void applyPolylineSizes(muse::uicomponents::PolylinePlot* polyline) const;

    void updatePolylinesGeometry();
    void onCurrentNotationChanged();

    void requestEditPoint(const PointData& oldPointData, const SysStaffKey& key, qreal x, qreal y);

    INotationAutomationPtr automation() const;
    mu::engraving::IAutomation* engravingAutomation() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_linesParent = nullptr;
    SysStaffToPolylinesMap m_stavesToLinesMap;
    muse::draw::Transform m_viewMatrix;
};
}
