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

#include "automation/notationautomationdisplaymapping.h"
#include "automation/notationautomationutils.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <set>

#include "uicomponents/qml/Muse/UiComponents/polylineplot.h"

#include "engraving/iengravingconfiguration.h" // IWYU pragma: keep
#include "engraving/automation/automationdata.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationautomation.h"
#include "notation/inotationelements.h" // IWYU pragma: keep

#include "project/inotationproject.h" // IWYU pragma: keep
#include "project/iprojectaudiosettings.h"

#include "audio/common/audiotypes.h"

#include "global/async/async.h"
#include "global/containers.h"
#include "global/realfn.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::uicomponents;

using SetPoint = mu::engraving::AutomationPointEdit::SetPoint;
using MovePoint = mu::engraving::AutomationPointEdit::MovePoint;
using ErasePoint = mu::engraving::AutomationPointEdit::ErasePoint;

constexpr static qreal POLYLINE_LINE_WIDTH = 1.5;

constexpr static qreal POLYLINE_STANDARD_CENTER_RADIUS = 3.0;
constexpr static qreal POLYLINE_HOVERED_CENTER_RADIUS = 3.5;
constexpr static qreal POLYLINE_SELECTED_CENTER_RADIUS = 3.5;

constexpr static qreal POLYLINE_SELECTED_MIDDLE_RING_WIDTH = 1.5;

constexpr static int POLYLINE_SELECTED_HOVERED_ALPHA = 127;
constexpr static int POLYLINE_GENERATED_AREA_ALPHA = 51;
constexpr static int POLYLINE_EDITED_AREA_ALPHA = 102;

static bool polylinePointIndexIsValid(const PolylinePlot* polyline, int pointIdx)
{
    IF_ASSERT_FAILED(polyline) {
        return false;
    }
    return pointIdx > -1 && pointIdx < static_cast<int>(polyline->points().size());
}

static AutomationCurveKey curveKeyFor(AutomationLane lane, const Staff* staff)
{
    const AutomationType type = automationTypeOf(lane);

    switch (lane) {
    case AutomationLane::Dynamics:
        return AutomationCurveKey::staff(type, staff->id());
    case AutomationLane::Tempo:
    case AutomationLane::MasterVolume:
    case AutomationLane::MasterPan:
        return AutomationCurveKey::global(type);
    case AutomationLane::Volume:
    case AutomationLane::Pan: {
        const Part* part = staff->part();
        const InstrumentTrackId trackId { part->id(), part->instrumentId() };
        return AutomationCurveKey::instrument(type, trackId);
    }
    }

    return AutomationCurveKey();
}

static int tickOfPointBefore(const AutomationCurve& curve, int tick)
{
    auto it = curve.lower_bound(tick);
    if (it == curve.begin()) {
        return -1;
    }
    --it;
    return it->first;
}

static int tickOfPointAfter(const AutomationCurve& curve, int tick)
{
    const auto it = curve.upper_bound(tick);
    if (it == curve.end()) {
        return -1;
    }
    return it->first;
}

// The curve's value at a tick lying between two of its points...
static double curveValueBetween(const AutomationCurve& curve, int tickBefore, int tickAfter, int tick)
{
    IF_ASSERT_FAILED(tickBefore >= 0 || tickAfter >= 0) {
        return 0.0;
    }

    if (tickAfter == -1) {
        // No point after - the curve stays flat at the last point's out value...
        const mu::engraving::AutomationPoint pointBefore = muse::value(curve, tickBefore, mu::engraving::AutomationPoint());
        return static_cast<double>(pointBefore.value.outValue);
    }

    const mu::engraving::AutomationPoint pointAfter = muse::value(curve, tickAfter, mu::engraving::AutomationPoint());
    if (tickBefore == -1) {
        // No point before - the curve is flat at the first point's in value...
        return static_cast<double>(muse::mpe::resolveInValue(pointAfter.value, std::nullopt));
    }

    const mu::engraving::AutomationPoint pointBefore = muse::value(curve, tickBefore, mu::engraving::AutomationPoint());
    const muse::real_t ratio = static_cast<muse::real_t>(tick - tickBefore) / static_cast<muse::real_t>(tickAfter - tickBefore);
    return static_cast<double>(muse::mpe::evaluateAt(pointAfter.value, pointBefore.value.outValue, ratio));
}

static staff_idx_t firstVisibleStaffIdx(const Score* score)
{
    for (staff_idx_t i = 0; i < score->nstaves(); ++i) {
        if (score->staff(i)->show()) {
            return i;
        }
    }

    return muse::nidx;
}

