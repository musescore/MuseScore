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

#ifndef KD_TABBAR_QUICK_P_H
#define KD_TABBAR_QUICK_P_H
#pragma once

#include "View.h"
#include "kddockwidgets/core/views/TabBarViewInterface.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QHash>

namespace KDDockWidgets {

class DockWidgetInstantiator;

namespace Core {
class TabBar;
}

namespace QtQuick {

class Stack;
class DockWidgetModel;

class DOCKS_EXPORT TabBar : public QtQuick::View, public Core::TabBarViewInterface
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *tabBarQmlItem READ tabBarQmlItem WRITE setTabBarQmlItem NOTIFY
                   tabBarQmlItemChanged)
    Q_PROPERTY(bool tabBarAutoHide READ tabBarAutoHide NOTIFY tabBarAutoHideChanged)
    Q_PROPERTY(DockWidgetModel *dockWidgetModel READ dockWidgetModel CONSTANT)
    Q_PROPERTY(int hoveredTabIndex READ hoveredTabIndex NOTIFY hoveredTabIndexChanged)
public:
    explicit TabBar(Core::TabBar *controller, QQuickItem *parent = nullptr);
    ~TabBar() override;

    DockWidgetModel *dockWidgetModel() const;

    int tabAt(QPoint localPos) const override;
    QQuickItem *tabBarQmlItem() const;
    void setTabBarQmlItem(QQuickItem *);

    QString text(int index) const override;
    QRect rectForTab(int index) const override;
    QRect globalRectForTab(int index) const;
    int indexForTabPos(QPoint) const;

    void moveTabTo(int from, int to) override;
    Q_INVOKABLE void setCurrentIndex(int index) override;

    /// Closes the dock widget at the specified index
    /// Returns true on success

    Q_INVOKABLE bool closeAtIndex(int index);

    Stack *stackView() const;
    void renameTab(int index, const QString &) override;
    void changeTabIcon(int index, const QIcon &icon) override;
    /// Returns whether the tab bar should hide when there's only 1 tab visible
    /// Default true, unless Flag_HideTitleBarWhenTabsVisible
    bool tabBarAutoHide() const;

    void removeDockWidget(Core::DockWidget *) override;
    void insertDockWidget(int index, Core::DockWidget *, const QIcon &,
                          const QString &title) override;

    /// @brief Returns the index of the currently hovered tab
    /// In case you want to style them differently in QML
    int hoveredTabIndex() const;

    Q_INVOKABLE void addDockWidgetAsTab(QQuickItem *other,
                                        KDDockWidgets::InitialVisibilityOption = {});

Q_SIGNALS:
    void tabBarQmlItemChanged();
    void tabBarAutoHideChanged();

    /// @brief Emitted when the hovered tab changes
    /// In case you want to style it differently
    void hoveredTabIndexChanged(int index);

protected:
    bool event(QEvent *ev) override;
    void init() override final;
    void onHoverEvent(QHoverEvent *, QPoint globalPos) override;

private:
    QQuickItem *tabAt(int index) const;
    void setHoveredTabIndex(int);

    class Private;
    Private *const d;
};

class DockWidgetModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum Role {
        Role_Title = Qt::UserRole
    };

    explicit DockWidgetModel(Core::TabBar *, QObject *parent);
    ~DockWidgetModel() override;

    int count() const;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Core::DockWidget *dockWidgetAt(int index) const;
    Core::DockWidget *currentDockWidget() const;
    void setCurrentDockWidget(Core::DockWidget *);
    void remove(Core::DockWidget *);
    int indexOf(const Core::DockWidget *);
    bool insert(Core::DockWidget *dw, int index);
    bool contains(Core::DockWidget *dw) const;
    int currentIndex() const;
    void setCurrentIndex(int index);

protected:
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void countChanged();
    void dockWidgetRemoved();

private:
    void emitDataChangedFor(Core::DockWidget *);

    class Private;
    Private *const d;
};

}

}

#endif
