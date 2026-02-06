#include "guiapp.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QTimer>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "thirdparty/kors_logger/src/log_base.h"
#include "ui/iuiengine.h"
#include "ui/graphicsapiprovider.h"

#include "async/processevents.h"

#include "muse_framework_config.h"
#include "app_config.h"

#ifdef MUE_ENABLE_SPLASHSCREEN
#include "appshell/widgets/splashscreen/splashscreen.h"
#else
namespace mu::appshell {
class SplashScreen
{
public:
    void close() {}
};
}
#endif

#ifdef QT_CONCURRENT_SUPPORTED
#include <QThreadPool>
#endif

#include "log.h"

using namespace muse;
using namespace muse::ui;
using namespace mu;
using namespace mu::app;
using namespace mu::appshell;

static int m_lastId = 0;

GuiApp::GuiApp(const CmdOptions& options, const modularity::ContextPtr& ctx)
    : muse::BaseApplication(ctx), m_options(options)
{
}

void GuiApp::addModule(muse::modularity::IModuleSetup* module)
{
    m_modules.push_back(module);
}

void GuiApp::setup()
{
    const CmdOptions& options = m_options;

    IApplication::RunMode runMode = options.runMode;
    IF_ASSERT_FAILED(runMode == IApplication::RunMode::GuiApp) {
        return;
    }

    setRunMode(runMode);

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================
    m_globalModule.setApplication(shared_from_this());
    m_globalModule.registerResources();
    m_globalModule.registerExports();
    m_globalModule.registerUiTypes();

    for (modularity::IModuleSetup* m : m_modules) {
        m->setApplication(shared_from_this());
        m->registerResources();
    }

    for (modularity::IModuleSetup* m : m_modules) {
        m->registerExports();
    }

#ifndef MUSE_MULTICONTEXT_WIP
    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ctx->id = 0;
    std::vector<muse::modularity::IContextSetup*>& csetups = contextSetups(ctx);
    for (modularity::IContextSetup* s : csetups) {
        s->registerExports();
    }
#endif

    m_globalModule.resolveImports();
    m_globalModule.registerApi();
    for (modularity::IModuleSetup* m : m_modules) {
        m->registerUiTypes();
        m->resolveImports();
        m->registerApi();
    }

#ifndef MUSE_MULTICONTEXT_WIP
    for (modularity::IContextSetup* s : csetups) {
        s->resolveImports();
    }
#endif

    // ====================================================
    // Setup modules: apply the command line options
    // ====================================================
    applyCommandLineOptions(options);

    // ====================================================
    // Setup modules: onPreInit
    // ====================================================
    m_globalModule.onPreInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onPreInit(runMode);
    }

#ifndef MUSE_MULTICONTEXT_WIP
    for (modularity::IContextSetup* s : csetups) {
        s->onPreInit(runMode);
    }
#endif

    // Process all pending events (see IpcSocket::onReadyRead())
    // so that we can use windowCount() as early as possible
    muse::async::processMessages();

#ifdef MUE_ENABLE_SPLASHSCREEN
    if (multiwindowsProvider()->windowCount() == 1) { // first
        m_splashScreen = new SplashScreen(SplashScreen::Default);
    } else {
        const project::ProjectFile& file = startupScenario()->startupScoreFile();
        if (file.isValid()) {
            if (file.hasDisplayName()) {
                m_splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false, file.displayName(true /* includingExtension */));
            } else {
                m_splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false);
            }
        } else if (startupScenario()->isStartWithNewFileAsSecondaryInstance()) {
            m_splashScreen = new SplashScreen(SplashScreen::ForNewInstance, true);
        } else {
            m_splashScreen = new SplashScreen(SplashScreen::Default);
        }
    }

    if (m_splashScreen) {
        m_splashScreen->show();
    }
#endif

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    m_globalModule.onInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

#ifndef MUSE_MULTICONTEXT_WIP
    for (modularity::IContextSetup* s : csetups) {
        s->onInit(runMode);
    }
