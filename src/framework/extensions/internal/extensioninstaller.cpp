#include "extensioninstaller.h"

#include "global/io/fileinfo.h"
#include "global/serialization/zipreader.h"
#include "global/translation.h"
#include "global/uuid.h"

#include "../extensionstypes.h"
#include "../extensionserrors.h"
#include "extensionsloader.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;

Ret ExtensionInstaller::isFileSupported(const io::path_t path) const
{
    bool isExt = io::FileInfo(path).suffix() == SINGLE_FILE_EXT;
    if (isExt) {
        return muse::make_ok();
    }
    return muse::make_ret(Ret::Code::NotSupported);
}

Ret ExtensionInstaller::installExtension(const io::path_t srcPath)
{
    // check manifest and duplication
    {
        ZipReader zip(srcPath);
        ByteArray data = zip.fileData("manifest.json");
        if (data.empty()) {
            LOGE() << "not found manifest.json in: " << srcPath;
            return make_ret(Err::ExtBadFormat);
        }

        ExtensionsLoader loader;
        Manifest m = loader.parseManifest(data);

        Manifest existingManifest = provider()->manifest(m.uri);
        bool alreadyInstalled = existingManifest.isValid();
        if (alreadyInstalled && existingManifest.version == m.version) {
            LOGI() << "already installed: " << m.uri;

            interactive()->info(trc("extensions", "The extension is already installed."), std::string(),
                                { interactive()->buttonData(IInteractive::Button::Ok) });

            return make_ok();
        }

        if (alreadyInstalled && !existingManifest.isUserExtension) {
            interactive()->error(trc("extensions", "This extension cannot be updated."), std::string(),
                                 { interactive()->buttonData(IInteractive::Button::Ok) });

            return make_ok();
        }

        if (alreadyInstalled) {
            std::string text = qtrc("extensions", "Another version of the extension “%1” is already installed (version %2). "
                                                  "Do you want to replace it with version %3?")
                               .arg(existingManifest.title, existingManifest.version, m.version).toStdString();

            IInteractive::Result result = interactive()->question(trc("extensions", "Update Extension"), text, {
                interactive()->buttonData(IInteractive::Button::Cancel),
                interactive()->buttonData(IInteractive::Button::Ok)
            });

            if (result.button() == int(IInteractive::Button::Ok)) {
                provider()->removeExtension(existingManifest.uri);
            } else {
                return make_ok();
            }
        }
    }

    // unpack
    Ret ret;
    {
        io::path_t dstPath = configuration()->userPath() + "/"
                             + io::FileInfo(srcPath).baseName() + "_"
                             + Uuid::gen();

        ZipUnpack zip;
        ret = zip.unpack(srcPath, dstPath);
        if (!ret) {
            LOGE() << "failed unpack from: " << srcPath << ", to: " << dstPath << ", err: " << ret.toString();
        } else {
            LOGI() << "success unpack from: " << srcPath << ", to: " << dstPath;
        }
    }

    // reload
    if (ret) {
        provider()->reloadExtensions();
    }

    return ret;
}
