/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief Implements a QTabWidget derived class with support for docking and undocking
 * KDockWidget::DockWidget as tabs .
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#pragma once

#include "View.h"
#include "core/views/TabBarViewInterface.h"

#include "kdbindings/signal.h"

namespace KDDockWidgets::Core {
class TabBar;
}

namespace KDDockWidgets::flutter {

class DockWidget;
class TabWidget;

class DOCKS_EXPORT TabBar : public View, public Core::TabBarViewInterface
{
public:
    explicit TabBar(Core::TabBar *controller, Core::View *parent = nullptr);
    int tabAt(Point localPos) const override;

    QString text(int index) const override;
    Rect rectForTab(int index) const override;

    void moveTabTo(int from, int to) override;

    void changeTabIcon(int index, const Icon &icon) override;
    void removeDockWidget(Core::DockWidget *dw) override;
    void insertDockWidget(int index, Core::DockWidget *dw, const Icon &icon,
                          const QString &title) override;
    void renameTab(int index, const QString &name) override;
    void setCurrentIndex(int index) override;

    void onMousePress(MouseEvent *) override;

private:
    Core::TabBar *const m_controller;
};
}
