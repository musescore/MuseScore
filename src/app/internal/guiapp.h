#ifndef MU_APP_GUIAPP_H
#define MU_APP_GUIAPP_H

#include <vector>

#include "iapp.h"
#include "modularity/imodulesetup.h"
#include "global/globalmodule.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "appshell/internal/istartupscenario.h"

namespace mu::app {
class GuiApp : public IApp
{
    INJECT(muse::IApplication, muapplication)
    INJECT(muse::mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(appshell::IStartupScenario, startupScenario)

public:
    GuiApp() = default;

    void addModule(muse::modularity::IModuleSetup* module);

    void perform(const CmdOptions& options) override;
    void finish() override;

private:

    //! NOTE Separately to initialize logger and profiler as early as possible
    muse::GlobalModule m_globalModule;

    std::vector<muse::modularity::IModuleSetup*> m_modules;
};
}

#endif // MU_APP_GUIAPP_H
