/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief The QWidget counter part of TabWidgetWidget. Handles GUI while TabWidget handles state.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KDTABWIDGETWIDGET_P_H
#define KDTABWIDGETWIDGET_P_H

#include "../TabWidget_p.h"

#include <QTabWidget>

namespace KDDockWidgets {

class Frame;
class TabBar;

class DOCKS_EXPORT TabWidgetWidget
        : public QTabWidget
        , public TabWidget
{
    Q_OBJECT
public:
    explicit TabWidgetWidget(Frame *parent);

    TabBar *tabBar() const override;

    int numDockWidgets() const override;
    void removeDockWidget(DockWidgetBase *) override;
    int indexOfDockWidget(const DockWidgetBase *) const override;

Q_SIGNALS:
    void currentTabChanged(int index) override;
    void currentDockWidgetChanged(KDDockWidgets::DockWidgetBase *dw) override;
protected:
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;
    bool isPositionDraggable(QPoint p) const override;
    void setCurrentDockWidget(int index) override;
    bool insertDockWidget(int index, DockWidgetBase *, const QIcon&, const QString &title) override;
    void setTabBarAutoHide(bool) override;
    void renameTab(int index, const QString &) override;
    void changeTabIcon(int index, const QIcon &) override;

    DockWidgetBase *dockwidgetAt(int index) const override;
    int currentIndex() const override;

private:
    Q_DISABLE_COPY(TabWidgetWidget)
    TabBar *const m_tabBar;
};
}

#endif
