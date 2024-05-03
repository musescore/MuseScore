#ifndef MU_APP_APPFACTORY_H
#define MU_APP_APPFACTORY_H

#include "app.h"

namespace mu::app {
class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<App> newApp() const;
};
}

#endif // MU_APP_APPFACTORY_H
