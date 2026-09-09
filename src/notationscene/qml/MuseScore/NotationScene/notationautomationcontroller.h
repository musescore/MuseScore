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
    struct PolylineKey {
        const System* system = nullptr;
        const staff_idx_t staffIdx = muse::nidx;
        const int startTick = -1;

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
    };

    struct PolylineData {
        muse::uicomponents::PolylinePlot* polyline = nullptr;
        QVector<PointData> pointsData;
    };

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

    QVector<PointData> pointsDataInStaff(const mu::engraving::Staff* staff, const muse::RectF& sysStaffCanvasRect, int startTick,
                                         int endTick) const;

    mu::engraving::AutomationType currentAutomationType() const;

    void applyPolylineStyle(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;
    void applyPolylineColors(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;
    // TODO: apply within a range? (for efficiency)
    void applyPolylineColorsUnderLine(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;

    QColor inversionRelativeColor(const muse::ui::ThemeStyleKey& key) const;

    void updatePolylinesGeometry();
    void updatePolylinesColors();
    void onCurrentNotationChanged();

    void rebuildAllPolylines();
    void buildAndAddPolylinesForSystem(const System* system);
    void buildAndAddPolylinesForStaff(const System* system, staff_idx_t staffIdx);

    void updateStaffPointsInRange(const PolylineKey& key, int tickFrom, int tickTo);

    void mergePendingChanges(const mu::engraving::AutomationChanges& changes);
    void mergePendingScoreChanges(const mu::engraving::ScoreChanges& changes);
    void scheduleUpdate();
    void processPendingChanges();
    void applyAutomationChanges(const mu::engraving::AutomationChanges& changes);

    bool requestEditPoint(const PointData& oldPointData, const PolylineKey& key, qreal x, qreal y);
    bool requestAddPoint(const PolylineKey& key, qreal x, qreal y);
    bool requestRemovePoint(const PointData& pointData, const PolylineKey& key);
    void editAutomationPoints(const mu::engraving::AutomationCurveKey& key, mu::engraving::AutomationPointEdits& edits);

    const mu::engraving::AutomationPoint* automationPointAt(const PolylineKey& key, int tick) const;

    INotationAutomationPtr automation() const;
    mu::engraving::AutomationDataConstPtr automationData() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_linesParent = nullptr;
    std::map<PolylineKey, PolylineData> m_polylinesDataMap;
    muse::draw::Transform m_viewMatrix;
    mu::engraving::AutomationChanges m_pendingChanges;
    PendingScoreState m_pendingScoreState;
    bool m_updateScheduled = false;
};
}
