/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "SideBarViewInterface.h"
#include "kddockwidgets/core/SideBar.h"


namespace KDDockWidgets::Core {

SideBarViewInterface::SideBarViewInterface(SideBar *controller)
    : m_sideBar(controller)
{
}

SideBarViewInterface::~SideBarViewInterface() = default;

SideBar *SideBarViewInterface::sideBar() const
{
    return m_sideBar;
}

} // namespace