static bool isStructuralChange(const mu::engraving::ScoreChanges& changes)
{
    if (!changes.changedObjects.empty() && !changes.isValidBoundary()) {
        return true;
    }

    static const std::unordered_set<mu::engraving::ElementType> STRUCTURAL_TYPES {
        mu::engraving::ElementType::MEASURE,
        mu::engraving::ElementType::PART,
        mu::engraving::ElementType::HBOX,
    };

    for (const mu::engraving::ElementType type : changes.changedTypes) {
        if (muse::contains(STRUCTURAL_TYPES, type)) {
            return true;
        }
    }

    return false;
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

    automation()->resetEditStateRequested().onNotify(this, [this]() {
        if (m_currentlyEditedPolyline) {
            m_currentlyEditedPolyline->cancelEdit();
            // Rebuild all because changes to this polyline may have affected those on other systems...
            // TODO: As above - e.g. changes to the end of a polyline in one system should affect the polyline on
            // the next system (probably requires the implementation of "startY" in PolylinePlot)...
            rebuildAllPolylines();
        } else {
            // Leave automation mode if we're not currently editing a point...
            automation()->setAutomationModeEnabled(false);
        }
        m_currentlyEditedPolyline = nullptr;
    }, Asyncable::Mode::SetReplace /* FIXME */);

    notationConfiguration()->currentAutomationLaneChanged().onNotify(this, [this]() {
        rebuildAllPolylines();
    }, Asyncable::Mode::SetReplace /* FIXME */);

    projectSession()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    }, Asyncable::Mode::SetReplace /* FIXME */);

    notationConfiguration()->scoreInversionChanged().onNotify(this, [this]() {
        updatePolylinesColors();
    }, Asyncable::Mode::SetReplace /* FIXME */);

    notationConfiguration()->isOnlyInvertInDarkThemeChanged().onNotify(this, [this]() {
        updatePolylinesColors();
    }, Asyncable::Mode::SetReplace /* FIXME */);

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        updatePolylinesColors();
    }, Asyncable::Mode::SetReplace /* FIXME */);

    engravingConfiguration()->selectionColorChanged().onReceive(this, [this](voice_idx_t idx, const muse::draw::Color&) {
        // voice 1 color is used for the centre of selected points, "all voices color" is used for the area under lines
        if (idx == 0 || idx == mu::engraving::VOICES) {
            updatePolylinesColors();
        }
    }, Asyncable::Mode::SetReplace /* FIXME */);
}

void NotationAutomationController::buildAndAddPolylinesForSystem(const System* system)
{
    IF_ASSERT_FAILED(system && m_linesParent && score()) {
        return;
    }

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        buildAndAddPolylinesForStaff(system, staffIdx);
        staffIdx = system->nextVisibleStaff(staffIdx);
    }
}

void NotationAutomationController::buildAndAddPolylinesForStaff(const System* system, staff_idx_t staffIdx)
{
    IF_ASSERT_FAILED(score() && system && staffIdx != muse::nidx) {
        return;
    }

    const Staff* staff = score()->staff(staffIdx);
    IF_ASSERT_FAILED(staff) {
        return;
    }
    if (!staff->isPrimaryStaff()) {
        return;
    }

    const AutomationCurveKey curveKey = currentCurveKey(staff);
    if (curveKey.isGlobal() && staffIdx != firstVisibleStaffIdx(score())) {
        // Score-scoped automation is only drawn on the score's first staff
        return;
    }

    if (curveKey.trackId().has_value() && !staff->isTop()) {
        // Instrument-scoped automation is only drawn on the instrument's first staff
        return;
    }

    const Measure* startMeasure = nullptr;
    const Measure* endMeasure = nullptr;
    for (const MeasureBase* mb : system->measures()) {
        if (mb->isMeasure()) {
            if (!startMeasure) {
                startMeasure = toMeasure(mb);
            }
            endMeasure = toMeasure(mb);
        } else if (mb->isHBox() && startMeasure) {
            // Horizontal boxes end the current region - anything after them belongs to the next one...
            buildAndAddPolylineForStaffRegion(system, staff, startMeasure, endMeasure);
            startMeasure = nullptr;
            endMeasure = nullptr;
        }
    }

    if (startMeasure) {
        buildAndAddPolylineForStaffRegion(system, staff, startMeasure, endMeasure);
    }
}

