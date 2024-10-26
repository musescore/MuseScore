/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"

namespace KDDockWidgets {

namespace Core {

class DockWidget;
class SideBar;

/// @brief The interface that SideBar views should implement
class DOCKS_EXPORT SideBarViewInterface
{
public:
    explicit SideBarViewInterface(SideBar *);
    virtual ~SideBarViewInterface();
    virtual void addDockWidget_Impl(DockWidget *dock) = 0;
    virtual void removeDockWidget_Impl(DockWidget *dock) = 0;

    SideBar *sideBar() const;

protected:
    SideBar *const m_sideBar;
    SideBarViewInterface(const SideBarViewInterface &) = delete;
    SideBarViewInterface &operator=(const SideBarViewInterface &) = delete;
};

}

}
