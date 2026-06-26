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

#include "notationautomationcontroller.h"

#include "uicomponents/qml/Muse/UiComponents/polylineplot.h"
#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"

using namespace mu::notation;
using namespace muse::uicomponents;

static bool polylinePointIndexIsValid(const PolylinePlot* polyline, int pointIdx)
{
    IF_ASSERT_FAILED(polyline) {
        return false;
    }
    return pointIdx > -1 && pointIdx < static_cast<int>(polyline->points().size());
}

NotationAutomationController::NotationAutomationController(QQuickItem* linesParent, const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx), m_linesParent(linesParent)
{
}

void NotationAutomationController::init()
{
    IF_ASSERT_FAILED(automation() && currentNotation()) {
        return;
    }

    onCurrentNotationChanged();

    automation()->automationModeEnabledChanged().onNotify(this, [this]() {
        // updatePolylinesGeometry();
        onCurrentNotationChanged(); // Hack for testing purposes...
    });
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}

NotationAutomationController::SysStaffToPolylinesMap NotationAutomationController::createPolylinesForSystem(const System* system)
{
    IF_ASSERT_FAILED(system && m_linesParent && score()) {
        return {};
    }

    SysStaffToPolylinesMap map;
    const int systemStartTick = system->first()->tick().ticks();
    const int systemEndTick = system->last()->endTick().ticks();

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        const Staff* staff = score()->staff(staffIdx);
        const SysStaff* sysStaff = system->staff(staffIdx);
        if (!staff || !sysStaff || !staff->isPrimaryStaff()) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        // TODO: Staves can have multiple polylines due to horizontal frames, at the moment we're
        // providing a single polyline over the entire staff...
        PolylinePlot* polyline = new PolylinePlot(m_linesParent);

        const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
        polyline->setPoints(pointsInStaff(staff->id(), staffCanvasRect, systemStartTick, systemEndTick));

        polyline->setDrawBackground(false);
        polyline->setVisible(false);

        const SysStaffKey key(system, staffIdx);
        map.emplace(key, PolylinesSet({ polyline }));

        QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointMoved,
                         [this, key, polyline](int pointIdx, qreal x, qreal y, bool completed) {
            IF_ASSERT_FAILED(polylinePointIndexIsValid(polyline, pointIdx)) {
                return;
            }
            QVector<QPointF> points = polyline->points();
            points.replace(pointIdx, { x, y });
            polyline->setPoints(points);
            polyline->update(); // TODO: pass update rect?
            if (completed) {
                // Only request to update the model when completed...
                requestEditPoint({ key, polyline, pointIdx, x, y });
            }
        });

        staffIdx = system->nextVisibleStaff(staffIdx);
    }

    return map;
}

QVector<QPointF> NotationAutomationController::pointsInStaff(const muse::ID& staffId, const muse::RectF& sysStaffCanvasRect,
                                                             int startTick, int endTick) const
{
    QVector<QPointF> points;
    IF_ASSERT_FAILED(staffId.isValid() && score() && engravingAutomation()) {
        return points;
    }

    const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staffId, std::nullopt };
    for (const auto& pair : engravingAutomation()->curve(key)) {
        const int tick = pair.first;
        if (tick < startTick || tick > endTick) {
            continue;
        }

        const Fraction frac = Fraction::fromTicks(tick);
        const Segment* seg = score()->tick2leftSegmentMM(frac);
        IF_ASSERT_FAILED(seg) {
            continue;
        }

        // The point's tick may not exactly match that of a segment. For this reason we can only calculate the x position
        // of our points based on a "tickRatio". This ratio is based on the "tick difference" between the point tick and
        // the segment's tick, and the duration of the segment (in ticks)...
        const int tickDiff = tick - seg->tick().ticks();
        const double tickRatio = static_cast<double>(tickDiff) / seg->ticks().ticks();
        const double pointXInSeg = tickRatio * seg->width(); // The point's x relative to the segment

        const double segXInStaff = seg->canvasX() - sysStaffCanvasRect.x(); // The segment's x relative to the staff
        const double pointXInStaff = (segXInStaff + pointXInSeg) / sysStaffCanvasRect.width();

        // Point in/out values are always between 0 and 1 - higher value == lower Y...
        const mu::engraving::AutomationPoint& autoPoint = pair.second;
        if (muse::RealIsEqual(autoPoint.inValue, autoPoint.outValue)) {
            points.emplace_back(pointXInStaff, 1.0 - autoPoint.inValue);
            continue;
        }

        points.emplace_back(pointXInStaff, 1.0 - autoPoint.inValue);
        points.emplace_back(pointXInStaff, 1.0 - autoPoint.outValue);
    }

    return points;
}

