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

#ifndef MU_DOCK_DOCKTOOLBARVIEW_H
#define MU_DOCK_DOCKTOOLBARVIEW_H

#include "internal/dockbase.h"

namespace mu::dock {
class DockToolBarView : public DockBase
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

public:
    explicit DockToolBarView(QQuickItem* parent = nullptr);

    Qt::Orientation orientation() const;

    Q_INVOKABLE void setDraggableMouseArea(QQuickItem* mouseArea);

public slots:
    void setOrientation(Qt::Orientation orientation);

signals:
    void orientationChanged(Qt::Orientation orientation);

protected:
    void componentComplete() override;

    DockType type() const override;

    static const int MIN_SIDE_SIZE;
    static const int MAX_SIDE_SIZE;

private:
    bool isOrientationChangingAllowed() const;

    void applySizeConstraintsByOrientation();

    class DraggableArea;
    DraggableArea* m_draggableArea = nullptr;

    bool m_movable = true;
    Qt::Orientation m_orientation = Qt::Horizontal;
};
}

#endif // MU_DOCK_DOCKTOOLBARVIEW_H
