/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Controller.h"
#include "Draggable_p.h"
#include "Group.h"
#include "TabBar.h"

namespace KDDockWidgets {

namespace QtWidgets {
class Stack;
}

namespace QtQuick {
class TabBar;
}

namespace Core {

class Group;
class TabBar;

class DOCKS_EXPORT Stack : public Controller, public Draggable
{
    Q_OBJECT
public:
    explicit Stack(Group *, StackOptions);
    virtual ~Stack() override;

    /**
     * @brief returns the number of dock widgets in this TabWidget
     */
    int numDockWidgets() const;

    /// Sets whether the tab bar should be hidden when there's only 1 tab shown
    /// Default is true.
    /// See Config::Flag_AlwaysShowTabs to change this behaviour
    void setTabBarAutoHide(bool);
    bool tabBarAutoHide() const;

    ///@brief appends a dock widget into this TabWidget
    void addDockWidget(DockWidget *);

    /**
     * @brief inserts @p dockwidget into the TabWidget, at @p index
     * @param dockwidget the dockwidget to insert
     * @param index The index to where to put it
     */
    bool insertDockWidget(DockWidget *dockwidget, int index);

    /**
     * @brief Returns whether dockwidget @p dw is contained in this tab widget
     * Equivalent to indexOf(dw) != -1
     */
    bool contains(DockWidget *dw) const;

    /**
     * @brief Returns the tab bar
     */
    Core::TabBar *tabBar() const;

    ///@brief getter for the group
    Group *group() const;

    // Draggable interface
    std::unique_ptr<WindowBeingDragged> makeWindow() override;
    DockWidget *singleDockWidget() const override final;
    bool isWindow() const override;
    bool isMDI() const override;
    bool isPositionDraggable(Point p) const override;

    StackOptions options() const;

    /// @brief Enables document mode. Default is false.
    void setDocumentMode(bool);

    /// The specified buttons, if disabled, will be hidden as well
    /// for example, with non-closable dock widgets we disable the close button
    /// this allows to hide it as well.
    /// Only relevant with Flag_ShowButtonsOnTabBarIfTitleBarHidden
    void setHideDisabledButtons(TitleBarButtonTypes);
    bool buttonHidesIfDisabled(TitleBarButtonType) const;

public:
    bool onMouseDoubleClick(Point localPos);

private:
    friend class QtWidgets::Stack;
    friend class QtQuick::TabBar;

    class Private;
    Private *const d;

    KDDW_DELETE_COPY_CTOR(Stack)
};

}

}
