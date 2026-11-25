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
#include <QQuickItem>

#include "project/iprojectsession.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuicontextconfiguration.h"
#include "notation/notationtypes.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationcontextconfiguration.h"
#include "engraving/automation/automationdata.h"
#include "engraving/automation/automationtypes.h"

#include "automation/notationautomationtypes.h"

namespace mu::engraving {
class Staff;
struct ScoreChanges;
}

namespace mu::notation {
class NotationAutomationController : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<mu::project::IProjectSession> projectSession = { this };
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

    QVector<PointData> pointsDataInStaffRegion(const mu::engraving::Staff* staff, const muse::RectF& regionRect, int startTick,
                                               int endTick) const;

    mu::engraving::AutomationType currentAutomationType() const;
    mu::engraving::AutomationCurveKey currentCurveKey(const mu::engraving::Staff* staff) const;

    void applyPolylineStyle(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;
    void applyPolylineColors(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;
    // TODO: apply within a range? (for efficiency)
    void applyPolylineColorsUnderLine(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;
    void applyPolylineEdgeValues(muse::uicomponents::PolylinePlot* polyline, const PolylineKey& key) const;

    QColor inversionRelativeColor(const muse::ui::ThemeStyleKey& key) const;

    void updatePolylinesGeometry();
    void updatePolylinesColors();
    void onCurrentNotationChanged();

    void rebuildAllPolylines();
    void buildAndAddPolylinesForSystem(const System* system);
    void buildAndAddPolylinesForStaff(const System* system, staff_idx_t staffIdx);
    void buildAndAddPolylineForStaffRegion(const System* system, const mu::engraving::Staff* staff, const Measure* startMeasure,
                                           const Measure* endMeasure);

    void rebuildPolylinePoints(const PolylineKey& key);

    void mergePendingChanges(const mu::engraving::AutomationChanges& changes);
    void mergePendingScoreChanges(const mu::engraving::ScoreChanges& changes);
    void scheduleUpdate();
    void processPendingChanges();
    void applyAutomationChanges(const mu::engraving::AutomationChanges& changes);

    bool requestEditPoint(const PointData& oldPointData, const PolylineKey& key, qreal x, qreal y);
    bool requestAddPoint(const PolylineKey& key, qreal x, qreal y, bool useCurveValue);
    bool requestRemovePoint(const PointData& pointData, const PolylineKey& key);
    void editAutomationPoints(const mu::engraving::AutomationCurveKey& key, mu::engraving::AutomationPointEdits& edits);

    PolylineData polylineData(const PolylineKey& key) const;
    void setPolylinePointsData(const PolylineKey& key, const QVector<PointData>& pointsData);

    const mu::engraving::AutomationPoint* automationPointAt(const PolylineKey& key, int tick) const;

    bool isMuseSamplerDynamics(const mu::engraving::Staff* staff, int tick) const;

    void navigate(MoveDirection direction, bool toTopBottom);

    INotationAutomationPtr automation() const;
    mu::engraving::AutomationDataConstPtr automationData() const;
    INotationPtr currentNotation() const;
    mu::engraving::Score* score() const;

    QQuickItem* m_linesParent = nullptr;

    PolylineMap m_polylinesDataMap;
    muse::draw::Transform m_viewMatrix;

    mu::engraving::AutomationChanges m_pendingChanges;
    PendingScoreState m_pendingScoreState;
    bool m_updateScheduled = false;

    muse::uicomponents::PolylinePlot* m_currentlyEditedPolyline = nullptr;
};
}
