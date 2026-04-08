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
#include "baseapplication.h"

#include "global/types/version.h"
#include "global/globalmodule.h"
#include "global/async/processevents.h"

#include "muse_framework_config.h"

#ifndef NO_QT_SUPPORT
#include <QApplication>
#endif

#ifdef QT_QPROCESS_SUPPORTED
#include <QProcess>
#endif

#ifdef QT_CONCURRENT_SUPPORTED
#include <QThreadPool>
#endif

#include "log.h"

using namespace muse;

String BaseApplication::appName()
{
#ifdef MUSE_APP_NAME
    return String::fromAscii(MUSE_APP_NAME);
#else
    return String();
#endif
}

String BaseApplication::appTitle()
{
#ifdef MUSE_APP_TITLE
#ifdef MUSE_APP_UNSTABLE
    return String::fromAscii(MUSE_APP_TITLE) + u" Development";
#else
    return String::fromAscii(MUSE_APP_TITLE);
#endif
#else
    return String();
#endif
}

bool BaseApplication::appUnstable()
{
#ifdef MUSE_APP_UNSTABLE
    return true;
#else
    return false;
#endif
}

Version BaseApplication::appVersion()
{
#ifdef MUSE_APP_VERSION
    static Version v(MUSE_APP_VERSION);
#else
    static Version v("0.0.0");
#endif
    return v;
}

Version BaseApplication::appFullVersion()
{
    static Version fv(appVersion());

#ifdef MUSE_APP_VERSION_LABEL
    static bool once = false;
    if (!once) {
        String versionLabel = String::fromAscii(MUSE_APP_VERSION_LABEL);
        if (!versionLabel.isEmpty()) {
            fv.setSuffix(versionLabel);
        }
        once = true;
    }
#endif

    return fv;
}

String BaseApplication::appBuild()
{
#ifdef MUSE_APP_BUILD_NUMBER
    return String::fromAscii(MUSE_APP_BUILD_NUMBER);
#else
    return String();
#endif
}

String BaseApplication::appRevision()
{
#ifdef MUSE_APP_REVISION
    return String::fromAscii(MUSE_APP_REVISION);
#else
    return String();
#endif
}

BaseApplication::BaseApplication(const std::shared_ptr<CmdOptions>& appOptions)
    : m_appOptions(appOptions)
{
}

void BaseApplication::setRunMode(const RunMode& mode)
{
    m_runMode = mode;
}

IApplication::RunMode BaseApplication::runMode() const
{
    return m_runMode;
}

bool BaseApplication::noGui() const
{
    switch (m_runMode) {
    case RunMode::GuiApp: return false;
    case RunMode::ConsoleApp: return true;
    case RunMode::AudioPluginRegistration: return true;
    }
    return false;
}

void BaseApplication::addModule(muse::modularity::IModuleSetup* module)
{
    m_modules.push_back(module);
}

void BaseApplication::setup()
{
    doSetup(m_appOptions);
}

void BaseApplication::doSetup(const std::shared_ptr<CmdOptions>& options)
{
    IF_ASSERT_FAILED(options) {
        return;
    }

    const IApplication::RunMode runMode = options->runMode;

    setRunMode(runMode);

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================

    modularity::globalIoc()->registerExport<IApplication>("global", shared_from_this());

    m_globalModule = new GlobalModule();
    m_globalModule->registerResources();
    m_globalModule->registerExports();
    m_globalModule->registerUiTypes();

    for (modularity::IModuleSetup* m : m_modules) {
        m->registerResources();
        m->registerExports();
    }

    m_globalModule->resolveImports();
    m_globalModule->registerApi();
    for (modularity::IModuleSetup* m : m_modules) {
        m->registerUiTypes();
        m->resolveImports();
        m->registerApi();
    }

    // ====================================================
    // Setup modules: apply the command line options
    // ====================================================
    applyCommandLineOptions(options);

    // ====================================================
    // Setup modules: onPreInit
    // ====================================================
    m_globalModule->onPreInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onPreInit(runMode);
    }

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    m_globalModule->onInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

    // ====================================================
    // Setup modules: onAllInited
    // ====================================================
    m_globalModule->onAllInited(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onAllInited(runMode);
    }

    // ====================================================
    // Setup modules: onStartApp (on next event loop)
    // ====================================================
    QMetaObject::invokeMethod(qApp, [this]() {
        m_globalModule->onStartApp();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onStartApp();
        }
    }, Qt::QueuedConnection);
}

void BaseApplication::applyCommandLineOptions(const std::shared_ptr<CmdOptions>& options)
{
    if (options->global.loggerLevel) {
        m_globalModule->setLoggerLevel(options->global.loggerLevel.value());
    }
}

BaseApplication::ContextData& BaseApplication::contextData(const muse::modularity::ContextPtr& ctx)
{
    for (ContextData& c : m_contexts) {
        if (c.ctxId->id == ctx->id) {
            return c;
        }
    }

    m_contexts.emplace_back();

    ContextData& ref = m_contexts.back();
    ref.ctxId = ctx;

    modularity::IContextSetup* global = m_globalModule->newContext(ctx);
    if (global) {
        ref.setups.push_back(global);
    }

    for (modularity::IModuleSetup* m : m_modules) {
        modularity::IContextSetup* s = m->newContext(ctx);
        if (s) {
            ref.setups.push_back(s);
        }
    }

    return ref;
}

