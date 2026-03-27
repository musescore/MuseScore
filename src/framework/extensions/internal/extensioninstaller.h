#pragma once

#include "../iextensioninstaller.h"

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "../iextensionsprovider.h"
#include "interactive/iinteractive.h"
#include "io/ifilesystem.h"

namespace muse::extensions {
class ExtensionInstaller : public IExtensionInstaller, public async::Asyncable, public muse::Contextable
{
    muse::GlobalInject<IExtensionsConfiguration> configuration;
    muse::GlobalInject<io::IFileSystem> fileSystem;
    muse::ContextInject<IExtensionsProvider> provider = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    ExtensionInstaller(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    Ret isFileSupported(const io::path_t& path) const override;
    void installExtension(const io::path_t& path) override;
    void uninstallExtension(const Uri& uri) override;

private:
    void doInstallExtension(const io::path_t& srcPath);
};
}
