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
#include <optional>
#include <unordered_set>
#include <vector>
#include <QPointF>
#include <QQuickItem>

#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuicontextconfiguration.h"
#include "notation/notationtypes.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationcontextconfiguration.h"
#include "engraving/automation/automationdata.h"
#include "engraving/automation/automationtypes.h"

namespace muse::uicomponents {
class PolylinePlot;
}

namespace mu::engraving {
class Staff;
struct ScoreChanges;
}

namespace mu::notation {
class NotationAutomationController : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };
    muse::ContextInject<muse::ui::IUiContextConfiguration> uiContextConfiguration = { this };
    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<INotationConfiguration> notationConfiguration;
    muse::ContextInject<INotationContextConfiguration> notationContextConfiguration = { this };
    muse::GlobalInject<mu::engraving::IEngravingConfiguration> engravingConfiguration;

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
            return system && !system->measures().empty() && staffIdx != muse::nidx;
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

    using PointsDataMap = std::map<SysStaffKey, QVector<PointData> >;

    struct TickStaffRange {
        int tickFrom = -1;
        int tickTo = -1;
        staff_idx_t staffIdxFrom = muse::nidx;
        staff_idx_t staffIdxTo = muse::nidx;
    };

    struct PendingScoreState {
        bool hasChanges = false;
        bool structural = false;
        std::optional<TickStaffRange> boundary;
    };

    SysStaffToPolylinesMap createPolylinesForSystem(const System* system);
    muse::uicomponents::PolylinePlot* createPolylineForStaff(const System* system, staff_idx_t staffIdx);
    QVector<PointData> pointsDataInStaff(const mu::engraving::Staff* staff, const muse::RectF& sysStaffCanvasRect, int startTick,
                                         int endTick) const;

    mu::engraving::AutomationType currentAutomationType() const;

    void applyPolylineStyle(muse::uicomponents::PolylinePlot* polyline, const SysStaffKey& key) const;
    void applyPolylineColors(muse::uicomponents::PolylinePlot* polyline, const SysStaffKey& key) const;
    // TODO: apply within a range? (for efficiency)
    void applyPolylineColorsUnderLine(muse::uicomponents::PolylinePlot* polyline, const SysStaffKey& key) const;

    QColor inversionRelativeColor(const muse::ui::ThemeStyleKey& key) const;

    void updatePolylinesGeometry();
    void updatePolylinesColors();
    void onCurrentNotationChanged();
    void rebuildAllPolylines();

    void updateStaffPointsInRange(const SysStaffKey& key, int tickFrom, int tickTo);

    void mergePendingChanges(const mu::engraving::AutomationChanges& changes);
    void mergePendingScoreChanges(const mu::engraving::ScoreChanges& changes);
    void scheduleUpdate();
    void processPendingChanges();
    void applyAutomationChanges(const mu::engraving::AutomationChanges& changes);

    bool requestEditPoint(const PointData& oldPointData, const SysStaffKey& key, qreal x, qreal y);
    bool requestAddPoint(const SysStaffKey& key, qreal x, qreal y);
    bool requestRemovePoint(const PointData& pointData, const SysStaffKey& key);
    void editAutomationPoints(const mu::engraving::AutomationCurveKey& key, mu::engraving::AutomationPointEdits& edits);

    const mu::engraving::AutomationPoint* automationPointAt(const SysStaffKey& key, int tick) const;

    INotationAutomationPtr automation() const;
    mu::engraving::AutomationDataConstPtr automationData() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_linesParent = nullptr;
    SysStaffToPolylinesMap m_stavesToLinesMap;
    PointsDataMap m_pointsDataByStaff;
    muse::draw::Transform m_viewMatrix;
    mu::engraving::AutomationChanges m_pendingChanges;
    PendingScoreState m_pendingScoreState;
    bool m_updateScheduled = false;
};
}
