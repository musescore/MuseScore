#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <vector>
#include <memory>

#include "global/internal/baseapplication.h"
#include "../cmdoptions.h"

#include "modularity/imodulesetup.h"
#include "global/globalmodule.h"

#include "modularity/ioc.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "appshell/iappshellconfiguration.h"
#include "appshell/internal/istartupscenario.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

namespace mu::appshell {
class SplashScreen;
}

namespace mu::app {
class GuiApp : public muse::BaseApplication, public std::enable_shared_from_this<GuiApp>
{
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<appshell::IAppShellConfiguration> appshellConfiguration;
    muse::GlobalInject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration;

public:
    GuiApp(const CmdOptions& options, const muse::modularity::ContextPtr& ctx);

    void addModule(muse::modularity::IModuleSetup* module);

    void setup() override;
    void finish() override;

    muse::modularity::ContextPtr setupNewContext(const muse::StringList& args = {}) override;
    void destroyContext(const muse::modularity::ContextPtr& ctx) override;
    int contextCount() const override;
    std::vector<muse::modularity::ContextPtr> contexts() const override;

private:
    void applyCommandLineOptions(const CmdOptions& options);

    std::vector<muse::modularity::IContextSetup*>& contextSetups(const muse::modularity::ContextPtr& ctx);

    CmdOptions m_options;

    appshell::SplashScreen* m_splashScreen = nullptr;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule* m_globalModule = nullptr;
    std::vector<muse::modularity::IModuleSetup*> m_modules;

    struct Context {
        muse::modularity::ContextPtr ctx;
        std::vector<muse::modularity::IContextSetup*> setups;
    };

    std::vector<Context> m_contexts;
};
}

#endif // MU_APP_GUIAPP_H