void NotationAutomationController::updatePolylinesGeometry()
{
    const bool visible = automation() && automation()->isAutomationModeEnabled();

    for (const auto& [key, polylines] : m_stavesToLinesMap) {
        IF_ASSERT_FAILED(key.isValid() && !polylines.empty()) {
            continue;
        }

        // TODO: Staves can have multiple polylines due to horizontal frames, at the moment we're
        // providing a single polyline over the entire staff...
        PolylinePlot* polyline = *polylines.begin();
        polyline->setVisible(visible);
        if (!visible) {
            continue;
        }

        const SysStaff* sysStaff = key.system->staff(key.staffIdx);
        IF_ASSERT_FAILED(sysStaff) {
            continue;
        }

        //! NOTE: Here we should only update properties of the polyline that change relative to the view matrix. Polyline points are
        //! placed relative to the polylines themselves, and thus do not need to be modified in here...
        muse::RectF staffCanvasRect = sysStaff->bbox().translated(key.system->canvasPos());
        staffCanvasRect = m_viewMatrix.map(staffCanvasRect);

        polyline->setWidth(staffCanvasRect.width());
        polyline->setHeight(staffCanvasRect.height());
        polyline->setX(staffCanvasRect.x());
        polyline->setY(staffCanvasRect.y());
    }
}

void NotationAutomationController::setViewMatrix(const muse::draw::Transform& viewMatrix)
{
    if (viewMatrix == m_viewMatrix) {
        return;
    }
    m_viewMatrix = viewMatrix;

    if (automation() && automation()->isAutomationModeEnabled()) {
        updatePolylinesGeometry();
    }
}

void NotationAutomationController::onCurrentNotationChanged()
{
    // TODO: More efficient if we don't clear/recreate the polylines every time...
    for (const auto& [staff, polylines] : m_stavesToLinesMap) {
        for (PolylinePlot* polyline : polylines) {
            delete polyline;
        }
    }
    m_stavesToLinesMap.clear();

    if (!score()) {
        // Happens on close...
        return;
    }

    if (score()->masterScore()) { // HACK - needs to happen somewhere...
        score()->masterScore()->initAutomation();
    }

    for (const System* system : score()->systems()) {
        m_stavesToLinesMap.merge(createPolylinesForSystem(system));
    }

    updatePolylinesGeometry();
}

void NotationAutomationController::requestEditPoint(const EditPointParams& params)
{
    IF_ASSERT_FAILED(params.sysStaffKey.isValid() && polylinePointIndexIsValid(params.polyline, params.pointIndex)) {
        return;
    }
    const Staff* staff = score() ? score()->staff(params.sysStaffKey.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff) {
        return;
    }

    // TODO: Not always dynamics...
    const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staff->id(), /*voiceIdx*/ std::nullopt };
}

INotationAutomationPtr NotationAutomationController::automation() const
{
    const IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();
    return masterNotation ? masterNotation->automation() : nullptr;
}

INotationPtr NotationAutomationController::currentNotation() const
{
    return globalContext()->currentNotation();
}

mu::engraving::IAutomation* NotationAutomationController::engravingAutomation() const
{
    return score() ? score()->automation() : nullptr;
}

mu::engraving::Score* NotationAutomationController::score() const
{
    const INotationElementsPtr currElems = currentNotation() ? currentNotation()->elements() : nullptr;
    return currElems ? currElems->msScore() : nullptr;
}
