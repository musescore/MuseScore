#ifndef MU_APP_APPFACTORY_H
#define MU_APP_APPFACTORY_H

#include "global/iapplication.h"

#include "cmdoptions.h"

namespace mu::app {
class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<muse::IApplication> newApp(const CmdOptions& options) const;

private:
    std::shared_ptr<muse::IApplication> newGuiApp(const CmdOptions& options) const;
    std::shared_ptr<muse::IApplication> newConsoleApp(const CmdOptions& options) const;

    mutable int m_lastID = 0;
};
}

#endif // MU_APP_APPFACTORY_H
