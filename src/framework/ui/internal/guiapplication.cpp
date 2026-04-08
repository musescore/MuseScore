/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "guiapplication.h"

#include <QTimer>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>

#include "ui/graphicsapiprovider.h"
#include "ui/iuiengine.h"

#include "global/globalmodule.h"

#include "muse_framework_config.h"
#include "app_config.h"

using namespace muse;
using namespace muse::ui;

GuiApplication::GuiApplication(const std::shared_ptr<CmdOptions>& options)
    : BaseApplication(options)
{
}

void GuiApplication::doSetup(const std::shared_ptr<CmdOptions>& options)
{
    BaseApplication::doSetup(options);

    // ====================================================
    // Setup modules: onDelayedInit
    // ====================================================
    m_delayedInitTimer.setSingleShot(true);
    m_delayedInitTimer.setInterval(5000);
    QObject::connect(&m_delayedInitTimer, &QTimer::timeout, [this]() {
        m_globalModule->onDelayedInit();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onDelayedInit();
        }
    });
    m_delayedInitTimer.start();

    // ====================================================
    // Setup Graphics Api check
    // ====================================================
    //! Needs to be set because we use transparent windows for PopupView.
    //! Needs to be called before any QQuickWindows are shown.
    QQuickWindow::setDefaultAlphaBuffer(true);

    setupGraphicsApi();
}

void GuiApplication::setupGraphicsApi()
{
    //! NOTE Adjust GS Api
    //! We can hide this algorithm in GSApiProvider,
    //! but it is intentionally left here to illustrate what is happening.

    GraphicsApiProvider* gApiProvider = new GraphicsApiProvider(BaseApplication::appVersion());

    GraphicsApi required = gApiProvider->requiredGraphicsApi();
    if (required != GraphicsApi::Default) {
        LOGI() << "Setting required graphics api: " << GraphicsApiProvider::apiName(required);
        GraphicsApiProvider::setGraphicsApi(required);
    }

    LOGI() << "Using graphics api: " << GraphicsApiProvider::graphicsApiName();
    LOGI() << "Gui platform: " << QGuiApplication::platformName();

    if (GraphicsApiProvider::graphicsApi() == GraphicsApi::Software) {
        gApiProvider->destroy();
    } else {
        LOGI() << "Detecting problems with graphics api";
        gApiProvider->listen([this, gApiProvider, required](bool res) {
            if (res) {
                LOGI() << "No problems detected with graphics api";
                gApiProvider->setGraphicsApiStatus(required, GraphicsApiProvider::Status::Checked);
            } else {
                GraphicsApi next = gApiProvider->switchToNextGraphicsApi(required);
                LOGE() << "Detected problems with graphics api; switching from " << GraphicsApiProvider::apiName(required)
                       << " to " << GraphicsApiProvider::apiName(next);

                this->restart();
            }
            gApiProvider->destroy();
        });
    }
}

void GuiApplication::doFinish()
{
    m_delayedInitTimer.stop();

    BaseApplication::doFinish();
}

void GuiApplication::startupScenario(const muse::modularity::ContextPtr& ctxId)
{
    TRACEFUNC;

    QMetaObject::invokeMethod(qApp, [this, ctxId]() {
        bool ok = loadMainWindow(ctxId);
        if (ok) {
            QMetaObject::invokeMethod(qApp, [this, ctxId]() {
                doStartupScenario(ctxId);
            }, Qt::QueuedConnection);
        }
    }, Qt::QueuedConnection);
}

bool GuiApplication::loadMainWindow(const muse::modularity::ContextPtr& ctxId)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(ctxId) {
        return false;
    }

#if defined(Q_OS_MAC)
    QString platform = "mac";
#elif defined(Q_OS_WIN)
    QString platform = "win";
#else
    QString platform = "linux";
#endif

    auto uiengine = muse::modularity::ioc(ctxId)->resolve<muse::ui::IUiEngine>("app");
    IF_ASSERT_FAILED(uiengine) {
        return false;
    }

    QQmlApplicationEngine* engine = uiengine->qmlAppEngine();
    IF_ASSERT_FAILED(engine) {
        return false;
    }

    QString path = mainWindowQmlPath(platform);
    QQmlComponent component = QQmlComponent(engine, path);
    if (!component.isReady()) {
        LOGE() << "Failed to load main qml file, err: " << component.errorString();
        return false;
    }

    QQmlContext* qmlCtx = new QQmlContext(engine);
    qmlCtx->setObjectName(QString("QQmlContext: %1").arg(ctxId->id));
    QmlIoCContext* iocCtx = new QmlIoCContext(qmlCtx);
    iocCtx->ctx = ctxId;
    qmlCtx->setContextProperty("ioc_context", QVariant::fromValue(iocCtx));

    QObject* obj = component.create(qmlCtx);
    if (!obj) {
        LOGE() << "failed Qml load\n";
        QCoreApplication::exit(-1);
        return false;
    }

    // The main window must be shown at this point so KDDockWidgets can read its size correctly
    // and scale all sizes properly. https://github.com/musescore/MuseScore/issues/21148
    // but before that, let's make the window transparent,
    // otherwise the empty window frame will be visible
    // https://github.com/musescore/MuseScore/issues/29630
    // Transparency will be removed after the page loads.

    QQuickWindow* window = dynamic_cast<QQuickWindow*>(obj);
    window->setOpacity(0.01);
    window->setVisible(true);

    m_windows[ctxId->id] = window;

    return true;
}

void GuiApplication::doDestroyContext(const ContextData& data)
{
    auto wit = m_windows.find(data.ctxId->id);
    if (wit != m_windows.end()) {
        QQuickWindow* window = wit->second;
        window->setVisible(false);
        m_windows.erase(wit);
        delete window;
    }

    // Engine quit
    muse::modularity::ioc(data.ctxId)->resolve<muse::ui::IUiEngine>("app")->quit();

    BaseApplication::doDestroyContext(data);
}
