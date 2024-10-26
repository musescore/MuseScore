/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ViewFactory.h"
#include "Config.h"

#include "kddockwidgets/core/indicators/ClassicDropIndicatorOverlay.h"
#include "kddockwidgets/core/indicators/NullDropIndicatorOverlay.h"
#include "kddockwidgets/core/indicators/SegmentedDropIndicatorOverlay.h"
#include "core/Utils_p.h"

#include "kddockwidgets/core/MainWindow.h"
#include "kddockwidgets/core/TabBar.h"
#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/FloatingWindow.h"

#include "qtquick/Action.h"
#include "qtquick/views/FloatingWindow.h"
#include "qtquick/views/DockWidget.h"
#include "qtquick/views/DropArea.h"
#include "qtquick/views/Group.h"
#include "qtquick/views/View.h"
#include "qtquick/views/RubberBand.h"
#include "qtquick/views/Separator.h"
#include "qtquick/views/TitleBar.h"
#include "qtquick/views/TabBar.h"
// #include "qtquick/views/SideBar.h"
#include "qtquick/views/Stack.h"
#include "qtquick/views/MainWindow.h"
#include "qtquick/views/MDILayout.h"
#include "qtquick/views/ClassicIndicatorsWindow.h"

#include "views/ClassicIndicatorsWindow.h"

// clazy:excludeall=ctor-missing-parent-argument

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;


ViewFactory::~ViewFactory()
{
}

Core::View *ViewFactory::createDockWidget(const QString &uniqueName, DockWidgetOptions options,
                                          LayoutSaverOptions layoutSaverOptions,
                                          Qt::WindowFlags windowFlags) const
{
    return createDockWidget(uniqueName, /*engine=*/nullptr, options, layoutSaverOptions,
                            windowFlags);
}

Core::View *ViewFactory::createDockWidget(const QString &uniqueName, QQmlEngine *qmlEngine,
                                          DockWidgetOptions options,
                                          LayoutSaverOptions layoutSaverOptions,
                                          Qt::WindowFlags windowFlags) const
{
    return new QtQuick::DockWidget(uniqueName, options, layoutSaverOptions, windowFlags,
                                   qmlEngine);
}

Core::View *ViewFactory::createGroup(Core::Group *controller, Core::View *parent) const
{
    return new Group(controller, QtQuick::asQQuickItem(parent));
}

Core::View *ViewFactory::createTitleBar(Core::TitleBar *titleBar, Core::View *parent) const
{
    return new TitleBar(titleBar, QtQuick::asQQuickItem(parent));
}

Core::View *ViewFactory::createTabBar(Core::TabBar *controller, Core::View *parent) const
{
    return new TabBar(controller, QtQuick::asQQuickItem(parent));
}

Core::View *ViewFactory::createStack(Core::Stack *controller, Core::View *parent) const
{
    return new Stack(controller, QtQuick::asQQuickItem(parent));
}

Core::View *ViewFactory::createSeparator(Core::Separator *controller, Core::View *parent) const
{
    return new Separator(
        controller, parent ? static_cast<QtQuick::View *>(parent) : nullptr);
}

Core::View *ViewFactory::createFloatingWindow(Core::FloatingWindow *controller,
                                              Core::MainWindow *parent,
                                              Qt::WindowFlags flags) const
{

    auto mainwindow = parent
        ? qobject_cast<QtQuick::MainWindow *>(QtQuick::asQQuickItem(parent->view()))
        : nullptr;
    return new FloatingWindow(controller, mainwindow, flags);
}

Core::View *ViewFactory::createRubberBand(Core::View *parent) const
{
    return new RubberBand(QtQuick::asQQuickItem(parent));
}

Core::View *ViewFactory::createSideBar(Core::SideBar *, Core::View *) const
{
    return {};
}

// iconForButtonType impl is the same for QtQuick and QtWidgets
QIcon ViewFactory::iconForButtonType(TitleBarButtonType type, qreal dpr) const
{
    QString iconName;
    switch (type) {
    case TitleBarButtonType::AutoHide:
        iconName = QStringLiteral("auto-hide");
        break;
    case TitleBarButtonType::UnautoHide:
        iconName = QStringLiteral("unauto-hide");
        break;
    case TitleBarButtonType::Close:
        iconName = QStringLiteral("close");
        break;
    case TitleBarButtonType::Minimize:
        iconName = QStringLiteral("min");
        break;
    case TitleBarButtonType::Maximize:
        iconName = QStringLiteral("max");
        break;
    case TitleBarButtonType::Normal:
        // We're using the same icon as dock/float
        iconName = QStringLiteral("dock-float");
        break;
    case TitleBarButtonType::Float:
        iconName = QStringLiteral("dock-float");
        break;
    case TitleBarButtonType::AllTitleBarButtonTypes:
        break;
    }

    if (iconName.isEmpty())
        return {};

    QIcon icon(QStringLiteral(":/img/%1.png").arg(iconName));
    if (!scalingFactorIsSupported(dpr))
        return icon;

    // Not using Qt's sugar syntax, which doesn't support 1.5x anyway when we need it.
    // Simply add the high-res files and Qt will pick them when needed

    if (scalingFactorIsSupported(1.5))
        icon.addFile(QStringLiteral(":/img/%1-1.5x.png").arg(iconName));

    icon.addFile(QStringLiteral(":/img/%1-2x.png").arg(iconName));

    return icon;
}

Core::View *ViewFactory::createDropArea(Core::DropArea *controller, Core::View *parent) const
{
    return new DropArea(controller, parent);
}

Core::View *ViewFactory::createMDILayout(Core::MDILayout *controller, Core::View *parent) const
{
    return new MDILayout(controller, parent);
}

QUrl ViewFactory::titleBarFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/TitleBar.qml"));
}

QUrl ViewFactory::dockwidgetFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/DockWidget.qml"));
}

QUrl ViewFactory::groupFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/Group.qml"));
}

QUrl ViewFactory::floatingWindowFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/FloatingWindow.qml"));
}

QUrl ViewFactory::tabbarFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/TabBar.qml"));
}

QUrl ViewFactory::separatorFilename() const
{
    return QUrl(QStringLiteral("qrc:/kddockwidgets/qtquick/views/qml/Separator.qml"));
}

Core::View *
ViewFactory::createSegmentedDropIndicatorOverlayView(Core::SegmentedDropIndicatorOverlay *,
                                                     Core::View *) const
{
    return nullptr;
}

Core::ClassicIndicatorWindowViewInterface *ViewFactory::createClassicIndicatorWindow(
    Core::ClassicDropIndicatorOverlay *classicIndicators, Core::View *parent) const
{
    return new QtQuick::ClassicDropIndicatorOverlay(classicIndicators, parent);
}

ViewFactory *ViewFactory::self()
{
    auto factory = qobject_cast<ViewFactory *>(Config::self().viewFactory());

    if (!factory)
        qWarning() << Q_FUNC_INFO << "Expected a ViewFactory subclass, not"
                   << Config::self().viewFactory();

    return factory;
}

KDDockWidgets::Core::Action *ViewFactory::createAction(Core::DockWidget *dw, const char *debugName) const
{
    return new QtQuick::Action(dw, debugName);
}