void NotationAutomationController::buildAndAddPolylineForStaffRegion(const System* system, const Staff* staff,
                                                                     const Measure* startMeasure, const Measure* endMeasure)
{
    IF_ASSERT_FAILED(score() && system && staff && startMeasure && endMeasure && m_linesParent) {
        return;
    }

    const staff_idx_t staffIdx = staff->idx();
    const muse::RectF regionRect = regionCanvasRect(system, staffIdx, startMeasure, endMeasure);
    IF_ASSERT_FAILED(regionRect.isValid()) {
        return;
    }

    const int startTick = startMeasure->tick().ticks();
    const int endTick = regionLastTick(endMeasure);

    PolylinePlot* polyline = new PolylinePlot(m_linesParent);

    const QVector<PointData> pointsData = pointsDataInStaffRegion(staff, regionRect, startTick, endTick);

    const PolylineKey key(system, staffIdx, startTick);
    const bool inserted = m_polylinesDataMap.emplace(key, PolylineData(polyline, pointsData, startMeasure, endMeasure)).second;
    IF_ASSERT_FAILED(inserted) {
        delete polyline;
        return;
    }

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

    applyPolylineStyle(polyline, key);
    applyPolylineEdgeValues(polyline, key);
    polyline->setVisible(false);

    QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointMoved,
                     [this, key, polyline](int pointIdx, qreal x, qreal y, bool completed) {
        IF_ASSERT_FAILED(polylinePointIndexIsValid(polyline, pointIdx)) {
            return;
        }

        const PolylineData data = polylineData(key);
        IF_ASSERT_FAILED(data.polyline) {
            return;
        }

        const QVector<PointData>& pointsData = data.pointsData;
        IF_ASSERT_FAILED(pointIdx >= 0 && pointIdx < pointsData.size()) {
            return;
        }

        // Points can't be dragged past the region's first/last segments (TODO: we shouldn't need to enforce it like this,
        // the polyline should start end at the first/last segments)...
        const muse::RectF regionRect = regionCanvasRect(key.system, key.staffIdx, data.startMeasure, data.endMeasure);
        const Segment* firstSeg = data.startMeasure->first(SegmentType::Duration);
        const Segment* lastSeg = data.endMeasure->last(SegmentType::Duration);
        IF_ASSERT_FAILED(regionRect.isValid() && firstSeg && lastSeg) {
            return;
        }

        const qreal minX = (firstSeg->canvasX() - regionRect.x()) / regionRect.width();
        const qreal maxX = (lastSeg->canvasX() + lastSeg->width() - regionRect.x()) / regionRect.width();

        const PointData& oldPointData = pointsData[pointIdx];
        const mu::engraving::AutomationPoint* automationPoint = automationPointAt(key, oldPointData.tick);
        const bool editRestricted = !oldPointData.isPending
                                    && (!automationPoint || automationPoint->generated || automationPoint->itemId.has_value());
        const qreal clampedX = editRestricted ? oldPointData.qPointF.x() : std::clamp(x, minX, maxX);

        const auto setPreviewPoint = [this, polyline, pointIdx, key](const QPointF& point) {
            QVector<QPointF> points = polyline->points();
            points.replace(pointIdx, point);
            polyline->setPoints(points);
            applyPolylineColorsUnderLine(polyline, key);
            polyline->update(); // TODO: pass update rect?
        };

        if (completed) {
            m_currentlyEditedPolyline = nullptr;

            if (oldPointData.isPending) {
                // Not in the model yet - add it where it was dropped
                requestAddPoint(key, clampedX, y, /*useCurveValue*/ false);
                rebuildPolylinePoints(key);
                return;
            }

            if (!requestEditPoint(oldPointData, key, clampedX, y)) {
                // Edit was rejected - snap the point back to where it actually is instead of
                // leaving the live-drag preview stuck at the rejected position
                setPreviewPoint(oldPointData.qPointF);
            }
            return;
        }

        m_currentlyEditedPolyline = polyline;

        // Live drag preview
        setPreviewPoint({ clampedX, y });
    });

    QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointAdded,
                     [this, key, polyline](qreal x, qreal y, bool completed) {
        if (completed) {
            m_currentlyEditedPolyline = nullptr;
            requestAddPoint(key, x, y, /*useCurveValue*/ true);
            rebuildPolylinePoints(key);
            return;
        }

        const PolylineData data = polylineData(key);
        IF_ASSERT_FAILED(data.polyline) {
            return;
        }

        const int tick = tickFromXInPolyline(key, data, x);
        if (tick == -1) {
            return;
        }

        m_currentlyEditedPolyline = polyline;

        QVector<PointData> pointsData = data.pointsData;

        int insertIdx = 0;
        while (insertIdx < pointsData.size() && pointsData.at(insertIdx).tick < tick) {
            ++insertIdx;
        }
        pointsData.insert(insertIdx, PointData(-1, tick, { x, y }, PointData::PointType::BOTH, /*isPending*/ true));
        setPolylinePointsData(key, pointsData);

        QVector<QPointF> points = polyline->points();
        points.insert(insertIdx, { x, y });
        polyline->setPoints(points);
        applyPolylineColorsUnderLine(polyline, key);
    });

    QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointRemoved,
                     [this, key](int pointIdx, bool completed) {
        if (!completed) {
            return;
        }
        const PolylineData data = polylineData(key);
        IF_ASSERT_FAILED(data.polyline) {
            return;
        }

        const QVector<PointData>& pointsData = data.pointsData;
        IF_ASSERT_FAILED(pointIdx >= 0 && pointIdx < pointsData.size()) {
            return;
        }
        requestRemovePoint(pointsData.at(pointIdx), key);
    });
}

QVector<PointData> NotationAutomationController::pointsDataInStaffRegion(
    const mu::engraving::Staff* staff, const muse::RectF& regionRect, int startTick, int endTick) const
{
    QVector<PointData> points;
    IF_ASSERT_FAILED(staff && score() && automationData() && regionRect.isValid()) {
        return points;
    }

    const AutomationType type = currentAutomationType();

    int currentPointIndex = 0;
    const mu::engraving::AutomationCurveKey key = currentCurveKey(staff);
    const mu::engraving::AutomationCurve& curve = automationData()->curve(key);

    // Start at the first point >= startTick rather than curve.begin() - resolvedInValue() only ever
    // looks backward via std::prev(it), which works on any valid iterator, not just one reached by
    // walking from the beginning
    for (auto it = curve.lower_bound(startTick); it != curve.end() && it->first <= endTick; ++it) {
        const int tick = it->first;
        const bool isMuseSampler = isMuseSamplerDynamics(staff, tick);

        const Fraction frac = Fraction::fromTicks(tick);
        const Segment* seg = score()->tick2leftSegmentMM(frac);
        if (!seg) { //! FIXME: fix automation curve on measure repeats
            continue;
        }

        // The point's tick may not exactly match that of a segment. For this reason we can only calculate the x position
        // of our points based on a "tickRatio". This ratio is based on the "tick difference" between the point tick and
        // the segment's tick, and the duration of the segment (in ticks)...
        const int tickDiff = tick - seg->tick().ticks();
        const double tickRatio = static_cast<double>(tickDiff) / seg->ticks().ticks();
        const double pointXInSeg = tickRatio * seg->width(); // The point's x relative to the segment

        const double segXInRegion = seg->canvasX() - regionRect.x(); // The segment's x relative to the region
        const double pointXInRegion = (segXInRegion + pointXInSeg) / regionRect.width();

        // Point in/out values are rescaled to the display range - higher value == lower Y...
        const mu::engraving::AutomationPoint& autoPoint = it->second;
        const mu::engraving::real_t resolvedIn = mu::engraving::resolveInValue(curve, it);
        if (resolvedIn == autoPoint.value.outValue) {
            const QPointF qpf(pointXInRegion, 1.0 - NotationAutomationDisplayMapping::toDisplay(type, isMuseSampler, resolvedIn));
            points.emplace_back(PointData(currentPointIndex++, tick, qpf, PointData::PointType::BOTH));
        } else {
            const QPointF qpfIn(pointXInRegion, 1.0 - NotationAutomationDisplayMapping::toDisplay(type, isMuseSampler, resolvedIn));
            points.emplace_back(PointData(currentPointIndex++, tick, qpfIn, PointData::PointType::IN));

            const QPointF qpfOut(pointXInRegion,
                                 1.0 - NotationAutomationDisplayMapping::toDisplay(type, isMuseSampler, autoPoint.value.outValue));
            points.emplace_back(PointData(currentPointIndex++, tick, qpfOut, PointData::PointType::OUT));
        }
    }

    return points;
}

