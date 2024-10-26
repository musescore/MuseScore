/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockWidgetViewInterface.h"
#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/DockWidget.h"
#include "core/DockWidget_p.h"

namespace KDDockWidgets::Core {

DockWidgetViewInterface::DockWidgetViewInterface(DockWidget *controller)
    : m_dockWidget(controller)
{
}

Group *DockWidgetViewInterface::group() const
{
    return m_dockWidget->dptr()->group();
}

DockWidget *DockWidgetViewInterface::dockWidget() const
{
    return m_dockWidget;
}

TitleBar *DockWidgetViewInterface::actualTitleBar() const
{
    if (Group *group = this->group())
        return group->actualTitleBar();
    return nullptr;
}

DockWidgetViewInterface::~DockWidgetViewInterface() = default;

bool DockWidgetViewInterface::isFocused() const
{
    return m_dockWidget->isFocused();
}

bool DockWidgetViewInterface::isFloating() const
{
    return m_dockWidget->isFloating();
}

QString DockWidgetViewInterface::uniqueName() const
{
    return m_dockWidget->uniqueName();
}

QString DockWidgetViewInterface::title() const
{
    return m_dockWidget->title();
}

void DockWidgetViewInterface::setTitle(const QString &title)
{
    m_dockWidget->setTitle(title);
}

void DockWidgetViewInterface::setFloating(bool is)
{
    m_dockWidget->setFloating(is);
}

void DockWidgetViewInterface::setAsCurrentTab()
{
    m_dockWidget->setAsCurrentTab();
}

bool DockWidgetViewInterface::isOpen() const
{
    return m_dockWidget->isOpen();
}

void DockWidgetViewInterface::forceClose()
{
    m_dockWidget->forceClose();
}

void DockWidgetViewInterface::open()
{
    m_dockWidget->open();
}

void DockWidgetViewInterface::show()
{
    open();
}

void DockWidgetViewInterface::raise()
{
    m_dockWidget->raise();
}

void DockWidgetViewInterface::moveToSideBar()
{
    m_dockWidget->moveToSideBar();
}

void DockWidgetViewInterface::addDockWidgetAsTab(DockWidgetViewInterface *other,
                                                 const KDDockWidgets::InitialOption &initialOption)
{
    DockWidget *dw = other ? other->dockWidget() : nullptr;
    m_dockWidget->addDockWidgetAsTab(dw, initialOption);
}

void DockWidgetViewInterface::addDockWidgetToContainingWindow(
    DockWidgetViewInterface *other, KDDockWidgets::Location location,
    DockWidgetViewInterface *relativeTo, const KDDockWidgets::InitialOption &initialOption)
{
    DockWidget *dw = other ? other->dockWidget() : nullptr;
    DockWidget *relativeToDw = relativeTo ? relativeTo->dockWidget() : nullptr;
    m_dockWidget->addDockWidgetToContainingWindow(dw, location, relativeToDw, initialOption);
}

DockWidgetOptions DockWidgetViewInterface::options() const
{
    return m_dockWidget->options();
}

void DockWidgetViewInterface::setOptions(DockWidgetOptions opts)
{
    m_dockWidget->setOptions(opts);
}

void DockWidgetViewInterface::setIcon(const Icon &icon, IconPlaces places)
{
    m_dockWidget->setIcon(icon, places);
}

Icon DockWidgetViewInterface::icon(IconPlace place) const
{
    return m_dockWidget->icon(place);
}

void DockWidgetViewInterface::setAffinities(const Vector<QString> &affinities)
{
    m_dockWidget->setAffinities(affinities);
}

void DockWidgetViewInterface::setAffinityName(const QString &name)
{
    m_dockWidget->setAffinityName(name);
}

Vector<QString> DockWidgetViewInterface::affinities() const
{
    return m_dockWidget->affinities();
}

void DockWidgetViewInterface::setMDIPosition(Point pos)
{
    m_dockWidget->setMDIPosition(pos);
}

void DockWidgetViewInterface::setMDISize(Size size)
{
    m_dockWidget->setMDISize(size);
}

void DockWidgetViewInterface::setMDIZ(int z)
{
    m_dockWidget->setMDIZ(z);
}

} // namespace
