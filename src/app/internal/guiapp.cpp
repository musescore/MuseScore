#include "guiapp.h"

#include <QApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "thirdparty/kors_logger/src/log_base.h"
#include "ui/iuiengine.h"
#include "ui/graphicsapiprovider.h"

#include "appshell/internal/istartupscenario.h"

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
    m_globalModule = new GlobalModule();
    m_globalModule->setApplication(shared_from_this());
    m_globalModule->registerResources();
    m_globalModule->registerExports();
    m_globalModule->registerUiTypes();

    for (modularity::IModuleSetup* m : m_modules) {
        m->setApplication(shared_from_this());
        m->registerResources();
    }

    for (modularity::IModuleSetup* m : m_modules) {
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

    // Process all pending events (see IpcSocket::onReadyRead())
    // so that we can use isFirstWindow() as early as possible
    muse::async::processMessages();

#ifdef MUE_ENABLE_SPLASHSCREEN
    if (multiwindowsProvider()->isFirstWindow()) {
        m_splashScreen = new SplashScreen(SplashScreen::Default);
    } else {
        auto startupScenario = muse::modularity::ioc(iocContext())->resolve<IStartupScenario>("app");
        const project::ProjectFile& file = startupScenario->startupScoreFile();
        if (file.isValid()) {
            if (file.hasDisplayName()) {
                m_splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false, file.displayName(true /* includingExtension */));
            } else {
                m_splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false);
            }
        } else if (startupScenario->isStartWithNewFileAsSecondaryInstance()) {
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
}

