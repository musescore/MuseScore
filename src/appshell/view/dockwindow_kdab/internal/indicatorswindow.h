/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_DOCK_INDICATORSWINDOW_H
#define MU_DOCK_INDICATORSWINDOW_H

#include <QQuickView>

#include "thirdparty/KDDockWidgets/src/private/DropIndicatorOverlayInterface_p.h"

namespace mu::dock {
class IndicatorsWindow : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem* dropIndicators READ dropIndicators CONSTANT)

public:
    explicit IndicatorsWindow(QQuickItem* dropIndicators);

    QQuickItem* dropIndicators() const;
    Q_INVOKABLE QString iconName(int dropLocation, bool active) const;

    KDDockWidgets::DropIndicatorOverlayInterface::DropLocation dropLocationForPosition(const QPoint& pos) const;
    QPoint positionForIndicator(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation dropLocation) const;

private:
    KDDockWidgets::DropIndicatorOverlayInterface::DropLocation locationForIndicator(const QQuickItem* indicator) const;
    const QQuickItem* indicatorForLocation(KDDockWidgets::DropIndicatorOverlayInterface::DropLocation dropLocation) const;

    const QQuickItem* indicatorForPos(QPoint) const;
    QList<const QQuickItem *> indicatorItems() const;

    QQuickItem* m_dropIndicators = nullptr;
};
}

#endif // MU_DOCK_INDICATORSWINDOW_H
