/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Controller.h"
#include "Group.h"
#include "Draggable_p.h"
#include "DockWidget.h"

namespace KDDockWidgets::Core {

class Stack;
class Group;

class DOCKS_EXPORT TabBar : public Controller, public Draggable
{
    Q_OBJECT
public:
    explicit TabBar(Stack *tabWidget = nullptr);
    virtual ~TabBar() override;

    /**
     * @brief returns the dock widgets at tab number @p index
     * @param index the tab number from which we want the dock widget
     * @return the dock widget at tab number @p index
     */
    DockWidget *dockWidgetAt(int index) const;

    ///@overload
    DockWidget *dockWidgetAt(Point localPos) const;

    /// @brief Returns the current dock widget
    DockWidget *currentDockWidget() const;
    void setCurrentDockWidget(DockWidget *dw);

    /// @brief Returns the index of the current tab
    int currentIndex() const;
    void setCurrentIndex(int index);

    /// @brief Returns the tab index of the specified dock widget
    int indexOfDockWidget(const Core::DockWidget *dw) const;
    void removeDockWidget(Core::DockWidget *dw);
    void insertDockWidget(int index, Core::DockWidget *dw, const Icon &icon,
                          const QString &title);

    // Draggable
    bool dragCanStart(Point pressPos, Point pos) const override;
    std::unique_ptr<WindowBeingDragged> makeWindow() override;
    bool isWindow() const override;

    void onMousePress(Point localPos);
    void onMouseDoubleClick(Point localPos);

    ///@brief returns whether there's only 1 tab
    bool hasSingleDockWidget() const;

    int numDockWidgets() const;

    bool tabsAreMovable() const;

    DockWidget *singleDockWidget() const override final;

    /// @reimp
    bool isMDI() const override;

    Group *group() const;
    Stack *stack() const;

    void moveTabTo(int from, int to);
    QString text(int index) const;
    Rect rectForTab(int index) const;
    ///@brief rename's the tab's text
    void renameTab(int index, const QString &);

    ///@brief change the tab's icon
    void changeTabIcon(int index, const Icon &);

    /// Returns whether we're inside Core::TabBar::moveTab()
    bool isMovingTab() const;

    class Private;
    Private *dptr() const;

private:
    Private *const d;
};

}
