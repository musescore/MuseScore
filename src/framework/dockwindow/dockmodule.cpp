/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "dockmodule.h"

#include <QQmlEngine>

#include "internal/dropcontroller.h"
#include "internal/dockseparator.h"
#include "internal/dockframemodel.h"
#include "internal/docktabbar.h"
#include "internal/dockwindowactionscontroller.h"
#include "internal/dockwindowprovider.h"

#include "view/dockwindow.h"
#include "view/dockpanelview.h"
#include "view/docktoolbarview.h"
#include "view/dockstatusbarview.h"
#include "view/dockingholderview.h"
#include "view/dockcentralview.h"
#include "view/dockpageview.h"
#include "view/docktitlebar.h"

#include "docktypes.h"

#include "thirdparty/KDDockWidgets/src/Config.h"
#include "thirdparty/KDDockWidgets/src/DockWidgetBase.h"
#include "thirdparty/KDDockWidgets/src/FrameworkWidgetFactory.h"
#include "thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

namespace muse::dock {
class DockWidgetFactory : public KDDockWidgets::DefaultWidgetFactory
{
public:
    DockWidgetFactory(const modularity::ContextPtr& iocCtx)
        : m_iocContext(iocCtx) {}

    KDDockWidgets::DropIndicatorOverlayInterface* createDropIndicatorOverlay(KDDockWidgets::DropArea* dropArea) const override
    {
        return new DropController(dropArea, m_iocContext);
    }

    Layouting::Separator* createSeparator(Layouting::Widget* parent = nullptr) const override
    {
        return new DockSeparator(parent);
    }

    KDDockWidgets::TitleBar* createTitleBar(KDDockWidgets::Frame* frame) const override
    {
        KDDockWidgets::TitleBar* titleBar = new DockTitleBar(frame);

        // Hide the default title bar because we add our own. The worst part is that the default
        // title bar captures and steals the mouse events from our title bar.
        titleBar->setHeight(0);

        return titleBar;
    }

    KDDockWidgets::TitleBar* createTitleBar(KDDockWidgets::FloatingWindow* floatingWindow) const override
    {
        KDDockWidgets::TitleBar* titleBar = new DockTitleBar(floatingWindow);

        // Hide the default title bar because we add our own. The worst part is that the default
        // title bar captures and steals the mouse events from our title bar.
        titleBar->setHeight(0);

        return titleBar;
    }

    KDDockWidgets::TabBar* createTabBar(KDDockWidgets::TabWidget* parent) const override
    {
        return new DockTabBar(parent);
    }

    QUrl titleBarFilename() const override
    {
        return QUrl("qrc:/qml/Muse/Dock/DockTitleBar.qml");
    }

    QUrl dockwidgetFilename() const override
    {
        return QUrl("qrc:/qml/Muse/Dock/DockWidget.qml");
    }

    QUrl frameFilename() const override
    {
        return QUrl("qrc:/qml/Muse/Dock/DockFrame.qml");
    }

    QUrl floatingWindowFilename() const override
    {
        return QUrl("qrc:/qml/Muse/Dock/DockFloatingWindow.qml");
    }

private:
    const modularity::ContextPtr m_iocContext;
};
}

using namespace muse::dock;
using namespace muse::modularity;

static void dock_init_qrc()
{
    Q_INIT_RESOURCE(dock);
}

std::string DockModule::moduleName() const
{
    return "dockwindow";
}

void DockModule::registerExports()
{
    m_actionsController = std::make_shared<DockWindowActionsController>(iocContext());

    ioc()->registerExport<IDockWindowProvider>(moduleName(), new DockWindowProvider());
}

void DockModule::registerResources()
{
    dock_init_qrc();
}

void DockModule::registerUiTypes()
{
    qmlRegisterType<DockWindow>("Muse.Dock", 1, 0, "DockWindow");
    qmlRegisterType<DockPanelView>("Muse.Dock", 1, 0, "DockPanelView");
    qmlRegisterType<DockStatusBarView>("Muse.Dock", 1, 0, "DockStatusBar");
    qmlRegisterType<DockToolBarView>("Muse.Dock", 1, 0, "DockToolBarView");
    qmlRegisterType<DockingHolderView>("Muse.Dock", 1, 0, "DockingHolderView");
    qmlRegisterType<DockCentralView>("Muse.Dock", 1, 0, "DockCentralView");
    qmlRegisterType<DockPageView>("Muse.Dock", 1, 0, "DockPageView");
    qmlRegisterType<DockFrameModel>("Muse.Dock", 1, 0, "DockFrameModel");

    qmlRegisterUncreatableType<DockToolBarAlignment>("Muse.Dock", 1, 0, "DockToolBarAlignment", "Not creatable from QML");
    qmlRegisterUncreatableType<DockLocation>("Muse.Dock", 1, 0, "Location", "Not creatable from QML");
}

void DockModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_actionsController->init();

    // ===================================
    // Setup KDDockWidgets
    // ===================================

    QQmlEngine* engine = ioc()->resolve<ui::IUiEngine>(moduleName())->qmlEngine();

    KDDockWidgets::Config::self().setFrameworkWidgetFactory(new DockWidgetFactory(iocContext()));
    KDDockWidgets::Config::self().setQmlEngine(engine);

    auto flags = KDDockWidgets::Config::self().flags()
                 | KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible
                 | KDDockWidgets::Config::Flag_TitleBarNoFloatButton;

    KDDockWidgets::Config::self().setFlags(flags);

    KDDockWidgets::FloatingWindow::s_windowFlagsOverride = Qt::Tool
                                                           | Qt::NoDropShadowWindowHint
                                                           | Qt::FramelessWindowHint;

    auto internalFlags = KDDockWidgets::Config::self().internalFlags()
                         | KDDockWidgets::Config::InternalFlag_UseTransparentFloatingWindow;

    KDDockWidgets::Config::self().setInternalFlags(internalFlags);

    KDDockWidgets::Config::self().setAbsoluteWidgetMinSize(QSize(10, 10));
    KDDockWidgets::Config::self().setSeparatorThickness(1);
}
