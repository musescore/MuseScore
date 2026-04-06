/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "automationoverlay.h"
#include "framework/uicomponents/qml/Muse/UiComponents/polylineplot.h"

using namespace muse::automation;

AutomationOverlay::AutomationOverlay(QQuickItem* parent)
    : QQuickItem(parent)
{
    setClip(true);     // TODO: Not ideal - we'll still be rendering clipped points
}

void AutomationOverlay::initAutomationLinesData(const QVariant& automationLinesData)
{
    for (size_t i = 0; i < m_automationLinesData.size(); ++i) {
        delete m_automationLinesData.at(i).polyline;
    }
    m_automationLinesData.clear();

    const QVariantList automationDataList = automationLinesData.toList();
    if (automationDataList.isEmpty()) {
        update();
        return;
    }

    m_automationLinesData.reserve(automationDataList.size());

    for (qsizetype i = 0; i < automationDataList.size(); ++i) {
        const QVariantMap& lineDataMap = automationDataList.at(i).toMap();
        IF_ASSERT_FAILED(!lineDataMap.isEmpty()) {
            continue;
        }

        const QVariantList pointsDataList = lineDataMap.value("points").toList();
        if (pointsDataList.size() < 2) { // TODO: Maybe it can have less than 2 points?
            continue;
        }

        bool ok = false;
        QVector<QPointF> pointsList;
        pointsList.reserve(pointsDataList.size());
        for (const QVariant& pointData : pointsDataList) {
            const QVariantMap& pointMap = pointData.toMap();
            IF_ASSERT_FAILED(!pointMap.isEmpty()) {
                continue;
            }
            const qreal pointX = pointMap.value("x").toReal(&ok);
            const qreal pointY = ok ? pointMap.value("y").toReal(&ok) : 0.0;
            IF_ASSERT_FAILED(ok) {
                continue;
            }
            pointsList.emplaceBack(QPointF(pointX, pointY));
        }

        //! NOTE: Only set the points for now - we'll update the geometry separately...
        muse::uicomponents::PolylinePlot* polyline = new muse::uicomponents::PolylinePlot(this);
        polyline->setPoints(pointsList);
        polyline->setDrawBackground(false);

        QObject::connect(polyline, &muse::uicomponents::PolylinePlot::pointMoved,
                         [polyline](int index, qreal x, qreal y, bool completed) {
            IF_ASSERT_FAILED(index > -1 && index < static_cast<int>(polyline->points().size())) {
                return;
            }
            // TODO: When completed, change the model (create an undo action in the process)...
            UNUSED(completed);

            QVector<QPointF> points = polyline->points();
            points.replace(index, { x, y });
            polyline->setPoints(points);
            polyline->update();                  // TODO: pass update rect?
        });

        AutomationLineData lineData = { lineDataMap, polyline };
        m_automationLinesData.emplace_back(lineData);
    }

    updateAllPolylinesGeometry();
    update();
}

QVariant AutomationOverlay::viewMatrix() const
{
    return m_viewMatrix;
}

void AutomationOverlay::setViewMatrix(const QVariant& matrix)
{
    if (m_viewMatrix == matrix) {
        return;
    }
    m_viewMatrix = matrix.value<QTransform>();
    emit viewMatrixChanged();

    if (!m_automationLinesData.empty()) {
        updateAllPolylinesGeometry();
        update();
    }
}

bool AutomationOverlay::lineIndexIsValid(size_t index) const
{
    return index < m_automationLinesData.size();
}

void AutomationOverlay::updatePolylinesGeometry(size_t firstIndex, size_t lastIndex)
{
    IF_ASSERT_FAILED(lineIndexIsValid(firstIndex) && lineIndexIsValid(lastIndex)) {
        return;
    }

    //! NOTE: Any Polyline properties that depend on the view matrix should be updated in
    //! this method. Note that Polyline::setPoints is not called here as these are expressed
    //! locally (relative to the Polyline itself)...
    for (size_t i = firstIndex; i <= lastIndex; ++i) {
        AutomationLineData& lineData = m_automationLinesData.at(i);
        const QVariantMap& rawMap = lineData.rawLineDataMap;
        muse::uicomponents::PolylinePlot* polyline = lineData.polyline;
        IF_ASSERT_FAILED(!rawMap.isEmpty() && polyline) {
            continue;
        }

        bool ok = false;
        const qreal lineX = rawMap.value("x").toReal(&ok);
        const qreal lineY = ok ? rawMap.value("y").toReal(&ok) : 0.0;
        const qreal lineWidth = ok ? rawMap.value("width").toReal(&ok) : 0.0;
        const qreal lineHeight = ok ? rawMap.value("height").toReal(&ok) : 0.0;
        IF_ASSERT_FAILED(ok) {
            continue;
        }

        const QPointF topLeft(lineX, lineY);
        const QPointF topLeftMapped = m_viewMatrix.map(topLeft);
        polyline->setX(topLeftMapped.x());
        polyline->setY(topLeftMapped.y());
        polyline->setWidth(lineWidth * m_viewMatrix.m11());
        polyline->setHeight(lineHeight * m_viewMatrix.m22());
    }
}
