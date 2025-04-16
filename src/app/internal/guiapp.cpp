#include "guiapp.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QThreadPool>
#include <QQuickWindow>

#include "appshell/view/internal/splashscreen/splashscreen.h"
#include "ui/iuiengine.h"

#include "muse_framework_config.h"
#include "ui/graphicsapiprovider.h"

#include "log.h"

using namespace muse;
using namespace muse::ui;
using namespace mu;
using namespace mu::app;
using namespace mu::appshell;

GuiApp::GuiApp(const CmdOptions& options, const modularity::ContextPtr& ctx)
    : muse::BaseApplication(ctx), m_options(options)
{
}

void GuiApp::addModule(muse::modularity::IModuleSetup* module)
{
    m_modules.push_back(module);
}

void GuiApp::perform()
{
    const CmdOptions& options = m_options;

    IApplication::RunMode runMode = options.runMode;
    IF_ASSERT_FAILED(runMode == IApplication::RunMode::GuiApp) {
        return;
    }

    setRunMode(runMode);

#ifdef MUE_BUILD_APPSHELL_MODULE
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

    SplashScreen* splashScreen = nullptr;
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

#if defined(Q_OS_WIN)
    const QString mainQmlFile = "/platform/win/Main.qml";
#elif defined(Q_OS_MACOS)
    const QString mainQmlFile = "/platform/mac/Main.qml";
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    const QString mainQmlFile = "/platform/linux/Main.qml";
#elif defined(Q_OS_WASM)
    const QString mainQmlFile = "/Main.wasm.qml";
#endif

#ifdef MUE_ENABLE_LOAD_QML_FROM_SOURCE
    const QUrl url(QString(appshell_QML_IMPORT) + mainQmlFile);
#else
    const QUrl url(QStringLiteral("qrc:/qml") + mainQmlFile);
#endif

    QObject::connect(engine, &QQmlApplicationEngine::objectCreated, qApp, [](QObject* obj, const QUrl&) {
        QQuickWindow* w = dynamic_cast<QQuickWindow*>(obj);
        //! NOTE It is important that there is a connection to this signal with an error,
        //! otherwise the default action will be performed - displaying a message and terminating.
        //! We will not be able to switch to another backend.
        QObject::connect(w, &QQuickWindow::sceneGraphError, qApp, [](QQuickWindow::SceneGraphError, const QString& msg) {
            LOGE() << "scene graph error: " << msg;
        });
    }, Qt::DirectConnection);

    QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                     qApp, [this, url, splashScreen](QObject* obj, const QUrl& objUrl) {
        if (url != objUrl) {
            return;
        }

        if (!obj) {
            LOGE() << "failed Qml load\n";
            QCoreApplication::exit(-1);
            return;
        }

        // ====================================================
        // Setup modules: onDelayedInit
        // ====================================================

        m_globalModule.onDelayedInit();
        for (modularity::IModuleSetup* m : m_modules) {
            m->onDelayedInit();
        }

        startupScenario()->runOnSplashScreen();

        if (splashScreen) {
            splashScreen->close();
            delete splashScreen;
        }

        // The main window must be shown at this point so KDDockWidgets can read its size correctly
        // and scale all sizes properly. https://github.com/musescore/MuseScore/issues/21148
        obj->setProperty("visible", true);

        startupScenario()->runAfterSplashScreen();
    }, Qt::QueuedConnection);

    QObject::connect(engine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const QQmlError& e : warnings) {
            LOGE() << "error: " << e.toString().toStdString() << "\n";
        }
    });

    // ====================================================
    // Load Main qml
    // ====================================================

    engine->load(url);

#endif // MUE_BUILD_APPSHELL_MODULE
}

void GuiApp::finish()
{
    PROFILER_PRINT;

// Wait Thread Poll
#ifndef Q_OS_WASM
    QThreadPool* globalThreadPool = QThreadPool::globalInstance();
    if (globalThreadPool) {
        LOGI() << "activeThreadCount: " << globalThreadPool->activeThreadCount();
        globalThreadPool->waitForDone();
    }
#endif

    // Engine quit
    ioc()->resolve<muse::ui::IUiEngine>("app")->quit();

    // Deinit

    m_globalModule.invokeQueuedCalls();

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
