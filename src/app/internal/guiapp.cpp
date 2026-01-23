#include "guiapp.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <qcontainerfwd.h>
#include <vector>

#include "appshell/widgets/splashscreen/splashscreen.h"
#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/graphicsapiprovider.h"

#include "async/processevents.h"

#include "muse_framework_config.h"
#include "app_config.h"
#ifdef QT_CONCURRENT_SUPPORTED
#include <QThreadPool>
#endif

#include "multiwindowprovider.h"

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

    //! NOTE Just for demonstration
    ioc()->unregister<muse::mi::IMultiInstancesProvider>("app");
    ioc()->registerExport<muse::mi::IMultiInstancesProvider>("app", new MultiWindowProvider());

    m_globalModule.resolveImports();
    m_globalModule.registerApi();
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
    m_globalModule.onPreInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onPreInit(runMode);
    }

#ifdef MUE_ENABLE_SPLASHSCREEN
    static SplashScreen* splashScreen = nullptr;
    if (multiInstancesProvider()->isMainInstance()) {
        splashScreen = new SplashScreen(SplashScreen::Default);
    } else {
        const project::ProjectFile& file = startupScenario()->startupScoreFile();
        if (file.isValid()) {
            if (file.hasDisplayName()) {
                splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false, file.displayName(true /* includingExtension */));
            } else {
                splashScreen = new SplashScreen(SplashScreen::ForNewInstance, false);
            }
        } else if (startupScenario()->isStartWithNewFileAsSecondaryInstance()) {
            splashScreen = new SplashScreen(SplashScreen::ForNewInstance, true);
        } else {
            splashScreen = new SplashScreen(SplashScreen::Default);
        }
    }

    if (splashScreen) {
        splashScreen->show();
    }
#else
    struct SplashScreen {
        void close() {}
    };
    static SplashScreen* splashScreen = nullptr;
#endif

    // ====================================================
    // Setup modules: onInit
    // ====================================================
    m_globalModule.onInit(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onInit(runMode);
    }

    // ====================================================
    // Setup modules: onAllInited
    // ====================================================
    m_globalModule.onAllInited(runMode);
    for (modularity::IModuleSetup* m : m_modules) {
        m->onAllInited(runMode);
    }

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

    QQmlApplicationEngine* engine = muse::modularity::globalIoc()->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();

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

muse::modularity::ContextPtr GuiApp::setupNewContext()
{
    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ++m_lastId;
    ctx->id = m_lastId;

    const CmdOptions& options = m_options;
    IApplication::RunMode runMode = options.runMode;
    IF_ASSERT_FAILED(runMode == IApplication::RunMode::GuiApp) {
        return nullptr;
    }

    LOGI() << "New session created with id: " << ctx->id;

    std::vector<muse::modularity::IContextSetup*>& sessions = m_sessions[ctx->id];

    modularity::IContextSetup* global = m_globalModule.newContext(ctx);
    if (global) {
        sessions.push_back(global);
    }

    for (modularity::IModuleSetup* m : m_modules) {
        modularity::IContextSetup* s = m->newContext(ctx);
        if (s) {
            sessions.push_back(s);
        }
    }

    // Setup
    for (modularity::IContextSetup* s : sessions) {
        s->registerExports();
    }

    for (modularity::IContextSetup* s : sessions) {
        s->resolveImports();
    }

    for (modularity::IContextSetup* s : sessions) {
        s->onPreInit(runMode);
    }

    for (modularity::IContextSetup* s : sessions) {
        s->onInit(runMode);
    }

    for (modularity::IContextSetup* s : sessions) {
        s->onAllInited(runMode);
    }

    // Load main window
    QQmlApplicationEngine* engine = muse::modularity::globalIoc()->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();
    //engine->loadFromModule("MuseScore.AppShell", "Main");
    QString path = ":/qt/qml/MuseScore/AppShell/platform/linux/Main.qml";
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

    // m_globalModule.onDelayedInit();
    // for (modularity::IModuleSetup* m : m_modules) {
    //     m->onDelayedInit();
    // }

    const auto finalizeStartup = [this, obj]() {
        static bool haveFinalized = false;
        // IF_ASSERT_FAILED(!haveFinalized) {
        //     // Only call this once...
        //     return;
        // }

        // if (splashScreen) {
        //     splashScreen->close();
        //     delete splashScreen;
        // }

        // The main window must be shown at this point so KDDockWidgets can read its size correctly
        // and scale all sizes properly. https://github.com/musescore/MuseScore/issues/21148
        // but before that, let's make the window transparent,
        // otherwise the empty window frame will be visible
        // https://github.com/musescore/MuseScore/issues/29630
        // Transparency will be removed after the page loads.
        QQuickWindow* w = dynamic_cast<QQuickWindow*>(obj);
        w->setOpacity(0.01);
        w->setVisible(true);

        startupScenario()->runAfterSplashScreen();
        haveFinalized = true;
    };

    muse::async::Promise<Ret> promise = startupScenario()->runOnSplashScreen();
    promise.onResolve(nullptr, [finalizeStartup](Ret) {
        finalizeStartup();
    });

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

    startupScenario()->setStartupType(options.startup.type);

    if (options.startup.scoreUrl.has_value()) {
        project::ProjectFile file { options.startup.scoreUrl.value() };

        if (options.startup.scoreDisplayNameOverride.has_value()) {
            file.displayNameOverride = options.startup.scoreDisplayNameOverride.value();
        }

        startupScenario()->setStartupScoreFile(file);
    }

    if (options.app.loggerLevel) {
        m_globalModule.setLoggerLevel(options.app.loggerLevel.value());
    }
}