void NotationAutomationController::applyPolylineStyle(PolylinePlot* polyline, const PolylineKey& key) const
{
    IF_ASSERT_FAILED(polyline) {
        return;
    }

    polyline->setLineWidth(POLYLINE_LINE_WIDTH);
    polyline->setDrawBackground(false);

    //! NOTE: The polyline resets its baseline to its default value whenever it has no points. Higher value == lower Y...
    const qreal defaultValue = NotationAutomationDisplayMapping::defaultDisplayValue(currentAutomationType());
    polyline->setDefaultValue(1.0 - defaultValue);
    polyline->setBaselineN(defaultValue);

    polyline->setGhostPointsEnabled(false);
    polyline->setSelectedPointsEnabled(true);

    PolylinePointStyle* standard = polyline->standardPointStyle();
    standard->setCenterRadius(POLYLINE_STANDARD_CENTER_RADIUS);
    standard->setOutlineWidth(POLYLINE_LINE_WIDTH);

    standard->setCenterRadiusHovered(POLYLINE_HOVERED_CENTER_RADIUS);
    standard->setOutlineWidthHovered(POLYLINE_LINE_WIDTH);

    PolylinePointStyle* selected = polyline->selectedPointStyle();
    selected->setCenterRadius(POLYLINE_SELECTED_CENTER_RADIUS);
    selected->setMiddleRingWidth(POLYLINE_SELECTED_MIDDLE_RING_WIDTH);
    selected->setOutlineWidth(POLYLINE_LINE_WIDTH);

    selected->setCenterRadiusHovered(POLYLINE_SELECTED_CENTER_RADIUS);
    selected->setMiddleRingWidthHovered(POLYLINE_SELECTED_MIDDLE_RING_WIDTH);
    selected->setOutlineWidthHovered(POLYLINE_LINE_WIDTH);

    applyPolylineColors(polyline, key);
}

void NotationAutomationController::applyPolylineColors(PolylinePlot* polyline, const PolylineKey& key) const
{
    IF_ASSERT_FAILED(polyline) {
        return;
    }

    const QColor lineColor = inversionRelativeColor(muse::ui::FONT_PRIMARY_COLOR);
    polyline->setLineColor(lineColor);

    const QColor foregroundColor = notationConfiguration()->foregroundColor();

    PolylinePointStyle* standard = polyline->standardPointStyle();
    standard->setCenterColor(foregroundColor);
    standard->setOutlineColor(lineColor);

    standard->setCenterColorHovered(inversionRelativeColor(muse::ui::BUTTON_COLOR));
    standard->setOutlineColorHovered(lineColor);

    QColor selectionColor = engravingConfiguration()->selectionColor().toQColor();

    PolylinePointStyle* selected = polyline->selectedPointStyle();
    selected->setCenterColor(selectionColor);
    selected->setMiddleRingColor(foregroundColor);
    selected->setOutlineColor(lineColor);

    selectionColor.setAlpha(POLYLINE_SELECTED_HOVERED_ALPHA);
    selected->setCenterColorHovered(selectionColor);
    selected->setMiddleRingColorHovered(foregroundColor);
    selected->setOutlineColorHovered(lineColor);

    applyPolylineColorsUnderLine(polyline, key);
}

void NotationAutomationController::applyPolylineColorsUnderLine(PolylinePlot* polyline, const PolylineKey& key) const
{
    const PolylineData data = polylineData(key);
    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(polyline && data.polyline && staff && automationData()) {
        return;
    }

    // TODO: Cache these colors?
    const QColor allVoicesColor = engravingConfiguration()->selectionColor(mu::engraving::VOICES).toQColor();

    QColor generatedColor = allVoicesColor;
    generatedColor.setAlpha(POLYLINE_GENERATED_AREA_ALPHA);

    QColor editedColor = allVoicesColor;
    editedColor.setAlpha(POLYLINE_EDITED_AREA_ALPHA);

    const QVector<PointData>& pointsData = data.pointsData;

    //! NOTE: The first and last segments run to/from the points just outside the region (if any), which can be in other polylines...
    const AutomationCurve& curve = automationData()->curve(currentCurveKey(staff));
    const int tickBeforeRegion = tickOfPointBefore(curve, data.startMeasure->tick().ticks());
    const int tickAfterRegion = tickOfPointAfter(curve, regionLastTick(data.endMeasure));

    QVector<QColor> colorsUnderLine;
    colorsUnderLine.reserve(pointsData.size() + 1); // +1 for the "trailing color" (see below)

    bool prevPointGenerated = tickBeforeRegion == -1 || curve.find(tickBeforeRegion)->second.generated;
    for (const PointData& pointData : pointsData) {
        //! NOTE: The following can be null for newly created (always non-generated) points because they're not in the model yet
        const mu::engraving::AutomationPoint* automationPoint = automationPointAt(key, pointData.tick);
        const bool currPointGenerated = automationPoint && automationPoint->generated;

        // Colors either side of an edited point should use the "edited color"...
        const bool useEditedColor = !prevPointGenerated || !currPointGenerated;
        colorsUnderLine.emplace_back(useEditedColor ? editedColor : generatedColor);

        prevPointGenerated = currPointGenerated;
    }

    // This is the trailing color (after the last point) - it follows the last point, as well as the point after the region...
    const bool nextPointGenerated = tickAfterRegion == -1 || curve.find(tickAfterRegion)->second.generated;
    colorsUnderLine.emplace_back(!prevPointGenerated || !nextPointGenerated ? editedColor : generatedColor);

    polyline->setColorsUnderLine(colorsUnderLine);
}

