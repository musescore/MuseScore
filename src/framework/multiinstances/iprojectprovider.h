#ifndef MU_MI_IPROJECTPROVIDER_H
#define MU_MI_IPROJECTPROVIDER_H

#include "modularity/imoduleinterface.h"

#include "global/io/path.h"

namespace mu::mi {
class IProjectProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::mi::IProjectProvider);

public:

    virtual ~IProjectProvider() = default;

    virtual bool isProjectOpened(const io::path_t& path) const = 0;
    virtual bool isAnyProjectOpened() const = 0;
};
}

#endif // MU_MI_IPROJECTPROVIDER_H
