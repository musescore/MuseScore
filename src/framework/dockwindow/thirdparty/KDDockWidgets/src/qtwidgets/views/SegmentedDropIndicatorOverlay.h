/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_SEGMENTEDDROPINDICATOROVERLAY_QTWIDGETS_H
#define KD_SEGMENTEDDROPINDICATOROVERLAY_QTWIDGETS_H

#pragma once

#include "View.h"

namespace KDDockWidgets {

namespace Core {
class SegmentedDropIndicatorOverlay;
}

namespace QtWidgets {

class DOCKS_EXPORT SegmentedDropIndicatorOverlay : public View<QWidget>
{
    Q_OBJECT
public:
    explicit SegmentedDropIndicatorOverlay(Core::SegmentedDropIndicatorOverlay *controller,
                                           QWidget *parent = nullptr);
    ~SegmentedDropIndicatorOverlay() override;

    static QColor s_segmentPenColor;
    static QColor s_segmentBrushColor;
    static QColor s_hoveredSegmentBrushColor;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void drawSegments(QPainter *p);
    void drawSegment(QPainter *p, const QPolygon &segment);
    Core::SegmentedDropIndicatorOverlay *const m_controller;
};

}

}

#endif
