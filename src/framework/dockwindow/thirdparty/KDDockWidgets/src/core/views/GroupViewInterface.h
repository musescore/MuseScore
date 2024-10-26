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

namespace KDDockWidgets {

namespace Core {
class DockWidget;
class Group;

/// @brief The interface that Frame views should implement
class DOCKS_EXPORT GroupViewInterface
{
public:
    explicit GroupViewInterface(Group *);
    virtual ~GroupViewInterface();
    virtual void removeDockWidget(DockWidget *);
    virtual void insertDockWidget(DockWidget *, int index);

    /// @brief Returns the height of the "non-dockwidget" part.
    /// i.e.: the height of the titlebar (if any), + height of tabbar (if any) + any margins.
    /// Should be implemented by frontend developers, as KDDW doesn't know the layout of the group.
    /// This is used to honour minimum-sizes of dock widgets.
    virtual int nonContentsHeight() const = 0;

    /// @brief the rect that should start a drag.
    /// Only relevant if the title bar isn't visible. For normal KDDW usage this method doesn't
    /// need to be reimplemented.
    virtual Rect dragRect() const;

    bool isMDI() const;
    Group *group() const;

protected:
    Group *const m_group;
    GroupViewInterface(const GroupViewInterface &) = delete;
    GroupViewInterface &operator=(const GroupViewInterface &) = delete;
};

}

}
