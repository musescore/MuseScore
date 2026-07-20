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

#include <algorithm>
#include <cmath>
#include <set>

#include "uicomponents/qml/Muse/UiComponents/polylineplot.h"

#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/staff.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationautomation.h"
#include "notation/inotationelements.h" // IWYU pragma: keep

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::uicomponents;

static bool polylinePointIndexIsValid(const PolylinePlot* polyline, int pointIdx)
{
    IF_ASSERT_FAILED(polyline) {
        return false;
    }
    return pointIdx > -1 && pointIdx < static_cast<int>(polyline->points().size());
}

// Rescale between the common dynamic value range [PPPP, FFFF] and the full [0, 1] display range
// (the staff box), so those points fill the staff
static const muse::real_t DISPLAY_VALUE_RANGE_MIN = mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::PPPP);
static const muse::real_t DISPLAY_VALUE_RANGE_MAX = mu::engraving::ORDINARY_DYNAMIC_VALUES.at(mu::engraving::DynamicType::FFFF);

static double automationValueToDisplay(muse::real_t value)
{
    const double display = (value - DISPLAY_VALUE_RANGE_MIN) / (DISPLAY_VALUE_RANGE_MAX - DISPLAY_VALUE_RANGE_MIN);
    return std::clamp(display, 0.0, 1.0);
}

static muse::real_t automationValueFromDisplay(double displayValue)
{
    return DISPLAY_VALUE_RANGE_MIN + displayValue * (DISPLAY_VALUE_RANGE_MAX - DISPLAY_VALUE_RANGE_MIN);
}

// TODO: This will do for now, but it needs to be smarter because there will be gaps between ChordRest/TimeTick segment types (e.g.
// barlines). This method effectively needs to return the closest segment of the desired type (and probably whether canvasX is before
// or after the segment). If the canvasX is before the closest segment, then we'll go on to use the start tick of the segment. If it's
// after, we'll use the end tick of the segment....
static const Segment* segmentForCanvasX(const System* system, double canvasX)
{
    IF_ASSERT_FAILED(system) {
        return nullptr;
    }
    const mu::engraving::SegmentType type = mu::engraving::SegmentType::Duration;
    const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(type) : nullptr;
    while (seg && seg->system() == system) {
        if (canvasX >= seg->canvasX() && canvasX <= seg->canvasX() + seg->width()) {
            return seg;
        }
        seg = seg->next1(type);
    }
    return nullptr;
}

static const Segment* lastSegmentOfSystem(const System* system)
{
    const mu::engraving::SegmentType type = mu::engraving::SegmentType::Duration;
    const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(type) : nullptr;
    const Segment* last = nullptr;
    while (seg && seg->system() == system) {
        last = seg;
        seg = seg->next1(type);
    }
    return last;
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
        if (automation()->isAutomationModeEnabled() && !m_pendingChanges.isEmpty()) {
            applyAutomationChanges(m_pendingChanges);
            m_pendingChanges.clear();
        } else {
            updatePolylinesGeometry();
        }
    }, Asyncable::Mode::SetReplace /* FIXME */);

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    }, Asyncable::Mode::SetReplace /* FIXME */);
}

NotationAutomationController::SysStaffToPolylinesMap NotationAutomationController::createPolylinesForSystem(const System* system)
{
    IF_ASSERT_FAILED(system && m_linesParent && score()) {
        return {};
    }

    SysStaffToPolylinesMap map;

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        PolylinePlot* polyline = createPolylineForStaff(system, staffIdx);
        if (polyline) {
            map.emplace(SysStaffKey(system, staffIdx), PolylinesSet({ polyline }));
        }
        staffIdx = system->nextVisibleStaff(staffIdx);
    }

    return map;
}

