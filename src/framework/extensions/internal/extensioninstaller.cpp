#include "extensioninstaller.h"

#include "global/io/fileinfo.h"
#include "global/serialization/zipreader.h"
#include "global/translation.h"
#include "global/uuid.h"

#include "../extensionstypes.h"
#include "extensionsloader.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;

Ret ExtensionInstaller::isFileSupported(const io::path_t& path) const
{
    bool isExt = io::FileInfo(path).suffix() == SINGLE_FILE_EXT;
    if (isExt) {
        return muse::make_ok();
    }
    return muse::make_ret(Ret::Code::NotSupported);
}

void ExtensionInstaller::installExtension(const io::path_t& srcPath)
{
    // check manifest and duplication

    const ZipReader zip(srcPath);
    const ByteArray data = zip.fileData("manifest.json");
    if (data.empty()) {
        LOGE() << "not found manifest.json in: " << srcPath;
        return;
    }

    const ExtensionsLoader loader;
    const Manifest m = loader.parseManifest(data);

    const Manifest existingManifest = provider()->manifest(m.uri);
    const bool alreadyInstalled = existingManifest.isValid();

    if (!alreadyInstalled) {
        doInstallExtension(srcPath);
        return;
    }

    if (existingManifest.version == m.version) {
        LOGI() << "already installed: " << m.uri;

        interactive()->info(trc("extensions", "The extension is already installed."), std::string(),
                            { interactive()->buttonData(IInteractive::Button::Ok) });

        return;
    }

    if (!existingManifest.isRemovable) {
        interactive()->error(trc("extensions", "This extension cannot be updated."),
                             trc("extensions", "The currently installed version cannot be uninstalled."),
                             { interactive()->buttonData(IInteractive::Button::Ok) });

        return;
    }

    const std::string text = qtrc("extensions", "Another version of the extension “%1” is already installed (version %2). "
                                                "Do you want to replace it with version %3?")
                             .arg(existingManifest.title, existingManifest.version, m.version).toStdString();

    interactive()->question(trc("extensions", "Update extension"),
                            text,
                            { IInteractive::Button::Cancel, IInteractive::Button::Ok })
    .onResolve(this, [this, existingManifest, srcPath](const IInteractive::Result& result) {
        if (result.isButton(IInteractive::Button::Ok)) {
            uninstallExtension(existingManifest.uri);
            doInstallExtension(srcPath);
        }
    });
}

void ExtensionInstaller::doInstallExtension(const io::path_t& srcPath)
{
    // unpack
    const io::path_t dstPath = configuration()->userPath() + "/"
                               + io::FileInfo(srcPath).baseName() + "_"
                               + Uuid::gen();

    ZipUnpack zip;
    Ret ret = zip.unpack(srcPath, dstPath);
    if (!ret) {
        LOGE() << "failed unpack from: " << srcPath << ", to: " << dstPath << ", err: " << ret.toString();
        return;
    }

    LOGI() << "success unpack from: " << srcPath << ", to: " << dstPath;

    // reload
    provider()->reloadExtensions();
}

void ExtensionInstaller::uninstallExtension(const Uri& uri)
{
    Manifest manifest = provider()->manifest(uri);
    if (!manifest.isValid()) {
        LOGE() << "Manifest not found for URI: " << uri.toString();
        return;
    } else if (!manifest.isRemovable) {
        LOGE() << "Extension is not removable: " << manifest.uri.toString();
        return;
    }

    Ret ret = fileSystem()->remove(io::dirpath(manifest.path));
    if (!ret) {
        LOGE() << "Failed to delete the folder: " << manifest.path << ", err: " << ret.toString();
        return;
    }

    provider()->reloadExtensions();
}