#endif

    // ====================================================
    // Setup modules: onAllInited
    // ====================================================
    m_globalModule.onAllInited(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onAllInited(runMode);
    }

#ifndef MUSE_MULTICONTEXT_WIP
    for (modularity::IContextSetup* s : csetups) {
        s->onAllInited(runMode);
    }
#endif

    // ====================================================
    // Setup modules: onStartApp (on next event loop)
    // ====================================================
    QMetaObject::invokeMethod(qApp, [this]() {
        m_globalModule.onStartApp();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onStartApp();
        }
    }, Qt::QueuedConnection);

    // ====================================================
    // Setup modules: onDelayedInit
    // ====================================================
    QTimer::singleShot(5000, [this]() {
        m_globalModule.onDelayedInit();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onDelayedInit();
        }
    });

    // ====================================================
    // Run
    // ====================================================

    // ====================================================
    // Setup Qml Engine
    // ====================================================
    //! Needs to be set because we use transparent windows for PopupView.
    //! Needs to be called before any QQuickWindows are shown.
    QQuickWindow::setDefaultAlphaBuffer(true);

    //! NOTE Adjust GS Api
    //! We can hide this algorithm in GSApiProvider,
    //! but it is intentionally left here to illustrate what is happening.
    {
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

    QQmlApplicationEngine* engine = ioc()->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();

    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, qApp, [](QObject* obj, const QUrl&) {
        QQuickWindow* w = dynamic_cast<QQuickWindow*>(obj);
        //! NOTE It is important that there is a connection to this signal with an error,
        //! otherwise the default action will be performed - displaying a message and terminating.
        //! We will not be able to switch to another backend.
        QObject::connect(w, &QQuickWindow::sceneGraphError, qApp, [](QQuickWindow::SceneGraphError, const QString& msg) {
            LOGE() << "scene graph error: " << msg;
        });
    }, Qt::DirectConnection);
}

std::vector<muse::modularity::IContextSetup*>& GuiApp::contextSetups(const muse::modularity::ContextPtr& ctx)
{
    for (Context& c : m_contexts) {
        if (c.ctx->id == ctx->id) {
            return c.setups;
        }
    }

    m_contexts.emplace_back();

    Context& ref = m_contexts.back();
    ref.ctx = ctx;

    modularity::IContextSetup* global = m_globalModule.newContext(ctx);
    if (global) {
        ref.setups.push_back(global);
    }

    for (modularity::IModuleSetup* m : m_modules) {
        modularity::IContextSetup* s = m->newContext(ctx);
        if (s) {
            ref.setups.push_back(s);
        }
    }

    return ref.setups;
}

int GuiApp::contextCount() const
{
    return static_cast<int>(m_contexts.size());
}

std::vector<muse::modularity::ContextPtr> GuiApp::contexts() const
{
    std::vector<muse::modularity::ContextPtr> ctxs;
    ctxs.reserve(m_contexts.size());
    for (const Context& c : m_contexts) {
        ctxs.push_back(c.ctx);
    }
    return ctxs;
}

