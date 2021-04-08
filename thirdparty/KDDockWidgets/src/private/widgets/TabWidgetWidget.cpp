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

#include "TabWidgetWidget_p.h"
#include "Frame_p.h"
#include "Config.h"
#include "FrameworkWidgetFactory.h"

#include <QMouseEvent>
#include <QTabBar>

using namespace KDDockWidgets;

TabWidgetWidget::TabWidgetWidget(Frame *parent)
    : QTabWidget(parent)
    , TabWidget(this, parent)
    , m_tabBar(Config::self().frameworkWidgetFactory()->createTabBar(this))
{
    setTabBar(static_cast<QTabBar*>(m_tabBar->asWidget()));
    setTabsClosable(Config::self().flags() & Config::Flag_TabsHaveCloseButton);

    // In case tabs closable is set by the factory, a tabClosedRequested() is emitted when the user presses [x]
    connect(this, &QTabWidget::tabCloseRequested, this, [this] (int index) {
        if (DockWidgetBase *dw = dockwidgetAt(index)) {
            if (dw->options() & DockWidgetBase::Option_NotClosable) {
                qWarning() << "QTabWidget::tabCloseRequested: Refusing to close dock widget with Option_NotClosable option. name=" << dw->uniqueName();
            } else {
                dw->close();
            }
        } else {
            qWarning() << "QTabWidget::tabCloseRequested Couldn't find dock widget for index" << index << "; count=" << count();
        }
    });

    connect(this, &QTabWidget::currentChanged, this, [this] (int index) {
        onCurrentTabChanged(index);
        Q_EMIT currentTabChanged(index);
        Q_EMIT currentDockWidgetChanged(currentDockWidget());
    });
}

TabBar *TabWidgetWidget::tabBar() const
{
    return m_tabBar;
}

int TabWidgetWidget::numDockWidgets() const
{
    return count();
}

void TabWidgetWidget::removeDockWidget(DockWidgetBase *dw)
{
    removeTab(indexOf(dw));
}

int TabWidgetWidget::indexOfDockWidget(const DockWidgetBase *dw) const
{
    return indexOf(const_cast<DockWidgetBase*>(dw));
}

void TabWidgetWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (onMouseDoubleClick(ev->pos())) {
        ev->accept();
    } else {
        ev->ignore();
    }
}

void TabWidgetWidget::mousePressEvent(QMouseEvent *ev)
{
    QTabWidget::mousePressEvent(ev);

    if ((Config::self().flags() & Config::Flag_TitleBarIsFocusable) && !frame()->isFocused()) {
        // User clicked on the tab widget itself
        frame()->FocusScope::focus(Qt::MouseFocusReason);
    }
}

void TabWidgetWidget::tabInserted(int)
{
    onTabInserted();
}

void TabWidgetWidget::tabRemoved(int)
{
    onTabRemoved();
}

bool TabWidgetWidget::isPositionDraggable(QPoint p) const
{
    if (tabPosition() != QTabWidget::North) {
        qWarning() << Q_FUNC_INFO << "Not implemented yet. Only North is supported";
        return false;
    }

    return p.y() >= 0 && p.y() <= QTabWidget::tabBar()->height();
}

void TabWidgetWidget::setCurrentDockWidget(int index)
{
    setCurrentIndex(index);
}

bool TabWidgetWidget::insertDockWidget(int index, DockWidgetBase *dw,
                                       const QIcon &icon, const QString &title)
{
    insertTab(index, dw, icon, title);
    return true;
}

void TabWidgetWidget::setTabBarAutoHide(bool b)
{
    QTabWidget::setTabBarAutoHide(b);
}

void TabWidgetWidget::renameTab(int index, const QString &text)
{
    setTabText(index, text);
}

void TabWidgetWidget::changeTabIcon(int index, const QIcon &icon)
{
    setTabIcon(index, icon);
}

DockWidgetBase *TabWidgetWidget::dockwidgetAt(int index) const
{
    return qobject_cast<DockWidgetBase *>(widget(index));
}

int TabWidgetWidget::currentIndex() const
{
    return QTabWidget::currentIndex();
}
