/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "internal/dropcontroller.h"
#include "internal/dockseparator.h"
#include "internal/docktabbar.h"
#include "internal/docktitlebar.h"
#include "internal/dockwindowactionscontroller.h"
#include "internal/dockwindowprovider.h"

#include "thirdparty/KDDockWidgets/src/Config.h"
#include "thirdparty/KDDockWidgets/src/DockWidgetBase.h"
#include "thirdparty/KDDockWidgets/src/FrameworkWidgetFactory.h"
#include "thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "muse_framework_config.h"

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
        return new DockTitleBar(frame);
    }

    KDDockWidgets::TitleBar* createTitleBar(KDDockWidgets::FloatingWindow* floatingWindow) const override
    {
        return new DockTitleBar(floatingWindow);
    }

    KDDockWidgets::TabBar* createTabBar(KDDockWidgets::TabWidget* parent) const override
    {
        return new DockTabBar(parent);
    }

    QUrl titleBarFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockTitleBar.qml");
    }

    QUrl dockwidgetFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockWidget.qml");
    }

    QUrl frameFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockFrame.qml");
    }

    QUrl floatingWindowFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockFloatingWindow.qml");
    }

private:
    const modularity::ContextPtr m_iocContext;
};
}

using namespace muse::dock;
using namespace muse::modularity;

static const std::string module_name = "dockwindow";

std::string DockModule::moduleName() const
{
    return module_name;
}

void DockModule::registerExports()
{
}

void DockModule::onInit(const IApplication::RunMode&)
{
    // ===================================
    // Setup KDDockWidgets
    // ===================================

    QQmlEngine* engine = globalIoc()->resolve<ui::IUiEngine>(moduleName())->qmlEngine();

    KDDockWidgets::Config::self().setFrameworkWidgetFactory(new DockWidgetFactory(globalCtx()));
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

// Context

IContextSetup* DockModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new DockContext(ctx);
}

void DockContext::registerExports()
{
    m_actionsController = std::make_shared<DockWindowActionsController>(iocContext());

    ioc()->registerExport<IDockWindowProvider>(module_name, new DockWindowProvider());
}

void DockContext::onInit(const IApplication::RunMode&)
{
    m_actionsController->init();
}
