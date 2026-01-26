#ifndef MUSE_MI_IPROJECTPROVIDER_H
#define MUSE_MI_IPROJECTPROVIDER_H

#include "modularity/imoduleinterface.h"

#include "global/io/path.h"

namespace muse::mi {
class IProjectProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(muse::mi::IProjectProvider);

public:

    virtual ~IProjectProvider() = default;

    virtual bool isProjectOpened(const io::path_t& path) const = 0;
    virtual bool isAnyProjectOpened() const = 0;
};
}

#endif // MUSE_MI_IPROJECTPROVIDER_H
