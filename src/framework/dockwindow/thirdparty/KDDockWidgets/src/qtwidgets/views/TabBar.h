/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_TABBAR_QTWIDGETS_H
#define KD_TABBAR_QTWIDGETS_H

#pragma once

#include "View.h"
#include <kddockwidgets/core/views/TabBarViewInterface.h>

#include <QTabBar>

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QTabWidget;
QT_END_NAMESPACE

namespace KDDockWidgets::Core {
class TabBar;
class DockWidget;
}

namespace KDDockWidgets::QtWidgets {

class DOCKS_EXPORT TabBar : public View<QTabBar>, public Core::TabBarViewInterface
{
    Q_OBJECT
public:
    explicit TabBar(Core::TabBar *controller, QWidget *parent = nullptr);
    ~TabBar() override;

    Core::TabBar *tabBar() const;

    void setCurrentIndex(int index) override;

    QString text(int index) const override;
    QRect rectForTab(int index) const override;
    void moveTabTo(int from, int to) override;

    int tabAt(QPoint localPos) const override;
    void renameTab(int index, const QString &) override;
    void changeTabIcon(int index, const QIcon &icon) override;
    void removeDockWidget(Core::DockWidget *) override;
    void insertDockWidget(int index, Core::DockWidget *, const QIcon &,
                          const QString &title) override;
    QTabWidget *tabWidget() const;
    void setTabsAreMovable(bool) override;

Q_SIGNALS:
    void dockWidgetInserted(int index);
    void dockWidgetRemoved(int index);
    void countChanged();
    void currentDockWidgetChanged(KDDockWidgets::Core::DockWidget *);

protected:
    void init() override final;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    bool event(QEvent *) override;
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    class Private;
    Private *const d;
};

}

#endif
