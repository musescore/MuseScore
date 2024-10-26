/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kddockwidgets/QtCompat_p.h>
#include <kddockwidgets/core/DropIndicatorOverlay.h>

#include <unordered_map>

namespace KDDockWidgets {

namespace Core {

class DOCKS_EXPORT SegmentedDropIndicatorOverlay : public DropIndicatorOverlay
{
    Q_OBJECT
public:
    explicit SegmentedDropIndicatorOverlay(Core::DropArea *dropArea);
    ~SegmentedDropIndicatorOverlay() override;
    DropLocation hover_impl(Point globalPos) override;

    DropLocation dropLocationForPos(Point pos) const;
    Point hoveredPt() const;
    const std::unordered_map<DropLocation, Polygon> &segments() const;

    static int s_segmentGirth;
    static int s_segmentPenWidth;
    static int s_centralIndicatorMaxWidth;
    static int s_centralIndicatorMaxHeight;
    static double s_draggedWindowOpacity;

protected:
    Point posForIndicator(DropLocation) const override;

private:
    std::unordered_map<DropLocation, Polygon> segmentsForRect(Rect, bool inner, bool useOffset = false) const;
    void updateSegments();
    Point m_hoveredPt = {};
    std::unordered_map<DropLocation, Polygon> m_segments;
};

}

}
