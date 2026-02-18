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

#include <QQmlContext>

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

    QQmlContext* qmlCreationContext() const override
    {
        if (!m_qmlCreationContext) {
            auto engine = KDDockWidgets::Config::self().qmlEngine();
            if (engine) {
                auto* self = const_cast<DockWidgetFactory*>(this);
                self->m_qmlCreationContext = new QQmlContext(engine->rootContext(), self);
                auto* qmlIoc = new muse::QmlIoCContext(self->m_qmlCreationContext);
                qmlIoc->ctx = m_iocContext;
                self->m_qmlCreationContext->setContextProperty("ioc_context", QVariant::fromValue(qmlIoc));
            }
        }
        return m_qmlCreationContext;
    }

private:
    const modularity::ContextPtr m_iocContext;
    QQmlContext* m_qmlCreationContext = nullptr;
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

    auto& globalConfig = KDDockWidgets::Config::self(-1);
    globalConfig.setFrameworkWidgetFactory(new DockWidgetFactory(globalCtx()));
    globalConfig.setQmlEngine(engine);

    auto flags = globalConfig.flags()
                 | KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible
                 | KDDockWidgets::Config::Flag_TitleBarNoFloatButton;

    globalConfig.setFlags(flags);

    KDDockWidgets::FloatingWindow::s_windowFlagsOverride = Qt::Tool
                                                           | Qt::NoDropShadowWindowHint
                                                           | Qt::FramelessWindowHint;

    auto internalFlags = globalConfig.internalFlags()
                         | KDDockWidgets::Config::InternalFlag_UseTransparentFloatingWindow;

    globalConfig.setInternalFlags(internalFlags);

    globalConfig.setAbsoluteWidgetMinSize(QSize(10, 10));
    globalConfig.setSeparatorThickness(1);
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

    int contextId = iocContext() ? iocContext()->id : -1;
    if (contextId >= 0) {
        auto& config = KDDockWidgets::Config::createContext(contextId);
        Q_ASSERT(config.qmlEngine()); // must be inherited from default context
        config.setFrameworkWidgetFactory(new DockWidgetFactory(iocContext()));
    }
}

void DockContext::onDeinit()
{
    int contextId = iocContext() ? iocContext()->id : -1;
    if (contextId >= 0) {
        KDDockWidgets::Config::destroyContext(contextId);
    }
}
