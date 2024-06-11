#pragma once

#include "modularity/imoduleinterface.h"

#include "global/types/ret.h"
#include "global/io/path.h"

namespace muse::extensions {
class IExtensionInstaller : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionInstaller)
public:

    virtual ~IExtensionInstaller() = default;

    virtual Ret isFileSupported(const io::path_t path) const = 0;
    virtual Ret installExtension(const io::path_t path) = 0;
};
}
