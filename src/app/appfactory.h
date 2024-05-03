#ifndef MU_APP_APPFACTORY_H
#define MU_APP_APPFACTORY_H

#include "iapp.h"
#include "global/iapplication.h"

namespace mu::app {
class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<IApp> newApp(muse::IApplication::RunMode& mode) const;

private:
    std::shared_ptr<IApp> newGuiApp() const;
    std::shared_ptr<IApp> newConsoleApp() const;
};
}

#endif // MU_APP_APPFACTORY_H