void NotationAutomationController::applyPolylineEdgeValues(PolylinePlot* polyline, const PolylineKey& key) const
{
    const PolylineData data = polylineData(key);
    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(polyline && data.polyline && staff && automationData()) {
        return;
    }

    const AutomationType type = currentAutomationType();
    const AutomationCurve& curve = automationData()->curve(currentCurveKey(staff));

    //! NOTE: The curve carries on beyond the region, so the line at the region's edges is taken from the points either side
    //! of them (which may belong to other polylines) rather than from the points inside the region...
    const int startTick = data.startMeasure->tick().ticks();
    const int lastTick = regionLastTick(data.endMeasure);
    const int endTick = data.endMeasure->endTick().ticks();

    if (curve.empty()) {
        polyline->setLeftEdgeValue(polyline->defaultValue());
        polyline->setRightEdgeValue(polyline->defaultValue());
        return;
    }

    // Left edge - between the last point before the region and the first point in or after it...
    const int tickBeforeStart = tickOfPointBefore(curve, startTick);
    const int tickAtOrAfterStart = tickOfPointAfter(curve, startTick - 1);
    const double left = curveValueBetween(curve, tickBeforeStart, tickAtOrAfterStart, startTick);

    // Right edge - between the last point in or before the region and the first point after it...
    const int tickAtOrBeforeLast = tickOfPointBefore(curve, lastTick + 1);
    const int tickAfterLast = tickOfPointAfter(curve, lastTick);
    const double right = curveValueBetween(curve, tickAtOrBeforeLast, tickAfterLast, endTick);

    // Higher value == lower Y...
    const bool isMuseSamplerAtStart = isMuseSamplerDynamics(staff, startTick);
    const bool isMuseSamplerAtEnd = isMuseSamplerDynamics(staff, endTick);
    polyline->setLeftEdgeValue(1.0 - NotationAutomationDisplayMapping::toDisplay(type, isMuseSamplerAtStart, left));
    polyline->setRightEdgeValue(1.0 - NotationAutomationDisplayMapping::toDisplay(type, isMuseSamplerAtEnd, right));
}

QColor NotationAutomationController::inversionRelativeColor(const muse::ui::ThemeStyleKey& key) const
{
    // This method is necessary because automation colors are relative to the score inversion as opposed to the current UI theme. In an
    // inverted score we use dark theme colors, and in a non-inverted score we use light theme colors...

    // TODO: High contrast colors should actually be fully customizable (issue #34154)
    const bool isHighContrast = uiConfiguration()->isHighContrast();
    const muse::ui::ThemeCode lightTheme = isHighContrast ? muse::ui::HIGH_CONTRAST_WHITE_THEME_CODE : muse::ui::LIGHT_THEME_CODE;
    const muse::ui::ThemeCode darkTheme = isHighContrast ? muse::ui::HIGH_CONTRAST_BLACK_THEME_CODE : muse::ui::DARK_THEME_CODE;

    const bool inverted = notationConfiguration()->shouldInvertScore();

    const muse::ui::ThemeList& themes = uiConfiguration()->themes();
    for (const muse::ui::ThemeInfo& theme : themes) {
        // Set line colors based on score inversion as opposed to current UI themes...
        const bool foundLightTheme = !inverted && theme.codeKey == lightTheme;
        const bool foundDarkTheme = inverted && theme.codeKey == darkTheme;
        if (foundLightTheme || foundDarkTheme) {
            return theme.values[key].toString();
        }
    }

    ASSERT_X("Error scanning themes");

    return QColor();
}

void NotationAutomationController::updatePolylinesGeometry()
{
    const bool visible = automation() && automation()->isAutomationModeEnabled();

    for (const auto& [key, data] : m_polylinesDataMap) {
        PolylinePlot* polyline = data.polyline;
        IF_ASSERT_FAILED(key.isValid() && polyline) {
            continue;
        }

        const muse::RectF regionRect = regionCanvasRect(key.system, key.staffIdx, data.startMeasure, data.endMeasure);
        const bool shown = visible && regionRect.isValid();
        polyline->setVisible(shown);
        if (!shown) {
            if (visible) {
                ASSERT_X("Invalid region rect");
            }
            continue;
        }

        //! NOTE: Here we should only update properties of the polyline that change relative to the view matrix. Polyline points are
        //! placed relative to the polylines themselves, and thus do not need to be modified in here...
        const muse::RectF viewRect = m_viewMatrix.map(regionRect);

        polyline->setWidth(viewRect.width());
        polyline->setHeight(viewRect.height());
        polyline->setX(viewRect.x());
        polyline->setY(viewRect.y());

        applyPolylineColors(polyline, key);
    }
}

