/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#ifndef MUSE_DOCK_DOCKTABBAR_H
#define MUSE_DOCK_DOCKTABBAR_H

#include <QQuickItem>

#include "kddockwidgets/src/qtquick/views/TabBar.h"

namespace KDDockWidgets::Core {
class TabBar;
}

namespace muse::dock {
class DockTabBar : public KDDockWidgets::QtQuick::TabBar
{
    Q_OBJECT

public:
    explicit DockTabBar(KDDockWidgets::Core::TabBar* controller, QQuickItem* parent = nullptr);

    // The following is a hack (see PR #29794)- revisit this system when updating KDDockWidgets
    Q_PROPERTY(bool tabChangedOnClick READ tabChangedOnClick CONSTANT)

    Q_INVOKABLE void setDraggableMouseArea(QQuickItem* mouseArea);
    Q_INVOKABLE void doubleClicked(QPoint pos) const;

    bool tabChangedOnClick() const { return m_tabChangedOnClick; }

protected:
    bool event(QEvent* event) override;

private:
    void onMousePressRelease(const QMouseEvent* mouseEvent);

    QQuickItem* m_draggableMouseArea = nullptr;
    int m_indexOfPressedTab = -1;

    bool m_tabChangedOnClick = false;
};
}

#endif // MUSE_DOCK_DOCKTABBAR_H
