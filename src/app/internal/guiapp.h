#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <memory>

#include "ui/internal/guiapplication.h"
#include "../cmdoptions.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "appshell/iappshellconfiguration.h"
#include "network/inetworkconfiguration.h"
#include "audio/main/iaudioconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_TESTFLOW
#include "testflow/itestflowcmdrunner.h"
#endif

#include "appshell/widgets/splashscreen/splashscreen.h"

class QQuickWindow;

namespace mu::app {
class MuseScoreGuiApp : public muse::ui::GuiApplication, public muse::async::Asyncable
{
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<appshell::IAppShellConfiguration> appshellConfiguration;
    muse::GlobalInject<muse::network::INetworkConfiguration> networkConfiguration;
    muse::GlobalInject<muse::audio::IAudioConfiguration> audioConfiguration;
    muse::GlobalInject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;
#ifdef MUSE_MODULE_TESTFLOW
    muse::GlobalInject<muse::testflow::ITestflowCmdRunner> testflowCmdRunner;
#endif

public:
    MuseScoreGuiApp(const std::shared_ptr<MuseScoreCmdOptions>& options);

    void showSplash() override;

private:

    struct SplashConfig {
        appshell::SplashScreen::SplashScreenType type = appshell::SplashScreen::SplashScreenType::Default;
        bool forNewScore = false;
        QString openingFileName;
    };

    SplashConfig splashConfig(const std::shared_ptr<muse::CmdOptions>& options) const;

    void applyCommandLineOptions(const std::shared_ptr<muse::CmdOptions>& options) override;

    std::shared_ptr<muse::CmdOptions> makeContextOptions(const muse::StringList& args) const override;
    void showContextSplash(const muse::modularity::ContextPtr& ctxId) override;
    QString mainWindowQmlPath(const QString& platform) const override;
    void doStartupScenario(const muse::modularity::ContextPtr& ctxId) override;

    appshell::SplashScreen* m_splashScreen = nullptr;
};
}

#endif // MU_APP_GUIAPP_H
