/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/QtCompat_p.h"

QT_BEGIN_NAMESPACE
class Icon;
class QString;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {

class DockWidget;
class TabBar;

/// @brief The interface that TabBar views should implement
class DOCKS_EXPORT TabBarViewInterface
{
public:
    explicit TabBarViewInterface(TabBar *);
    virtual ~TabBarViewInterface();

    /// @brief Implement if your frontend will support reordering tabs with mouse
    /// Currently only the QtWidgets frontend supports it
    virtual void setTabsAreMovable(bool);

    /// @brief Returns the tab text for the specified index
    /// This is only used by tests, to make sure your tab's text is correct
    virtual QString text(int index) const = 0;

    virtual int tabAt(Point localPt) const = 0;
    virtual void moveTabTo(int from, int to) = 0;
    virtual Rect rectForTab(int index) const = 0;
    virtual void setCurrentIndex(int index) = 0;

    virtual void renameTab(int index, const QString &) = 0;
    virtual void changeTabIcon(int index, const Icon &icon) = 0;

    virtual void removeDockWidget(DockWidget *dw) = 0;
    virtual void insertDockWidget(int index, DockWidget *dw, const Icon &icon,
                                  const QString &title) = 0;

protected:
    TabBar *const m_tabBar;
    TabBarViewInterface(const TabBarViewInterface &) = delete;
    TabBarViewInterface &operator=(const TabBarViewInterface &) = delete;
};

}

}