muse::uicomponents::PolylinePlot* NotationAutomationController::createPolylineForStaff(const System* system, staff_idx_t staffIdx)
{
    IF_ASSERT_FAILED(system && m_linesParent && score()) {
        return nullptr;
    }

    const Staff* staff = score()->staff(staffIdx);
    const SysStaff* sysStaff = system->staff(staffIdx);
    if (!staff || !sysStaff || !staff->isPrimaryStaff()) {
        return nullptr;
    }

    const int systemStartTick = system->first()->tick().ticks();
    const int systemEndTick = system->last()->endTick().ticks();

    const Measure* firstMeasure = system->firstMeasure();
    const Segment* firstSeg = firstMeasure ? firstMeasure->first(mu::engraving::SegmentType::Duration) : nullptr;
    const Segment* lastSeg = lastSegmentOfSystem(system);

    // TODO: Staves can have multiple polylines due to horizontal frames, at the moment we're
    // providing a single polyline over the entire staff...
    PolylinePlot* polyline = new PolylinePlot(m_linesParent);

    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const QVector<PointData> pointsData = pointsDataInStaff(staff->id(), staffCanvasRect, systemStartTick, systemEndTick);

    const SysStaffKey key(system, staffIdx);
    m_pointsDataByStaff[key] = pointsData;

    //! NOTE: There can't be a 1-to-1 match between the number of points in the automation model and
    //! points on the polyline. A point with equal in/out values (i.e. a "BOTH" point) is represented
    //! as 1 polyline point, whereas a point with different in/out values will be represented with 2
    //! separate polyline points...
    QVector<QPointF> pointsForPolyline;
    pointsForPolyline.reserve(pointsData.size());
    for (const PointData& pointData : pointsData) {
        pointsForPolyline.emplace_back(pointData.qPointF);
    }
    polyline->setPoints(pointsForPolyline);

    applyPolylineStyle(polyline);
    polyline->setVisible(false);

    // Points can't be dragged past the system's first/last segment
    const qreal minX = firstSeg ? (firstSeg->canvasX() - staffCanvasRect.x()) / staffCanvasRect.width() : 0.0;
    const qreal maxX = lastSeg ? (lastSeg->canvasX() + lastSeg->width() - staffCanvasRect.x()) / staffCanvasRect.width() : 1.0;

    QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointMoved,
                     [this, key, polyline, minX, maxX](int pointIdx, qreal x, qreal y, bool completed) {
        IF_ASSERT_FAILED(polylinePointIndexIsValid(polyline, pointIdx)) {
            return;
        }
        const qreal clampedX = std::clamp(x, minX, maxX);

        if (completed) {
            // Only request to update the model when completed... If rejected, leave the point
            // where the live drag preview last put it rather than moving it any further
            const auto pointsDataIt = m_pointsDataByStaff.find(key);
            IF_ASSERT_FAILED(pointsDataIt != m_pointsDataByStaff.end() && pointIdx < pointsDataIt->second.size()) {
                return;
            }
            if (!requestEditPoint(pointsDataIt->second.at(pointIdx), key, clampedX, y)) {
                return;
            }
        }

        QVector<QPointF> points = polyline->points();
        points.replace(pointIdx, { clampedX, y });
        polyline->setPoints(points);
        polyline->update(); // TODO: pass update rect?
    });

    return polyline;
}

QVector<NotationAutomationController::PointData> NotationAutomationController::pointsDataInStaff(const muse::ID& staffId,
                                                                                                 const muse::RectF& sysStaffCanvasRect,
                                                                                                 int startTick, int endTick) const
{
    QVector<PointData> points;
    IF_ASSERT_FAILED(staffId.isValid() && score() && engravingAutomation()) {
        return points;
    }

    int currentPointIndex = 0;
    const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staffId, std::nullopt };
    const mu::engraving::AutomationCurve& curve = engravingAutomation()->curve(key);

    // Start at the first point >= startTick rather than curve.begin() - resolvedInValue() only ever
    // looks backward via std::prev(it), which works on any valid iterator, not just one reached by
    // walking from the beginning
    for (auto it = curve.lower_bound(startTick); it != curve.end(); ++it) {
        const int tick = it->first;
        const bool isPastEnd = tick > endTick;

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

        // Point in/out values are rescaled to the display range - higher value == lower Y...
        const mu::engraving::AutomationPoint& autoPoint = it->second;
        const mu::engraving::real_t resolvedIn = mu::engraving::resolvedInValue(curve, it);
        if (resolvedIn == autoPoint.outValue) {
            const QPointF qpf(pointXInStaff, 1.0 - automationValueToDisplay(resolvedIn));
            points.emplace_back(PointData(currentPointIndex++, tick, qpf, PointData::PointType::BOTH));
        } else {
            const QPointF qpfIn(pointXInStaff, 1.0 - automationValueToDisplay(resolvedIn));
            points.emplace_back(PointData(currentPointIndex++, tick, qpfIn, PointData::PointType::IN));

            const QPointF qpfOut(pointXInStaff, 1.0 - automationValueToDisplay(autoPoint.outValue));
            points.emplace_back(PointData(currentPointIndex++, tick, qpfOut, PointData::PointType::OUT));
        }

        if (isPastEnd) {
            // Included one point past the range - its own resolved value may depend on the outValue
            // of the last in-range point, which could have just changed
            break;
        }
    }

    return points;
}

