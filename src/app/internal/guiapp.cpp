#include "guiapp.h"

#include <QQuickWindow>
#include <QTimer>

#include "modularity/ioc.h"
#include "appshell/internal/istartupscenario.h"
#include "dockwindow/idockwindowprovider.h"
#include "dockwindow/idockwindow.h"

#include "commandlineparser.h"

#include "muse_framework_config.h"
#include "app_config.h"

#include "log.h"

using namespace muse;
using namespace mu;
using namespace mu::app;
using namespace mu::appshell;

MuseScoreGuiApp::MuseScoreGuiApp(const std::shared_ptr<MuseScoreCmdOptions>& options)
    : muse::ui::GuiApplication(options)
{
}

MuseScoreGuiApp::SplashConfig MuseScoreGuiApp::splashConfig(const std::shared_ptr<CmdOptions>& opt) const
{
    std::shared_ptr<MuseScoreCmdOptions> options = std::dynamic_pointer_cast<MuseScoreCmdOptions>(opt);
    IF_ASSERT_FAILED(options) {
        return SplashConfig();
    }

    SplashConfig cfg;
    cfg.type = SplashScreen::Default;

    if (options->startup.type.has_value()) {
        if (options->startup.type.value() == "start-with-new") {
            cfg.type = SplashScreen::ForNewInstance;
            cfg.forNewScore = true;
        }
    } else if (options->startup.scoreUrl.has_value()) {
        project::ProjectFile file { options->startup.scoreUrl.value() };

        if (options->startup.scoreDisplayNameOverride.has_value()) {
            file.displayNameOverride = options->startup.scoreDisplayNameOverride.value();
        }

        cfg.type = SplashScreen::ForNewInstance;
        cfg.forNewScore = false;
        if (file.hasDisplayName()) {
            cfg.openingFileName = file.displayName(true /* includingExtension */);
        }
    }

    return cfg;
}

void MuseScoreGuiApp::showSplash()
{
#ifdef MUE_ENABLE_SPLASHSCREEN
    SplashConfig cfg = splashConfig(m_appOptions);
    if (cfg.type == SplashScreen::Default) {
        m_splashScreen = new SplashScreen(SplashScreen::Default);
        m_splashScreen->show();
    }
#endif
}

void MuseScoreGuiApp::showContextSplash(const muse::modularity::ContextPtr& ctxId)
{
 #ifdef MUE_ENABLE_SPLASHSCREEN
    SplashConfig cfg = splashConfig(contextData(ctxId).options);
    if (cfg.type == SplashScreen::ForNewInstance) {
        m_splashScreen = new SplashScreen(cfg.type, cfg.forNewScore, cfg.openingFileName);
        m_splashScreen->show();
    }
#endif
}

std::shared_ptr<muse::CmdOptions> MuseScoreGuiApp::makeContextOptions(const muse::StringList& args) const
{
    if (args.size() > 0) {
        std::vector<std::string> args_ = args.toStdStringList();
        args_.insert(args_.begin(), "dummy/path/to/app.exe");  // for compatibility
        const int argc = static_cast<int>(args_.size());
        std::vector<char*> argv(argc + 1, nullptr);
        for (int i = 0; i < argc; ++i) {
            argv[i] = args_[i].data();
        }
        argv[argc] = nullptr;

        CommandLineParser commandLineParser;
        commandLineParser.init();
        commandLineParser.parse(argc, argv.data());
        return commandLineParser.options();
    } else {
        return m_appOptions;
    }
}

QString MuseScoreGuiApp::mainWindowQmlPath(const QString& platform) const
{
    return QString(":/qt/qml/MuseScore/AppShell/platform/%1/Main.qml").arg(platform);
}

