#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <vector>
#include <memory>
#include <QQmlApplicationEngine>

#include "global/internal/baseapplication.h"
#include "../cmdoptions.h"

#include "modularity/imodulesetup.h"
#include "global/globalmodule.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "appshell/iappshellconfiguration.h"
#include "appshell/internal/istartupscenario.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"
#include "appshell/view/internal/splashscreen/splashscreen.h"

namespace mu::app {
class GuiApp : public muse::BaseApplication, public std::enable_shared_from_this<GuiApp>
{
    muse::Inject<muse::IApplication> muapplication;
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider;
    muse::Inject<appshell::IAppShellConfiguration> appshellConfiguration;
    muse::Inject<appshell::IStartupScenario> startupScenario;
    muse::Inject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;

public:
    GuiApp(const CmdOptions& options, const muse::modularity::ContextPtr& ctx);

    void addModule(muse::modularity::IModuleSetup* module);

    void perform() override;
    void finish() override;

private:
    void applyCommandLineOptions(const CmdOptions& options);
    // Loads the Main.qml file to the Qml Engine
    QQmlApplicationEngine* loadApplication();

    // Sets up modules
    // - Loads .qrc files
    // - Register exports (so that you can do `ioc()->resolve<SomeType>("moduleName");`)
    // - Calls events (`onPreInit`, `onInit`, ...)
    void setupModules();
    // Cleanup work to modules. Sends `onDeinit` and `onDestroy` events.
    void deinitModules();

    SplashScreen *m_splashScreen;
    void displaySplashScreen();
    void hideSplashScreen();

    // Sets up a signal so that when the QQmlApplicationEngine is loaded,
    // - makes sure the Main.qml was in fact loaded
    // - the splash screen is closed and deleted
    // - call onDelayedInit on all modules
    void engineLoadedWork();

    // Makes sure to log SceneGraph errors of the Qml Engine to OUR logging system.
    // Overrides default behaviour (logging & closing the application).
    void logSceneGraphErrors();
    // Logs all QQmlApplicationEngine messages through our logging system (log.h).
    void logQmlEngineMessages();

    CmdOptions m_options;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule m_globalModule;

    std::vector<muse::modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APP_GUIAPP_H
