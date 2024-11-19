/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "SegmentedDropIndicatorOverlay.h"
#include "kddockwidgets/core/indicators/SegmentedDropIndicatorOverlay.h"

#include <QPainter>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

QColor SegmentedDropIndicatorOverlay::s_segmentPenColor = Qt::black;
QColor SegmentedDropIndicatorOverlay::s_segmentBrushColor = QColor(0xbb, 0xd5, 0xee, /*alpha=*/200);
QColor SegmentedDropIndicatorOverlay::s_hoveredSegmentBrushColor = QColor(0x3574c5);

SegmentedDropIndicatorOverlay::SegmentedDropIndicatorOverlay(
    Core::SegmentedDropIndicatorOverlay *controller, QWidget *parent)
    : View<QWidget>(controller, Core::ViewType::None, parent)
    , m_controller(controller)
{
}

SegmentedDropIndicatorOverlay::~SegmentedDropIndicatorOverlay() = default;

void SegmentedDropIndicatorOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    drawSegments(&p);
}

void SegmentedDropIndicatorOverlay::drawSegments(QPainter *p)
{
    const std::unordered_map<DropLocation, QPolygon> &segments = m_controller->segments();
    for (DropLocation loc :
         { DropLocation_Left, DropLocation_Top, DropLocation_Right, DropLocation_Bottom,
           DropLocation_Center, DropLocation_OutterLeft, DropLocation_OutterTop,
           DropLocation_OutterRight, DropLocation_OutterBottom }) {
        auto it = segments.find(loc);
        const Polygon segment = it == segments.cend() ? Polygon() : it->second;
        drawSegment(p, segment);
    }
}

void SegmentedDropIndicatorOverlay::drawSegment(QPainter *p, const QPolygon &segment)
{
    if (segment.isEmpty())
        return;

    QPen pen(SegmentedDropIndicatorOverlay::s_segmentPenColor);
    pen.setWidth(Core::SegmentedDropIndicatorOverlay::s_segmentPenWidth);
    p->setPen(pen);
    QColor brush(SegmentedDropIndicatorOverlay::s_segmentBrushColor);

    if (segment.containsPoint(m_controller->hoveredPt(), Qt::OddEvenFill))
        brush = SegmentedDropIndicatorOverlay::s_hoveredSegmentBrushColor;

    p->setBrush(brush);
    p->drawPolygon(segment);
}
