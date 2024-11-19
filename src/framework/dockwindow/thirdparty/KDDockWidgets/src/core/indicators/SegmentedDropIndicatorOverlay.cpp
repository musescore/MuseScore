/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "SegmentedDropIndicatorOverlay.h"
#include "core/DropArea.h"
#include "Config.h"
#include "core/ViewFactory.h"

#include <algorithm>
#include <cmath>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

int SegmentedDropIndicatorOverlay::s_segmentGirth = 50;
int SegmentedDropIndicatorOverlay::s_segmentPenWidth = 4;
double SegmentedDropIndicatorOverlay::s_draggedWindowOpacity = 0.7;
int SegmentedDropIndicatorOverlay::s_centralIndicatorMaxWidth = 300;
int SegmentedDropIndicatorOverlay::s_centralIndicatorMaxHeight = 160;

SegmentedDropIndicatorOverlay::SegmentedDropIndicatorOverlay(Core::DropArea *dropArea)
    : DropIndicatorOverlay(dropArea, Config::self().viewFactory()->createSegmentedDropIndicatorOverlayView(this, dropArea->view()))
{
    // If the app didn't choose opacity then we choose a suitable default value.
    // ClassicIndicators works fine with an opaque dragged window because the indicators have higher
    // Z, However for SegmentedIndicators the indicators are in the main window, so lower Z. Make
    // the dragged window translucent a bit, so we can see the indicators
    const bool userChoseOpacity = !std::isnan(Config::self().draggedWindowOpacity());
    if (!userChoseOpacity)
        Config::self().setDraggedWindowOpacity(s_draggedWindowOpacity);
}

SegmentedDropIndicatorOverlay::~SegmentedDropIndicatorOverlay() = default;

DropLocation SegmentedDropIndicatorOverlay::hover_impl(Point pt)
{
    m_hoveredPt = view()->mapFromGlobal(pt);
    updateSegments();
    setCurrentDropLocation(dropLocationForPos(m_hoveredPt));

    return currentDropLocation();
}

DropLocation SegmentedDropIndicatorOverlay::dropLocationForPos(Point pos) const
{
    for (const auto &m_segment : m_segments) {
        if (m_segment.second.containsPoint(pos, Qt::OddEvenFill)) {
            return m_segment.first;
        }
    }

    return DropLocation_None;
}

std::unordered_map<DropLocation, Polygon> SegmentedDropIndicatorOverlay::segmentsForRect(Rect r, bool inner,
                                                                                         bool useOffset) const
{
    const int halfPenWidth = s_segmentPenWidth / 2;

    const int l = s_segmentGirth;
    const int top = (r.y() == 0 && useOffset) ? l : r.y();
    const int left = (r.x() == 0 && useOffset) ? l : r.x();
    const int right = (rect().right() == r.right() && useOffset) ? r.right() - l : r.right();
    const int bottom = (rect().bottom() == r.bottom() && useOffset) ? r.bottom() - l : r.bottom();
    const Point topLeft = { left + halfPenWidth, top + halfPenWidth };
    const Point topRight = { right, top + halfPenWidth };
    const Point bottomLeft = { left + halfPenWidth, bottom };
    const Point bottomRight = { right, bottom };

    const Vector<Point> leftPoints = { topLeft, bottomLeft, Point(left, bottom) + Point(l, -l),
                                       topLeft + Point(l, l), topLeft };

    const Vector<Point> rightPoints = { topRight, bottomRight, bottomRight + Point(-l, -l),
                                        topRight + Point(-l, l) };

    const Vector<Point> topPoints = { topLeft, topRight, topRight + Point(-l, l),
                                      topLeft + Point(l, l) };

    const Vector<Point> bottomPoints = { bottomLeft, bottomRight, bottomRight + Point(-l, -l),
                                         bottomLeft + Point(l, -l) };

    if (inner) {
        Polygon bounds =
            Vector<Point> { topLeft + Point(l, l), topRight + Point(-l, l),
                            bottomRight + Point(-l, -l), bottomLeft + Point(l, -l) };
        const int maxWidth = bounds.boundingRect().width();
        const Point centerPos = bounds.boundingRect().center();

        // Build the center
        const int indicatorWidth = std::min(s_centralIndicatorMaxWidth, maxWidth - 100);
        const int indicatorHeight = std::min(s_centralIndicatorMaxHeight, int(indicatorWidth * 0.60));
        const int tabWidth = int(indicatorWidth * 0.267);
        const int tabHeight = int(indicatorHeight * 0.187);
        const int centerRectLeft = centerPos.x() - indicatorWidth / 2;
        const int centerRectRight = centerPos.x() + indicatorWidth / 2;
        const int centerRectBottom = centerPos.y() + indicatorHeight / 2;
        const int centerRectTop = centerPos.y() - indicatorHeight / 2;


        const auto center = Vector<Point> {
            { centerRectLeft, centerRectTop },
            { centerRectLeft + tabWidth, centerRectTop },
            { centerRectLeft + tabWidth, centerRectTop + tabHeight },
            { centerRectRight, centerRectTop + tabHeight },
            { centerRectRight, centerRectBottom },
            { centerRectLeft, centerRectBottom },
        };

        return { { DropLocation_Left, leftPoints },
                 { DropLocation_Top, topPoints },
                 { DropLocation_Right, rightPoints },
                 { DropLocation_Bottom, bottomPoints },
                 { DropLocation_Center, center } };
    }

    return { { DropLocation_OutterLeft, leftPoints },
             { DropLocation_OutterTop, topPoints },
             { DropLocation_OutterRight, rightPoints },
             { DropLocation_OutterBottom, bottomPoints } };
}

void SegmentedDropIndicatorOverlay::updateSegments()
{
    m_segments.clear();

    const auto outterSegments = segmentsForRect(rect(), /*inner=*/false);

    for (auto indicator : { DropLocation_OutterLeft, DropLocation_OutterRight,
                            DropLocation_OutterTop, DropLocation_OutterBottom }) {
        if (dropIndicatorVisible(indicator)) {
            auto it = outterSegments.find(indicator);
            const Polygon segment = it == outterSegments.cend() ? Polygon() : it->second;
            m_segments[indicator] = segment;
        }
    }

    const bool hasOutter = !m_segments.empty();
    const bool useOffset = hasOutter;
    const auto innerSegments = segmentsForRect(hoveredGroupRect(), /*inner=*/true, useOffset);

    for (auto indicator : { DropLocation_Left, DropLocation_Top, DropLocation_Right,
                            DropLocation_Bottom, DropLocation_Center }) {
        if (dropIndicatorVisible(indicator)) {
            auto it = innerSegments.find(indicator);
            const Polygon segment = it == innerSegments.cend() ? Polygon() : it->second;
            m_segments[indicator] = segment;
        }
    }

    view()->update();
}

Point SegmentedDropIndicatorOverlay::posForIndicator(DropLocation) const
{
    /// Doesn't apply to segmented indicators, completely different concept
    return {};
}

Point SegmentedDropIndicatorOverlay::hoveredPt() const
{
    return m_hoveredPt;
}

const std::unordered_map<DropLocation, Polygon> &SegmentedDropIndicatorOverlay::segments() const
{
    return m_segments;
}
