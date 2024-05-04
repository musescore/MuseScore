#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <vector>
#include <memory>

#include "global/internal/baseapplication.h"
#include "../cmdoptions.h"

#include "modularity/imodulesetup.h"
#include "global/globalmodule.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "appshell/internal/istartupscenario.h"

namespace mu::app {
class GuiApp : public muse::BaseApplication, public std::enable_shared_from_this<GuiApp>
{
    muse::Inject<muse::IApplication> muapplication;
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider;
    muse::Inject<appshell::IStartupScenario> startupScenario;

public:
    GuiApp(const CmdOptions& options, const muse::modularity::ContextPtr& ctx);

    void addModule(muse::modularity::IModuleSetup* module);

    void perform() override;
    void finish() override;

private:

    CmdOptions m_options;

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule m_globalModule;

    std::vector<muse::modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APP_GUIAPP_H