void NotationAutomationController::updatePolylinesColors()
{
    for (const auto& [key, data] : m_polylinesDataMap) {
        PolylinePlot* polyline = data.polyline;
        IF_ASSERT_FAILED(key.isValid() && polyline) {
            continue;
        }
        applyPolylineColors(polyline, key);
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
    m_pendingScoreState = PendingScoreState();
    rebuildAllPolylines();

    if (automationData()) {
        automationData()->changed().onReceive(this, [this](const mu::engraving::AutomationChanges& changes) {
            mergePendingChanges(changes);
            scheduleUpdate();
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }

    if (score()) {
        score()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChanges& changes) {
            mergePendingScoreChanges(changes);
            scheduleUpdate();
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }

    if (currentNotation()) {
        currentNotation()->viewModeChanged().onNotify(this, [this]() {
            rebuildAllPolylines();
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }

    const project::INotationProjectPtr notationProject = projectSession()->project();
    if (const project::IProjectAudioSettingsPtr settings = notationProject ? notationProject->audioSettings() : nullptr) {
        // The Dynamics display range depends on the track's sound
        settings->trackInputParamsChanged().onReceive(this, [this](const InstrumentTrackId&) {
            if (currentAutomationType() != AutomationType::Dynamics) {
                return;
            }
            m_pendingScoreState.hasChanges = true;
            m_pendingScoreState.structural = true;
            scheduleUpdate();
        }, Asyncable::Mode::SetReplace /* FIXME */);
    }
}

void NotationAutomationController::mergePendingScoreChanges(const mu::engraving::ScoreChanges& changes)
{
    const bool firstChange = !m_pendingScoreState.hasChanges;
    m_pendingScoreState.hasChanges = true;
    m_pendingScoreState.structural = m_pendingScoreState.structural || isStructuralChange(changes);

    if (!changes.isValidBoundary()) {
        m_pendingScoreState.boundary = std::nullopt;
        return;
    }
    if (!firstChange && !m_pendingScoreState.boundary) {
        return;
    }

    const TickStaffRange changeRange { changes.tickFrom, changes.tickTo, changes.staffIdxFrom, changes.staffIdxTo };
    TickStaffRange range = m_pendingScoreState.boundary.value_or(changeRange);
    range.tickFrom = std::min(range.tickFrom, changeRange.tickFrom);
    range.tickTo = std::max(range.tickTo, changeRange.tickTo);
    range.staffIdxFrom = std::min(range.staffIdxFrom, changeRange.staffIdxFrom);
    range.staffIdxTo = std::max(range.staffIdxTo, changeRange.staffIdxTo);
    m_pendingScoreState.boundary = range;
}

void NotationAutomationController::scheduleUpdate()
{
    if (m_updateScheduled) {
        return;
    }
    m_updateScheduled = true;

    muse::async::Async::call(this, [this]() {
        m_updateScheduled = false;
        processPendingChanges();
    });
}

void NotationAutomationController::processPendingChanges()
{
    if (!m_pendingScoreState.hasChanges && m_pendingChanges.isEmpty()) {
        return;
    }
    const PendingScoreState scoreState = m_pendingScoreState;
    m_pendingScoreState = PendingScoreState();

    const bool automationVisible = automation() && automation()->isAutomationModeEnabled();

    if (scoreState.structural) {
        if (!automationVisible) {
            // Nothing visible right now; defer the rebuild until automation mode is enabled again
            m_pendingChanges.isFullReset = true;
            return;
        }
        rebuildAllPolylines();
        m_pendingChanges.clear();
        return;
    }

    if (!automationVisible) {
        // Nothing visible right now; m_pendingChanges keeps accumulating for next time
        return;
    }

    if (!m_pendingChanges.isEmpty()) {
        applyAutomationChanges(m_pendingChanges);
        m_pendingChanges.clear();
        return;
    }

    // No automation-data change and nothing structural - just layout drift
    // (e.g. measure widths shifted); refresh point positions using the batch's own range
    for (const auto& [key, data] : m_polylinesDataMap) {
        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }
        const Staff* staff = score()->staff(key.staffIdx);
        if (!staff) {
            continue;
        }

        const int regionStartTick = data.startMeasure->tick().ticks();
        const int regionEndTick = data.endMeasure->endTick().ticks();
        if (scoreState.boundary) {
            const TickStaffRange& range = *scoreState.boundary;
            if (staff->idx() < range.staffIdxFrom || staff->idx() > range.staffIdxTo) {
                continue;
            }
            if (regionEndTick < range.tickFrom || regionStartTick > range.tickTo) {
                continue;
            }
        }

        rebuildPolylinePoints(key);
    }

    updatePolylinesGeometry();
}

void NotationAutomationController::rebuildAllPolylines()
{
    m_currentlyEditedPolyline = nullptr;

    // TODO: More efficient if we don't clear/recreate the polylines every time...
    for (const auto& [key, data] : m_polylinesDataMap) {
        delete data.polyline;
    }
    m_polylinesDataMap.clear();

    if (!score()) {
        // Happens on close...
        return;
    }

    for (const System* system : score()->systems()) {
        buildAndAddPolylinesForSystem(system);
    }

    updatePolylinesGeometry();
}

void NotationAutomationController::rebuildPolylinePoints(const PolylineKey& key)
{
    const PolylineData data = polylineData(key);
    PolylinePlot* polyline = data.polyline;
    IF_ASSERT_FAILED(key.isValid() && polyline) {
        return;
    }

    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    const muse::RectF regionRect = staff ? regionCanvasRect(key.system, key.staffIdx, data.startMeasure, data.endMeasure) : muse::RectF();
    IF_ASSERT_FAILED(staff && regionRect.isValid()) {
        return;
    }

    //! NOTE: A polyline only holds the points inside its own region. Whatever lies outside it (which can still affect
    //! the polyline) is covered by the edge values...
    const QVector<PointData> pointsData = pointsDataInStaffRegion(staff, regionRect, data.startMeasure->tick().ticks(),
                                                                  regionLastTick(data.endMeasure));
    setPolylinePointsData(key, pointsData);

    QVector<QPointF> points;
    points.reserve(pointsData.size());
    for (const PointData& pointData : pointsData) {
        points.push_back(pointData.qPointF);
    }
    polyline->setPoints(points);

    applyPolylineEdgeValues(polyline, key);
    applyPolylineColorsUnderLine(polyline, key);
    polyline->update();
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

    IF_ASSERT_FAILED(automationData()) {
        return;
    }

    std::set<muse::ID> affectedStaffIds;
    std::set<mu::engraving::InstrumentTrackId> affectedTrackIds;
    bool globalAffected = false;
    for (const mu::engraving::AutomationCurveKey& key : changes.affectedKeys) {
        if (const std::optional<muse::ID> staffId = key.staffId()) {
            affectedStaffIds.insert(*staffId);
        } else if (const std::optional<mu::engraving::InstrumentTrackId> trackId = key.trackId()) {
            affectedTrackIds.insert(*trackId);
        } else if (key.isGlobal()) {
            globalAffected = true;
        }
    }

    //! NOTE: A change can affect any polyline of an affected staff, even ones in other systems (the line between the
    //! changed points and the points either side of them can span many regions), so they're all rebuilt...
    for (const auto& [key, data] : m_polylinesDataMap) {
        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }
        const Staff* staff = score()->staff(key.staffIdx);
        if (!staff) {
            continue;
        }
        const bool staffAffected = affectedStaffIds.find(staff->id()) != affectedStaffIds.end();
        const mu::engraving::InstrumentTrackId staffTrackId { staff->part()->id(), staff->part()->instrumentId() };
        const bool trackAffected = affectedTrackIds.find(staffTrackId) != affectedTrackIds.end();
        const bool globalCurveAffected = globalAffected && currentCurveKey(staff).isGlobal();
        if (staffAffected || trackAffected || globalCurveAffected) {
            rebuildPolylinePoints(key);
        }
    }

    updatePolylinesGeometry();
}

bool NotationAutomationController::requestEditPoint(const PointData& oldPointData, const PolylineKey& key, qreal x, qreal y)
{
    // STEP 1 - Check that all of our parameters are valid...
    const PointData::PointType pointType = oldPointData.pointType;
    IF_ASSERT_FAILED(key.isValid() && pointType != PointData::PointType::UNKNOWN) {
        return false;
    }
    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    const PolylineData data = polylineData(key);
    IF_ASSERT_FAILED(data.polyline) {
        return false;
    }

    // STEP 2 - Determine the new tick value based on the x parameter...
    const int resolvedTick = tickFromXInPolyline(key, data, x);
    const int newTick = resolvedTick != -1 ? resolvedTick : oldPointData.tick;
    const bool tickChanged = newTick != oldPointData.tick;

    // STEP 3 - Fetch the point being edited...
    const mu::engraving::AutomationCurveKey curveKey = currentCurveKey(staff);

    const mu::engraving::AutomationCurve& curve = automationData()->curve(curveKey);
    const auto existingIt = curve.find(oldPointData.tick);
    IF_ASSERT_FAILED(existingIt != curve.end()) {
        return false;
    }
    const mu::engraving::AutomationPoint& existingPoint = existingIt->second;
    const mu::engraving::real_t existingInValue = mu::engraving::resolveInValue(curve, existingIt);

    //! NOTE: Point in/out values are rescaled to the display range - higher value == lower Y...
    const mu::engraving::real_t referenceValue = pointType == PointData::PointType::IN ? existingInValue : existingPoint.value.outValue;
    const bool isMuseSampler = isMuseSamplerDynamics(staff, newTick);
    const mu::engraving::real_t newValue = NotationAutomationDisplayMapping::fromDisplay(currentAutomationType(), isMuseSampler, 1.0 - y,
                                                                                         referenceValue);

    // STEP 4 - Update the point's value, and move it to the new tick if necessary...

    //! NOTE: Moving a BOTH point is the simplest case - we can update its value then simply change the tick.
    //! Moving IN/OUT points is slightly more complex. In this case we need to set the in/out values to be
    //! equal at oldTick (effectively converting the original point to a BOTH point) and create a new point
    //! at newTick...

    if (!tickChanged || pointType == PointData::PointType::BOTH) {
        mu::engraving::AutomationPoint editedPoint = existingPoint;
        const mu::engraving::AutomationPoint::Ease preservedEase
            = mu::engraving::ease(editedPoint).value_or(mu::engraving::AutomationPoint::Ease::none());
        if (pointType == PointData::PointType::IN) {
            // The user explicitly chose this arrival value; it no longer follows whatever precedes it
            editedPoint.value.inValue = mu::engraving::AutomationPoint::ExplicitArrival { newValue, preservedEase };
        } else if (pointType == PointData::PointType::BOTH) {
            editedPoint.value.outValue = newValue;
            editedPoint.value.inValue = mu::engraving::AutomationPoint::ExplicitArrival { editedPoint.value.outValue, preservedEase };
        } else {
            editedPoint.value.outValue = newValue;
        }
        editedPoint.generated = false;

        mu::engraving::AutomationPointEdits edits {
            { newTick, MovePoint { editedPoint, oldPointData.tick } }
        };

        editAutomationPoints(curveKey, edits);

        return true;
    }

    // oldTick becomes a flat BOTH point, so its inValue is frozen to outValue at edit time (it no
    // longer live-tracks outValue if it's edited again later)
    const mu::engraving::AutomationPoint::Ease originalEase
        = mu::engraving::ease(existingPoint).value_or(mu::engraving::AutomationPoint::Ease::none());

    mu::engraving::AutomationPoint updatedOldPoint = existingPoint;
    if (pointType == PointData::PointType::OUT) {
        updatedOldPoint.value.outValue = existingInValue;
    }
    updatedOldPoint.value.inValue = mu::engraving::AutomationPoint::ExplicitArrival { updatedOldPoint.value.outValue, originalEase };
    updatedOldPoint.generated = false;

    mu::engraving::AutomationPoint newPoint;
    newPoint.value.outValue = newValue;
    newPoint.value.inValue = mu::engraving::AutomationPoint::ExplicitArrival { newPoint.value.outValue, originalEase };
    newPoint.itemId = existingPoint.itemId;

    mu::engraving::AutomationPointEdits edits {
        { oldPointData.tick, SetPoint { updatedOldPoint } },
        { newTick, SetPoint { newPoint } }
    };

    editAutomationPoints(curveKey, edits);

    return true;
}

bool NotationAutomationController::requestAddPoint(const PolylineKey& key, qreal x, qreal y, bool useCurveValue)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return false;
    }

    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    const PolylineData data = polylineData(key);
    IF_ASSERT_FAILED(data.polyline) {
        return false;
    }

    //! NOTE: No need to clamp x here - tickFromXInPolyline can't resolve to a tick outside the region...
    const int newTick = tickFromXInPolyline(key, data, x);
    if (newTick == -1) {
        return false;
    }

    const mu::engraving::AutomationCurveKey curveKey = currentCurveKey(staff);
    const mu::engraving::AutomationCurve& curve = automationData()->curve(curveKey);

    //! A click lands on the drawn line, which may be clamped, so keep the curve's own value. A dragged point
    //! is placed wherever the user dropped it, so its y is used instead...
    const std::optional<muse::real_t> curveValue = useCurveValue ? mu::engraving::evaluateCurveAt(curve, newTick) : std::nullopt;

    const bool isMuseSampler = isMuseSamplerDynamics(staff, newTick);

    mu::engraving::AutomationPoint newPoint;
    newPoint.value.outValue = curveValue ? *curveValue
                              : NotationAutomationDisplayMapping::fromDisplay(currentAutomationType(), isMuseSampler, 1.0 - y);
    newPoint.value.inValue = mu::engraving::AutomationPoint::ExplicitArrival { newPoint.value.outValue,
                                                                               mu::engraving::AutomationPoint::Ease::none() };
    newPoint.generated = false;

    mu::engraving::AutomationPointEdits edits {
        { newTick, SetPoint { newPoint } }
    };

    editAutomationPoints(curveKey, edits);

    return true;
}

bool NotationAutomationController::requestRemovePoint(const PointData& pointData, const PolylineKey& key)
{
    IF_ASSERT_FAILED(key.isValid()) {
        return false;
    }

    const mu::engraving::AutomationPoint* automationPoint = automationPointAt(key, pointData.tick);
    if (!automationPoint || automationPoint->generated || automationPoint->itemId.has_value()) {
        return false;
    }

    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff) {
        return false;
    }

    const mu::engraving::AutomationCurveKey curveKey = currentCurveKey(staff);

    mu::engraving::AutomationPointEdits edits {
        { pointData.tick, ErasePoint {} }
    };

    editAutomationPoints(curveKey, edits);

    return true;
}

