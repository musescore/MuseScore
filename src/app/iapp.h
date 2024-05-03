#ifndef MU_APP_IAPP_H
#define MU_APP_IAPP_H

#include "commandlineparser.h"

namespace mu::app {
class IApp
{
public:
    virtual ~IApp() = default;

    virtual void perform(const CommandLineParser& commandLineParser) = 0;
    virtual void finish() = 0;
};
}

#endif // MU_APP_IAPP_H
