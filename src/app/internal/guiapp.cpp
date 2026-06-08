#include "guiapp.h"

#include "modularity/ioc.h"
#include "appshell/internal/istartupscenario.h"

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
#ifdef MUE_ENABLE_SPLASHSCREEN
        if (m_splashScreen) {
            m_splashScreen->close();
            delete m_splashScreen;
            m_splashScreen = nullptr;
        }
#endif

        startupScenario->runAfterSplashScreen();
    }, Qt::QueuedConnection);
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
