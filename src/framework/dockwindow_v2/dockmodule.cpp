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

#include "kddockwidgets/src/Config.h"
#include "kddockwidgets/src/core/FloatingWindow.h"
#include "kddockwidgets/src/qtquick/ViewFactory.h"
#include "kddockwidgets/src/qtquick/Platform.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "muse_framework_config.h"

namespace muse::dock {
class DockWidgetFactory : public KDDockWidgets::QtQuick::ViewFactory
{
public:
    explicit DockWidgetFactory(const modularity::ContextPtr& iocCtx)
        : m_iocContext(iocCtx) {}

    // KDDockWidgets::Core::DropIndicatorOverlay* createDropIndicatorOverlay(KDDockWidgets::Core::DropArea* dropArea) const override
    // {
    //     return new DropController(dropArea, m_iocContext);
    // }

    KDDockWidgets::Core::View* createSeparator(KDDockWidgets::Core::Separator* controller,
                                               KDDockWidgets::Core::View* parent = nullptr) const override
    {
        auto* parentItem = KDDockWidgets::QtQuick::asQQuickItem(parent);
        return new DockSeparator(controller, parentItem);
    }

    KDDockWidgets::Core::View* createTitleBar(KDDockWidgets::Core::TitleBar* controller,
                                              KDDockWidgets::Core::View* parent) const override
    {
        auto* parentItem = KDDockWidgets::QtQuick::asQQuickItem(parent);
        return new DockTitleBar(controller, parentItem);
    }

    KDDockWidgets::Core::View* createTabBar(KDDockWidgets::Core::TabBar* controller,
                                            KDDockWidgets::Core::View* parent = nullptr) const override
    {
        auto* parentItem = KDDockWidgets::QtQuick::asQQuickItem(parent);
        return new DockTabBar(controller, parentItem);
    }

    QUrl titleBarFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockTitleBar.qml");
    }

    QUrl dockwidgetFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockWidget.qml");
    }

    QUrl groupFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockFrame.qml");
    }

    QUrl floatingWindowFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockFloatingWindow.qml");
    }

    QUrl separatorFilename() const override
    {
        return QUrl("qrc:/qt/qml/Muse/Dock/DockSeparator.qml");
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

    // ===================================
    // Setup KDDockWidgets
    // ===================================

    QQmlEngine* engine = ioc()->resolve<ui::IUiEngine>(module_name)->qmlEngine();

    KDDockWidgets::Config& config = KDDockWidgets::Config::self();

    config.setViewFactory(new DockWidgetFactory(iocContext()));

    KDDockWidgets::QtQuick::Platform::instance()->setQmlEngine(engine);

    auto flags = config.flags()
                 | KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;

    config.setFlags(flags);

    KDDockWidgets::Core::FloatingWindow::s_windowFlagsOverride = Qt::Tool
                                                                 | Qt::NoDropShadowWindowHint
                                                                 | Qt::FramelessWindowHint;

    auto internalFlags = config.internalFlags()
                         | KDDockWidgets::Config::InternalFlag_UseTransparentFloatingWindow;

    config.setInternalFlags(internalFlags);

    config.setAbsoluteWidgetMinSize(QSize(10, 10));
    config.setSeparatorThickness(1);
}

void DockContext::onDeinit()
{
}
