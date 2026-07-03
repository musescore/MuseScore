#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <memory>

#include "ui/internal/guiapplication.h"
#include "../cmdoptions.h"

#include "modularity/ioc.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "appshell/iappshellconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

#include "appshell/widgets/splashscreen/splashscreen.h"

class QQuickWindow;

namespace mu::app {
class MuseScoreGuiApp : public muse::ui::GuiApplication
{
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<appshell::IAppShellConfiguration> appshellConfiguration;
    muse::GlobalInject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;

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
