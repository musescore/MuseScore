#ifndef MU_APP_IAPP_H
#define MU_APP_IAPP_H

#include "cmdoptions.h"

namespace mu::app {
class IApp
{
public:
    virtual ~IApp() = default;

    virtual void perform(const CmdOptions& options) = 0;
    virtual void finish() = 0;
};
}

#endif // MU_APP_IAPP_H