void NotationAutomationController::editAutomationPoints(const mu::engraving::AutomationCurveKey& key,
                                                        mu::engraving::AutomationPointEdits& edits)
{
    const INotationAutomationPtr notationAutomation = automation();
    IF_ASSERT_FAILED(notationAutomation) {
        return;
    }

    notationAutomation->editPoints(key, edits);
}

PolylineData NotationAutomationController::polylineData(const PolylineKey& key) const
{
    const auto it = m_polylinesDataMap.find(key);
    IF_ASSERT_FAILED(it != m_polylinesDataMap.end()) {
        return PolylineData();
    }

    return it->second;
}

void NotationAutomationController::setPolylinePointsData(const PolylineKey& key, const QVector<PointData>& pointsData)
{
    const auto it = m_polylinesDataMap.find(key);
    IF_ASSERT_FAILED(it != m_polylinesDataMap.end()) {
        return;
    }

    it->second.pointsData = pointsData;
}

const mu::engraving::AutomationPoint* NotationAutomationController::automationPointAt(const PolylineKey& key, int tick) const
{
    const Staff* staff = score() ? score()->staff(key.staffIdx) : nullptr;
    IF_ASSERT_FAILED(staff && automationData()) {
        return nullptr;
    }

    const mu::engraving::AutomationCurveKey curveKey = currentCurveKey(staff);
    const mu::engraving::AutomationCurve& curve = automationData()->curve(curveKey);
    const auto it = curve.find(tick);
    if (it == curve.end()) {
        return nullptr;
    }

    return &it->second;
}