void NotationAutomationController::applyPolylineStyle(PolylinePlot* polyline) const
{
    IF_ASSERT_FAILED(polyline) {
        return;
    }

    const QColor lineColor = notationConfiguration()->notationColor();
    const QColor pointFillColor = Qt::white;

    polyline->setLineColor(lineColor);
    polyline->setDrawBackground(false);

    PolylinePointStyle* standard = polyline->standardPointStyle();
    standard->setCenterColor(pointFillColor);
    standard->setOutlineColor(lineColor);

    PolylinePointStyle* hovered = polyline->hoveredPointStyle();
    hovered->setCenterColor(pointFillColor);
    hovered->setOutlineColor(lineColor);

    PolylinePointStyle* ghost = polyline->ghostPointStyle();
    QColor ghostColor = lineColor;
    ghostColor.setAlphaF(0.4f);
    ghost->setCenterColor(ghostColor);

    applyPolylineSizes(polyline);
}

void NotationAutomationController::applyPolylineSizes(PolylinePlot* polyline) const
{
    IF_ASSERT_FAILED(polyline) {
        return;
    }

    // Point/line sizes are raw pixels, so scale them with zoom manually
    // and clamp so they don't get huge or vanish at extreme zoom
    constexpr qreal baseLineWidth = 1.5;
    constexpr qreal baseStandardRadius = 3.0;
    constexpr qreal baseHoveredRadius = 4.0;
    constexpr qreal minZoomScale = 0.5;
    constexpr qreal maxZoomScale = 2.0;
    const qreal hundredPercentScale = notationContextConfiguration()->scalingFromZoomPercentage(100);
    const qreal zoomRatio = m_viewMatrix.m11() / hundredPercentScale;
    const qreal zoomScale = std::clamp(std::sqrt(zoomRatio), minZoomScale, maxZoomScale);
    const qreal lineWidth = baseLineWidth * zoomScale;

    polyline->setLineWidth(lineWidth);

    PolylinePointStyle* standard = polyline->standardPointStyle();
    standard->setCenterRadius(baseStandardRadius * zoomScale);
    standard->setOutlineWidth(lineWidth);

    PolylinePointStyle* hovered = polyline->hoveredPointStyle();
    hovered->setCenterRadius(baseHoveredRadius * zoomScale);
    hovered->setOutlineWidth(lineWidth);

    PolylinePointStyle* ghost = polyline->ghostPointStyle();
    ghost->setCenterRadius(baseStandardRadius * zoomScale);
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

        applyPolylineSizes(polyline);
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
    m_pendingChanges.clear();
    rebuildAllPolylines();

    if (engravingAutomation()) {
        engravingAutomation()->changed().onReceive(this, [this](const mu::engraving::AutomationChanges& changes) {
            onAutomationChanged(changes);
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }
}

void NotationAutomationController::rebuildAllPolylines()
{
    // TODO: More efficient if we don't clear/recreate the polylines every time...
    for (const auto& [staff, polylines] : m_stavesToLinesMap) {
        for (PolylinePlot* polyline : polylines) {
            delete polyline;
        }
    }
    m_stavesToLinesMap.clear();
    m_pointsDataByStaff.clear();

    if (!score()) {
        // Happens on close...
        return;
    }

    for (const System* system : score()->systems()) {
        m_stavesToLinesMap.merge(createPolylinesForSystem(system));
    }

    updatePolylinesGeometry();
}

void NotationAutomationController::updateStaffPointsInRange(const SysStaffKey& key, int tickFrom, int tickTo)
{
    auto mapIt = m_stavesToLinesMap.find(key);
    IF_ASSERT_FAILED(key.isValid() && mapIt != m_stavesToLinesMap.end() && !mapIt->second.empty()) {
        return;
    }
    PolylinePlot* polyline = *mapIt->second.begin();

    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    const SysStaff* sysStaff = key.system ? key.system->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff && sysStaff) {
        return;
    }

    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(key.system->canvasPos());
    const QVector<PointData> newRangeData = pointsDataInStaff(staff->id(), staffCanvasRect, tickFrom, tickTo);

    QVector<PointData>& pointsData = m_pointsDataByStaff[key];

    int firstIdx = 0;
    while (firstIdx < pointsData.size() && pointsData.at(firstIdx).tick < tickFrom) {
        ++firstIdx;
    }
    int lastIdx = firstIdx;
    while (lastIdx < pointsData.size() && pointsData.at(lastIdx).tick <= tickTo) {
        ++lastIdx;
    }
    if (!newRangeData.isEmpty() && newRangeData.back().tick > tickTo) {
        const int trailingTick = newRangeData.back().tick;
        while (lastIdx < pointsData.size() && pointsData.at(lastIdx).tick == trailingTick) {
            ++lastIdx;
        }
    }

    QVector<PointData> updatedPointsData;
    updatedPointsData.reserve(pointsData.size() - (lastIdx - firstIdx) + newRangeData.size());
    for (int i = 0; i < firstIdx; ++i) {
        updatedPointsData.push_back(pointsData.at(i));
    }
    for (const PointData& pointData : newRangeData) {
        updatedPointsData.push_back(pointData);
    }
    for (int i = lastIdx; i < pointsData.size(); ++i) {
        updatedPointsData.push_back(pointsData.at(i));
    }
    pointsData = updatedPointsData;

    QVector<QPointF> points;
    points.reserve(pointsData.size());
    for (const PointData& pointData : pointsData) {
        points.push_back(pointData.qPointF);
    }
    polyline->setPoints(points);
    polyline->update();
}

void NotationAutomationController::onAutomationChanged(const mu::engraving::AutomationChanges& changes)
{
    if (m_isApplyingOwnEdit) {
        return;
    }

    if (!automation() || !automation()->isAutomationModeEnabled()) {
        mergePendingChanges(changes);
        return;
    }

    applyAutomationChanges(changes);
}

void NotationAutomationController::mergePendingChanges(const mu::engraving::AutomationChanges& changes)
{
    if (changes.isFullReset) {
        m_pendingChanges.isFullReset = true;
        return;
    }
    for (const mu::engraving::AutomationCurveKey& key : changes.affectedKeys) {
        m_pendingChanges.extend(key, changes.tickFrom, changes.tickTo);
    }
}

void NotationAutomationController::applyAutomationChanges(const mu::engraving::AutomationChanges& changes)
{
    if (changes.isFullReset || !score()) {
        rebuildAllPolylines();
        return;
    }

    std::set<muse::ID> affectedStaffIds;
    for (const mu::engraving::AutomationCurveKey& key : changes.affectedKeys) {
        affectedStaffIds.insert(key.staffId);
    }

    // Only touch the staves that were actually affected and whose system overlaps the changed tick
    // range, and only recompute points within that range, rather than the whole score or even the
    // whole staff
    for (const auto& [key, polylines] : m_stavesToLinesMap) {
        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }
        const Staff* staff = score()->staff(key.staffIdx);
        if (!staff || affectedStaffIds.find(staff->id()) == affectedStaffIds.end()) {
            continue;
        }
        const System* system = key.system;
        const int systemStartTick = system->first()->tick().ticks();
        const int systemEndTick = system->last()->endTick().ticks();
        if (systemEndTick >= changes.tickFrom && systemStartTick <= changes.tickTo) {
            updateStaffPointsInRange(key, changes.tickFrom, changes.tickTo);
        }
    }

    updatePolylinesGeometry();
}