void MuseScoreGuiApp::doStartupScenario(const muse::modularity::ContextPtr& ctxId)
{
    auto startupScenario = muse::modularity::ioc(ctxId)->resolve<IStartupScenario>("app");

    //! NOTE Apply startup options
    const std::shared_ptr<MuseScoreCmdOptions> options = std::dynamic_pointer_cast<MuseScoreCmdOptions>(contextData(ctxId).options);
    IF_ASSERT_FAILED(options) {
        return;
    }

    startupScenario->setStartupType(options->startup.type);

    if (options->startup.scoreUrl.has_value()) {
        project::ProjectFile file = { options->startup.scoreUrl.value() };

        if (options->startup.scoreDisplayNameOverride.has_value()) {
            file.displayNameOverride = options->startup.scoreDisplayNameOverride.value();
        }

        startupScenario->setStartupScoreFile(file);
    }

    startupScenario->runOnSplashScreen();

    QMetaObject::invokeMethod(qApp, [this, ctxId, startupScenario]() {
        auto dockWindowProvider = muse::modularity::ioc(ctxId)->resolve<muse::dock::IDockWindowProvider>("app");
        muse::dock::IDockWindow* dockWindow = dockWindowProvider ? dockWindowProvider->window() : nullptr;
        if (dockWindow) {
            dockWindow->currentPageChanged().onNotify(this, [this, ctxId, dockWindow]() {
                dockWindow->currentPageChanged().disconnect(this);

                QMetaObject::invokeMethod(qApp, [this, ctxId]() {
                    showMainWindowAndCloseSplash(ctxId);
                }, Qt::QueuedConnection);
            });
        } else {
            //! NOTE No dock window - no page loading to wait for
            showMainWindowAndCloseSplash(ctxId);
        }

        startupScenario->runAfterSplashScreen();
    }, Qt::QueuedConnection);
}

void MuseScoreGuiApp::showMainWindowAndCloseSplash(const muse::modularity::ContextPtr& ctxId)
{
    auto it = m_windows.find(ctxId->id);
    QQuickWindow* window = it != m_windows.end() ? it->second : nullptr;
    if (!window) {
        closeSplash();
        return;
    }

    //! NOTE Close the splash only after the window has rendered its first frame,
    //! so that the empty window is never visible
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = QObject::connect(window, &QQuickWindow::frameSwapped, qApp, [this, conn]() {
        QObject::disconnect(*conn);
        closeSplash();
    }, Qt::QueuedConnection);

    //! NOTE Safety net for the case when the window never renders a frame;
    //! long enough so that it cannot fire before the first frame of a healthy startup
    static constexpr int SPLASH_CLOSE_TIMEOUT_MS = 10000;
    QTimer::singleShot(SPLASH_CLOSE_TIMEOUT_MS, qApp, [this]() {
        if (m_splashScreen) {
            LOGW() << "the main window has not rendered any frame, closing the splash screen forcibly";
            closeSplash();
        }
    });

    window->setVisible(true);
}

void MuseScoreGuiApp::closeSplash()
{
#ifdef MUE_ENABLE_SPLASHSCREEN
    if (m_splashScreen) {
        m_splashScreen->close();
        delete m_splashScreen;
        m_splashScreen = nullptr;
    }
#endif
}

void MuseScoreGuiApp::applyCommandLineOptions(const std::shared_ptr<CmdOptions>& opt)
{
    GuiApplication::applyCommandLineOptions(opt);

    std::shared_ptr<MuseScoreCmdOptions> options = std::dynamic_pointer_cast<MuseScoreCmdOptions>(opt);
    IF_ASSERT_FAILED(options) {
        return;
    }

    if (options->app.revertToFactorySettings) {
        appshellConfiguration()->revertToFactorySettings(options->app.revertToFactorySettings.value());
    }

    if (guitarProConfiguration()) {
        if (options->guitarPro.experimental) {
            guitarProConfiguration()->setExperimental(true);
        }

        if (options->guitarPro.linkedTabStaffCreated) {
            guitarProConfiguration()->setLinkedTabStaffCreated(true);
        }
    }
}
