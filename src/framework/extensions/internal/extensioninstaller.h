#pragma once

#include "../iextensioninstaller.h"

#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "../iextensionsprovider.h"

namespace muse::extensions {
class ExtensionInstaller : public IExtensionInstaller
{
    muse::GlobalInject<IExtensionsConfiguration> configuration;
    muse::GlobalInject<IExtensionsProvider> provider;

public:
    ExtensionInstaller() = default;

    Ret isFileSupported(const io::path_t path) const override;
    Ret installExtension(const io::path_t path) override;
};
}