bool NotationAutomationController::requestEditPoint(const PointData& oldPointData, const SysStaffKey& key, qreal x, qreal y)
{
    // STEP 1 - Check that all of our parameters are valid...
    const PointData::PointType pointType = oldPointData.pointType;
    IF_ASSERT_FAILED(key.isValid() && pointType != PointData::PointType::UNKNOWN) {
        return false;
    }
    const System* system = key.system;
    const SysStaff* sysStaff = system ? system->staff(key.staffIdx) : nullptr;
    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(sysStaff && staff) {
        return false;
    }

    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const double staffStartCanvasX = staffCanvasRect.x();
    const double staffEndCanvasX = staffStartCanvasX + staffCanvasRect.width();
    const double pointCanvasX = staffStartCanvasX + x * (staffEndCanvasX - staffStartCanvasX);

    // No segment at this position - reject the move rather than guessing where it should go
    const Segment* newSeg = segmentForCanvasX(system, pointCanvasX);
    if (!newSeg) {
        return false;
    }

    // STEP 2 - Determine the new tick value based on the x parameter...
    const double segStartCanvasX = newSeg->canvasX();
    const double setEndCanvasX = segStartCanvasX + newSeg->width();

    const int segStartTick = newSeg->tick().ticks();
    const int segEndTick = segStartTick + newSeg->ticks().ticks();

    const double segCanvasWidth = setEndCanvasX - segStartCanvasX;
    const double tickRatio = segCanvasWidth > 0.0 ? (pointCanvasX - segStartCanvasX) / segCanvasWidth : 0.0;
    const int newTick = segStartTick + static_cast<int>(tickRatio * (segEndTick - segStartTick));
    const bool tickChanged = newTick != oldPointData.tick;

    // STEP 3 - Fetch the point being edited...
    // TODO: Not always dynamics...
    const mu::engraving::AutomationCurveKey curveKey { mu::engraving::AutomationType::Dynamics, staff->id(), /*voiceIdx*/ std::nullopt };

    const mu::engraving::AutomationCurve& curve = engravingAutomation()->curve(curveKey);
    const auto existingIt = curve.find(oldPointData.tick);
    IF_ASSERT_FAILED(existingIt != curve.end()) {
        return false;
    }
    const mu::engraving::AutomationPoint& existingPoint = existingIt->second;
    const mu::engraving::real_t existingInValue = mu::engraving::resolvedInValue(curve, existingIt);

    //! NOTE: Point in/out values are rescaled to the display range - higher value == lower Y...
    const mu::engraving::real_t newValue = automationValueFromDisplay(1.0 - y);

    // STEP 4 - Update the point's value, and move it to the new tick if necessary...

    //! NOTE: Moving a BOTH point is the simplest case - we can update its value then simply change the tick.
    //! Moving IN/OUT points is slightly more complex. In this case we need to set the in/out values to be
    //! equal at oldTick (effectively converting the original point to a BOTH point) and create a new point
    //! at newTick...

    if (!tickChanged || pointType == PointData::PointType::BOTH) {
        mu::engraving::AutomationPoint editedPoint = existingPoint;
        if (pointType == PointData::PointType::IN) {
            // The user explicitly chose this arrival value; it no longer follows whatever precedes it
            editedPoint.inValue = newValue;
        } else if (pointType == PointData::PointType::BOTH) {
            editedPoint.outValue = newValue;
            editedPoint.inValue = mu::engraving::AutomationPoint::SameAsOut {};
        } else {
            editedPoint.outValue = newValue;
        }
        editedPoint.generated = false;

        m_isApplyingOwnEdit = true;
        engravingAutomation()->editPoints(curveKey, { { newTick, editedPoint, oldPointData.tick } });
        m_isApplyingOwnEdit = false;

        return true;
    }

    // oldTick becomes a flat BOTH point via SameAsOut, so its inValue keeps following outValue
    // even if outValue changes again later
    mu::engraving::AutomationPoint updatedOldPoint = existingPoint;
    updatedOldPoint.inValue = mu::engraving::AutomationPoint::SameAsOut {};
    if (pointType == PointData::PointType::OUT) {
        updatedOldPoint.outValue = existingInValue;
    }
    updatedOldPoint.generated = false;

    mu::engraving::AutomationPoint newPoint;
    newPoint.outValue = newValue;
    newPoint.inValue = mu::engraving::AutomationPoint::SameAsOut {};
    newPoint.interpolation = existingPoint.interpolation;
    newPoint.itemId = existingPoint.itemId;

    m_isApplyingOwnEdit = true;
    engravingAutomation()->editPoints(curveKey, { { oldPointData.tick, updatedOldPoint }, { newTick, newPoint } });
    m_isApplyingOwnEdit = false;

    return true;
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
    return currentNotation() ? currentNotation()->elements()->msScore() : nullptr;
}
