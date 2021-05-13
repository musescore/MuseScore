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

#ifndef MU_DOCK_DOCKTOOLBAR_H
#define MU_DOCK_DOCKTOOLBAR_H

#include "internal/dockbase.h"

namespace mu::dock {
class DockToolBar : public DockBase
{
    Q_OBJECT

    Q_PROPERTY(bool movable READ movable WRITE setMovable NOTIFY movableChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

    Q_PROPERTY(
        QSize horizontalPreferredSize READ horizontalPreferredSize WRITE setHorizontalPreferredSize NOTIFY horizontalPreferredSizeChanged)
    Q_PROPERTY(QSize verticalPreferredSize READ verticalPreferredSize WRITE setVerticalPreferredSize NOTIFY verticalPreferredSizeChanged)

    Q_PROPERTY(DockToolBar * tabifyToolBar READ tabifyToolBar WRITE setTabifyToolBar NOTIFY tabifyToolBarChanged)

public:
    explicit DockToolBar(QQuickItem* parent = nullptr);

    bool movable() const;
    Qt::Orientation orientation() const;

    Q_INVOKABLE void setDraggableMouseArea(QQuickItem* mouseArea);

    QSize horizontalPreferredSize() const;
    QSize verticalPreferredSize() const;
    DockToolBar* tabifyToolBar() const;

public slots:
    void setMinimumWidth(int width) override;
    void setMinimumHeight(int height) override;
    void setMaximumWidth(int width) override;
    void setMaximumHeight(int height) override;

    void setMovable(bool movable);
    void setOrientation(Qt::Orientation orientation);
    void setHorizontalPreferredSize(QSize horizontalPreferredSize);
    void setVerticalPreferredSize(QSize verticalPreferredSize);
    void setTabifyToolBar(DockToolBar* tabifyToolBar);

signals:
    void movableChanged(bool movable);
    void orientationChanged(Qt::Orientation orientation);
    void horizontalPreferredSizeChanged(QSize horizontalPreferredSize);
    void verticalPreferredSizeChanged(QSize verticalPreferredSize);
    void tabifyToolBarChanged(DockToolBar* tabifyToolBar);

private slots:
    void updateOrientation();

private:
    void componentComplete() override;

    DockType type() const override;

    QSize preferredSize() const;

    class DraggableArea;
    DraggableArea* m_draggableArea = nullptr;

    bool m_movable = true;
    Qt::Orientation m_orientation = Qt::Horizontal;
    QSize m_horizontalPreferredSize;
    QSize m_verticalPreferredSize;
    DockToolBar* m_tabifyToolBar = nullptr;
};
}

#endif // MU_DOCK_DOCKTOOLBAR_H
