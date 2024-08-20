#include "extensioninstaller.h"

#include "global/io/fileinfo.h"
#include "global/serialization/zipreader.h"
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

        bool hasSame = provider()->manifest(m.uri).isValid();
        if (hasSame) {
            LOGI() << "already installed: " << m.uri;
            return make_ok();
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
