#pragma once

#include "../iextensioninstaller.h"

#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "../iextensionsprovider.h"
#include "global/iinteractive.h"
#include "io/ifilesystem.h"

namespace muse::extensions {
class ExtensionInstaller : public IExtensionInstaller
{
    muse::GlobalInject<IExtensionsConfiguration> configuration;
    muse::GlobalInject<IExtensionsProvider> provider;
    muse::GlobalInject<muse::IInteractive> interactive;
    muse::GlobalInject<io::IFileSystem> fileSystem;

public:
    ExtensionInstaller() = default;

    Ret isFileSupported(const io::path_t path) const override;
    Ret installExtension(const io::path_t path) override;
};
}