bool NotationAutomationController::isMuseSamplerDynamics(const Staff* staff, int tick) const
{
    if (currentAutomationType() != AutomationType::Dynamics) {
        return false;
    }

    const project::INotationProjectPtr notationProject = projectSession()->project();
    const project::IProjectAudioSettingsPtr settings = notationProject ? notationProject->audioSettings() : nullptr;
    const Part* part = staff ? staff->part() : nullptr;
    if (!settings || !part) {
        return false;
    }

    const InstrumentTrackId trackId { part->id(), part->instrumentId(Fraction::fromTicks(tick)) };
    return muse::audio::isResourceType(settings->trackInputParams(trackId).resourceMeta,
                                       muse::audio::AudioResourceType::MuseSamplerSoundPack);
}

AutomationType NotationAutomationController::currentAutomationType() const
{
    return automationTypeOf(notationConfiguration()->currentAutomationLane());
}

AutomationCurveKey NotationAutomationController::currentCurveKey(const Staff* staff) const
{
    return curveKeyFor(notationConfiguration()->currentAutomationLane(), staff);
}

INotationAutomationPtr NotationAutomationController::automation() const
{
    const IMasterNotationPtr masterNotation = projectSession()->masterNotation();
    return masterNotation ? masterNotation->automation() : nullptr;
}

INotationPtr NotationAutomationController::currentNotation() const
{
    return projectSession()->currentNotation();
}

mu::engraving::AutomationDataConstPtr NotationAutomationController::automationData() const
{
    const INotationAutomationPtr notationAutomation = automation();
    return notationAutomation ? notationAutomation->automationData() : nullptr;
}

mu::engraving::Score* NotationAutomationController::score() const
{
    return currentNotation() ? currentNotation()->elements()->msScore() : nullptr;
}