muse::modularity::ContextPtr GuiApp::setupNewContext()
{
    //! NOTE
    //! We're currently in a transitional state from a single global context to multiple contexts.
    //! Therefore, this code will be improved; not everything is yet complete,
    //! for example, there's no way to delete (close) a specific context.
    //! Probably the context initialization needs to be moved to the base class of the app.

#ifndef MUSE_MULTICONTEXT_WIP
    static bool once = false;
    IF_ASSERT_FAILED(!once) {
        return nullptr;
    }
    once = true;
#endif

    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ++m_lastId;
#ifdef MUSE_MULTICONTEXT_WIP
    ctx->id = m_lastId;
#else
    // only global
    ctx->id = 0;
#endif

    const CmdOptions& options = m_options;
    IApplication::RunMode runMode = options.runMode;
    IF_ASSERT_FAILED(runMode == IApplication::RunMode::GuiApp) {
        return nullptr;
    }

    LOGI() << "New context created with id: " << ctx->id;

    // Setup
#ifdef MUSE_MULTICONTEXT_WIP
    std::vector<muse::modularity::IContextSetup*>& csetups = contextSetups(ctx);

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
#endif

    // Load main window
#if defined(Q_OS_MAC)
    QString platform = "mac";
#elif defined(Q_OS_WIN)
    QString platform = "win";
#else
    QString platform = "linux";
#endif

    QQmlApplicationEngine* engine = ioc()->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();

    QString path = QString(":/qt/qml/MuseScore/AppShell/platform/%1/Main.qml").arg(platform);
    QQmlComponent component = QQmlComponent(engine, path);
    if (!component.isReady()) {
        LOGE() << "Failed to load main qml file, err: " << component.errorString();
        return nullptr;
    }

    QQmlContext* qmlCtx = new QQmlContext(engine);
    qmlCtx->setObjectName(QString("QQmlContext: %1").arg(ctx ? ctx->id : 0));
    QmlIoCContext* iocCtx = new QmlIoCContext(qmlCtx);
    iocCtx->ctx = ctx;
    qmlCtx->setContextProperty("ioc_context", QVariant::fromValue(iocCtx));

    QObject* obj = component.create(qmlCtx);
    if (!obj) {
        LOGE() << "failed Qml load\n";
        QCoreApplication::exit(-1);
        return nullptr;
    }

    auto ss = muse::modularity::ioc(ctx)->resolve<IStartupScenario>("app");
    ss->runOnSplashScreen();

    if (m_splashScreen) {
        m_splashScreen->close();
        delete m_splashScreen;
        m_splashScreen = nullptr;
    }

    // The main window must be shown at this point so KDDockWidgets can read its size correctly
    // and scale all sizes properly. https://github.com/musescore/MuseScore/issues/21148
    // but before that, let's make the window transparent,
    // otherwise the empty window frame will be visible
    // https://github.com/musescore/MuseScore/issues/29630
    // Transparency will be removed after the page loads.
    QQuickWindow* w = dynamic_cast<QQuickWindow*>(obj);
    w->setOpacity(0.01);
    w->setVisible(true);

    ss->runAfterSplashScreen();

    return ctx;
}

void GuiApp::finish()
{
    PROFILER_PRINT;

// Wait Thread Poll
#ifdef QT_CONCURRENT_SUPPORTED
    QThreadPool* globalThreadPool = QThreadPool::globalInstance();
    if (globalThreadPool) {
        LOGI() << "activeThreadCount: " << globalThreadPool->activeThreadCount();
        globalThreadPool->waitForDone();
    }
#endif

    // Engine quit
    ioc()->resolve<muse::ui::IUiEngine>("app")->quit();

    // Deinit
    async::processMessages();

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDeinit();
    }

    m_globalModule.onDeinit();

    for (modularity::IModuleSetup* m : m_modules) {
        m->onDestroy();
    }

    m_globalModule.onDestroy();

    // Delete contexts
    for (auto& c : m_contexts) {
        qDeleteAll(c.setups);
    }

    // Delete modules
    qDeleteAll(m_modules);
    m_modules.clear();

    removeIoC();
}

void GuiApp::applyCommandLineOptions(const CmdOptions& options)
{
    if (options.app.revertToFactorySettings) {
        appshellConfiguration()->revertToFactorySettings(options.app.revertToFactorySettings.value());
    }

    if (guitarProConfiguration()) {
        if (options.guitarPro.experimental) {
            guitarProConfiguration()->setExperimental(true);
        }

        if (options.guitarPro.linkedTabStaffCreated) {
            guitarProConfiguration()->setLinkedTabStaffCreated(true);
        }
    }

    // startupScenario()->setStartupType(options.startup.type);

    if (options.startup.scoreUrl.has_value()) {
        project::ProjectFile file { options.startup.scoreUrl.value() };

        if (options.startup.scoreDisplayNameOverride.has_value()) {
            file.displayNameOverride = options.startup.scoreDisplayNameOverride.value();
        }

        // startupScenario()->setStartupScoreFile(file);
    }

    if (options.app.loggerLevel) {
        m_globalModule.setLoggerLevel(options.app.loggerLevel.value());
    }
}