std::vector<muse::modularity::ContextPtr> BaseApplication::contexts() const
{
    std::vector<muse::modularity::ContextPtr> ctxs;
    ctxs.reserve(m_contexts.size());
    for (const ContextData& c : m_contexts) {
        ctxs.push_back(c.ctxId);
    }
    return ctxs;
}

size_t BaseApplication::contextCount() const
{
    return m_contexts.size();
}

std::shared_ptr<CmdOptions> BaseApplication::makeContextOptions(const muse::StringList&) const
{
    return m_appOptions;
}

void BaseApplication::showContextSplash(const muse::modularity::ContextPtr&)
{
}

muse::modularity::ContextPtr BaseApplication::setupNewContext(const StringList& args)
{
#ifndef MUSE_MODULE_MULTIWINDOWS_SINGLEPROC_MODE
    static bool once = false;
    IF_ASSERT_FAILED(!once) {
        return nullptr;
    }
    once = true;
#endif

    static int lastId = 0;

    modularity::ContextPtr ctxId = std::make_shared<modularity::Context>();
    ++lastId;
    ctxId->id = lastId;

    ContextData& ctx = contextData(ctxId);
    ctx.initializing = true;
    ctx.options = makeContextOptions(args);

    LOGI() << "Creating new context with id: " << ctxId->id;

    QMetaObject::invokeMethod(qApp, [this, ctxId]() {
        showContextSplash(ctxId);
        QMetaObject::invokeMethod(qApp, [this, ctxId]() {
            setupContext(ctxId);
            QMetaObject::invokeMethod(qApp, [this, ctxId]() {
                ContextData& ctx = contextData(ctxId);
                ctx.initializing = false;
                startupScenario(ctxId);
            }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);

    return ctxId;
}

void BaseApplication::setupContext(const muse::modularity::ContextPtr& ctxId)
{
    const IApplication::RunMode runMode = m_appOptions->runMode;
    // Setup
    std::vector<muse::modularity::IContextSetup*>& csetups = contextData(ctxId).setups;

    for (modularity::IContextSetup* s : csetups) {
        s->registerExports();
    }

    for (modularity::IContextSetup* s : csetups) {
        s->resolveImports();
    }

    for (modularity::IContextSetup* s : csetups) {
        s->onPreInit(runMode);
    }

    for (modularity::IContextSetup* s : csetups) {
        s->onInit(runMode);
    }

    for (modularity::IContextSetup* s : csetups) {
        s->onAllInited(runMode);
    }
}

void BaseApplication::destroyContext(const modularity::ContextPtr& ctx)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(ctx) {
        return;
    }

    LOGI() << "Destroying context with id: " << ctx->id;

    auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
                           [&ctx](const ContextData& c) { return c.ctxId->id == ctx->id; });
    if (it == m_contexts.end()) {
        LOGW() << "Context not found: " << ctx->id;
        return;
    }

    //! The context is in the process of initializing.
    //! It is not possible to delete the context until initialization is complete.
    IF_ASSERT_FAILED(!it->initializing) {
        return;
    }

    doDestroyContext(*it);

    m_contexts.erase(it);
}

void BaseApplication::doDestroyContext(const ContextData& data)
{
    for (modularity::IContextSetup* s : data.setups) {
        s->onDeinit();
    }

    qDeleteAll(data.setups);

    modularity::removeIoC(data.ctxId);
}

#ifndef NO_QT_SUPPORT
QWindow* BaseApplication::focusWindow() const
{
    return qApp->focusWindow();
}

bool BaseApplication::notify(QObject* object, QEvent* event)
{
    return qApp->notify(object, event);
}

Qt::KeyboardModifiers BaseApplication::keyboardModifiers() const
{
    return QApplication::keyboardModifiers();
}

#endif

void BaseApplication::restart()
{
    m_finishMode = FinishMode::Restart;
    QCoreApplication::exit();
}

void BaseApplication::finish()
{
    TRACEFUNC;

    doFinish();

    PROFILER_PRINT;
}

void BaseApplication::doFinish()
{
    // Wait Thread Poll
#ifdef QT_CONCURRENT_SUPPORTED
    QThreadPool* globalThreadPool = QThreadPool::globalInstance();
    if (globalThreadPool) {
        LOGI() << "activeThreadCount: " << globalThreadPool->activeThreadCount();
        globalThreadPool->waitForDone();
    }
#endif

// Deinit
    async::processMessages();

// Deinit and delete contexts
    std::vector<muse::modularity::ContextPtr> ctxs = contexts();
    for (auto& c : ctxs) {
        destroyContext(c);
    }

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    m_globalModule->onDeinit();

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDestroy();
    }

    m_globalModule->onDestroy();

// Delete modules
    qDeleteAll(m_modules);
    m_modules.clear();

    delete m_globalModule;
    m_globalModule = nullptr;

    muse::modularity::resetAll();

    if (m_finishMode == IApplication::FinishMode::Restart) {
#ifdef QT_QPROCESS_SUPPORTED
        QString program = qApp->arguments()[0];

        // NOTE: remove the first argument - the program name
        QStringList arguments = qApp->arguments().mid(1);

        QProcess::startDetached(program, arguments);
#else
        NOT_SUPPORTED;
#endif
    }
}

void BaseApplication::processEvents()
{
    qApp->processEvents();
    tickerProvider()->forceSchedule();
}
