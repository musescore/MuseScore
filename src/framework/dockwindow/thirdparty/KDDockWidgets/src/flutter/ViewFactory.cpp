/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ViewFactory.h"
#include "Config.h"

#include "ClassicIndicatorsWindow.h"
#include "core/Utils_p.h"
#include "core/Logging_p.h"
#include "Action.h"

#include "kddockwidgets/core/TabBar.h"
#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/FloatingWindow.h"
#include "kddockwidgets/core/indicators/ClassicDropIndicatorOverlay.h"
#include "kddockwidgets/core/indicators/NullDropIndicatorOverlay.h"
#include "kddockwidgets/core/indicators/SegmentedDropIndicatorOverlay.h"
#include "kddockwidgets/core/MainWindow.h"

#include "views/ClassicIndicatorWindowViewInterface.h"


// clazy:excludeall=ctor-missing-parent-argument

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

ViewFactory::~ViewFactory()
{
}

Core::View *ViewFactory::createDockWidget(const QString &, DockWidgetOptions, LayoutSaverOptions,
                                          Qt::WindowFlags) const
{
    return {};
}


Core::View *ViewFactory::createGroup(Core::Group *, Core::View *) const
{
    assert(false);
    return {};
}

Core::View *ViewFactory::createTitleBar(Core::TitleBar *, Core::View *) const
{
    return {};
}

Core::View *ViewFactory::createTabBar(Core::TabBar *, Core::View *) const
{
    return {};
}

Core::View *ViewFactory::createStack(Core::Stack *, Core::View *) const
{
    return {};
}

Core::View *ViewFactory::createSeparator(Core::Separator *, Core::View *) const
{
    return {};
}

Core::View *ViewFactory::createFloatingWindow(Core::FloatingWindow *,
                                              Core::MainWindow *, Qt::WindowFlags) const
{
    return {};
}

Core::View *ViewFactory::createRubberBand(Core::View *) const
{
    return nullptr;
}

Core::View *ViewFactory::createSideBar(Core::SideBar *, Core::View *) const
{
    return {};
}

// iconForButtonType impl is the same for QtQuick and QtWidgets
Icon ViewFactory::iconForButtonType(TitleBarButtonType, double) const
{
    return {};
}

Core::View *ViewFactory::createDropArea(Core::DropArea *, Core::View *) const
{
    return {};
}

Core::View *ViewFactory::createMDILayout(Core::MDILayout *, Core::View *) const
{
    return {};
}

Core::View *
ViewFactory::createSegmentedDropIndicatorOverlayView(Core::SegmentedDropIndicatorOverlay *,
                                                     Core::View *) const
{
    return {};
}

Core::ClassicIndicatorWindowViewInterface *
ViewFactory::createClassicIndicatorWindow(Core::ClassicDropIndicatorOverlay *controller, Core::View *parent) const
{
    // We need a little indirection here, since our bindings don't support multiple inheritance
    // In dart View factory can't return something of type ClassicIndicatorWindowViewInterface when only having IndicatorWindow
    // But in C++ we can, so get it from flutter and cast it here
    return createClassicIndicatorWindow_flutter(controller, parent);
}

flutter::IndicatorWindow *
ViewFactory::createClassicIndicatorWindow_flutter(Core::ClassicDropIndicatorOverlay *, Core::View *) const
{
    KDDW_WARN("ViewFactory::createClassicIndicatorWindow_flutter: Implemented in dart");
    return nullptr;
}

KDDockWidgets::Core::Action *ViewFactory::createAction(Core::DockWidget *dw, const char *debugName) const
{
    return new Flutter::Action(dw, debugName);
}