GuiApp::Context& GuiApp::context(const muse::modularity::ContextPtr& ctx)
{
    for (Context& c : m_contexts) {
        if (c.ctx->id == ctx->id) {
            return c;
        }
    }

    m_contexts.emplace_back();

    Context& ref = m_contexts.back();
    ref.ctx = ctx;

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

std::vector<muse::modularity::ContextPtr> GuiApp::contexts() const
{
    std::vector<muse::modularity::ContextPtr> ctxs;
    ctxs.reserve(m_contexts.size());
    for (const Context& c : m_contexts) {
        ctxs.push_back(c.ctx);
    }
    return ctxs;
}

size_t GuiApp::contextCount() const
{
    return m_contexts.size();
}

muse::modularity::ContextPtr GuiApp::setupNewContext(const StringList& args)
{
#ifndef MUSE_MODULE_MULTIWINDOWS_SINGLEPROC_MODE
    static bool once = false;
    IF_ASSERT_FAILED(!once) {
        return nullptr;
    }
    once = true;
#endif

    modularity::ContextPtr ctxId = std::make_shared<modularity::Context>();
    ++m_lastId;
    ctxId->id = m_lastId;

    const CmdOptions& options = m_options;
    IApplication::RunMode runMode = options.runMode;
    IF_ASSERT_FAILED(runMode == IApplication::RunMode::GuiApp) {
        return nullptr;
    }

    LOGI() << "New context created with id: " << ctxId->id;

    // Setup
    std::vector<muse::modularity::IContextSetup*>& csetups = context(ctxId).setups;

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

    // Load main window
#if defined(Q_OS_MAC)
    QString platform = "mac";
#elif defined(Q_OS_WIN)
    QString platform = "win";
#else
    QString platform = "linux";
#endif

    QQmlApplicationEngine* engine = muse::modularity::ioc(ctxId)->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();

    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, qApp, [](QObject* obj, const QUrl&) {
        QQuickWindow* w = dynamic_cast<QQuickWindow*>(obj);
        //! NOTE It is important that there is a connection to this signal with an error,
        //! otherwise the default action will be performed - displaying a message and terminating.
        //! We will not be able to switch to another backend.
        QObject::connect(w, &QQuickWindow::sceneGraphError, qApp, [](QQuickWindow::SceneGraphError, const QString& msg) {
            LOGE() << "scene graph error: " << msg;
        });
    }, Qt::DirectConnection);

    QString path = QString(":/qt/qml/MuseScore/AppShell/platform/%1/Main.qml").arg(platform);
    QQmlComponent component = QQmlComponent(engine, path);
    if (!component.isReady()) {
        LOGE() << "Failed to load main qml file, err: " << component.errorString();
        return nullptr;
    }

    QQmlContext* qmlCtx = new QQmlContext(engine);
    qmlCtx->setObjectName(QString("QQmlContext: %1").arg(ctxId ? ctxId->id : 0));
    QmlIoCContext* iocCtx = new QmlIoCContext(qmlCtx);
    iocCtx->ctx = ctxId;
    qmlCtx->setContextProperty("ioc_context", QVariant::fromValue(iocCtx));

    QObject* obj = component.create(qmlCtx);
    if (!obj) {
        LOGE() << "failed Qml load\n";
        QCoreApplication::exit(-1);
        return nullptr;
    }

    auto startupScenario = muse::modularity::ioc(ctxId)->resolve<IStartupScenario>("app");

    //! NOTE Apply startup options from either:
    //! 1. Direct args (single-process mode: openNewWindow passes args)
    //! 2. Command line options (multi-process mode: parsed by CommandLineParser)

    //! Parse known options from args
    auto sessionTypeIdx = args.indexOf(u"--session-type");
    if (sessionTypeIdx != muse::nidx && sessionTypeIdx + 1 < args.size()) {
        startupScenario->setStartupType(args.at(sessionTypeIdx + 1).toStdString());
    } else if (m_options.startup.type.has_value()) {
        startupScenario->setStartupType(m_options.startup.type.value());
    }

    //! Find the first positional argument (not starting with "--") to use as score file
    muse::String scoreArg;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args.at(i).startsWith(u"--")) {
            ++i; // skip the option's value
            continue;
        }
        scoreArg = args.at(i);
        break;
    }

    project::ProjectFile file;
    if (!scoreArg.isEmpty()) {
        file = { QUrl::fromUserInput(scoreArg.toQString(), QDir::currentPath(), QUrl::AssumeLocalFile) };

        size_t dnIdx = args.indexOf(u"--score-display-name-override");
        if (dnIdx != muse::nidx && dnIdx + 1 < args.size()) {
            file.displayNameOverride = args.at(dnIdx + 1);
        }
    } else if (m_options.startup.scoreUrl.has_value()) {
        file = { m_options.startup.scoreUrl.value() };

        if (m_options.startup.scoreDisplayNameOverride.has_value()) {
            file.displayNameOverride = m_options.startup.scoreDisplayNameOverride.value();
        }
    }

    startupScenario->setStartupScoreFile(file);
    startupScenario->runOnSplashScreen();

    QMetaObject::invokeMethod(qApp, [this, ctxId, obj, startupScenario]() {
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
        Context& ctx = context(ctxId);
        ctx.window = dynamic_cast<QQuickWindow*>(obj);
        ctx.window->setOpacity(0.01);
        ctx.window->setVisible(true);

        startupScenario->runAfterSplashScreen();
    }, Qt::QueuedConnection);

    return ctxId;
}

void GuiApp::destroyContext(const modularity::ContextPtr& ctx)
{
    TRACEFUNC;

    if (!ctx) {
        return;
    }

    LOGI() << "Destroying context with id: " << ctx->id;

    auto it = std::find_if(m_contexts.begin(), m_contexts.end(),
                           [&ctx](const Context& c) { return c.ctx->id == ctx->id; });
    if (it == m_contexts.end()) {
        LOGW() << "Context not found: " << ctx->id;
        return;
    }

    if (it->window) {
        it->window->setVisible(false);
    }

    // Engine quit
    muse::modularity::ioc(ctx)->resolve<muse::ui::IUiEngine>("app")->quit();

    for (modularity::IContextSetup* s : it->setups) {
        s->onDeinit();
    }

    qDeleteAll(it->setups);
    m_contexts.erase(it);

    modularity::removeIoC(ctx);
}

void GuiApp::finish()
{
    {
        TRACEFUNC

        m_delayedInitTimer.stop();

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

        BaseApplication::finish();
    }

    PROFILER_PRINT;
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
        m_globalModule->setLoggerLevel(options.app.loggerLevel.value());
    }
}
