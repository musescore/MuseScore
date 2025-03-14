#include "modularity/ioc.h"

#include "global/serialization/zipreader.h"
#include "global/serialization/zipwriter.h"

#include "global/io/internal/filesystem.h"

#include "log.h"

using namespace muse;

void unzip(const std::string& zip_path, const std::string& extract_path)
{
    ZipReader zip(zip_path);
    if (!zip.exists() || zip.hasError()) {
        LOGE() << "Failed to open zip file: " << zip_path;
        return;
    }

    std::shared_ptr<io::IFileSystem> fileSystem = modularity::globalIoc()->resolve<io::IFileSystem>("ziprw");

    for (const auto& info : zip.fileInfoList()) {
        if (info.isDir) {
            fileSystem->makePath(extract_path + "/" + info.filePath.toStdString());
        } else if (info.isFile) {
            io::path_t out_path = extract_path + "/" + info.filePath.toStdString();
            fileSystem->makePath(io::dirpath(out_path));

            fileSystem->writeFile(out_path, zip.fileData(info.filePath.toStdString()));
        }
    }

    zip.close();
}

void zip(const std::string& dir_path, const std::string& zip_path)
{
    ZipWriter zip(zip_path);
    if (zip.hasError()) {
        LOGE() << "Failed to create zip writer for file: " << zip_path;
        return;
    }

    std::shared_ptr<io::IFileSystem> fileSystem = modularity::globalIoc()->resolve<io::IFileSystem>("ziprw");

    RetVal<io::paths_t> files = fileSystem->scanFiles(dir_path, { "*" });
    if (!files.ret) {
        LOGE() << "Failed to scan files in directory: " << dir_path;
        return;
    }

    for (const io::path_t& file : files.val) {
        RetVal<ByteArray> data = fileSystem->readFile(file);
        if (!data.ret) {
            LOGE() << "Failed to read file: " << file.toStdString();
            continue;
        }

        zip.addFile(file.toString().replace(String::fromStdString(dir_path + "/"), u"").toStdString(), data.val);
    }

    zip.close();
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        LOGE() << "Usage: " << argv[0] << " <action> <source> <destination>";
        LOGE() << "Actions: unzip, zip";
        return 1;
    }

    muse::modularity::globalIoc()->registerExport<muse::io::IFileSystem>("ziprw", new muse::io::FileSystem());

    std::string action = argv[1];
    std::string source = argv[2];
    std::string destination = argv[3];

    if (action == "unzip") {
        unzip(source, destination);
    } else if (action == "zip") {
        zip(source, destination);
    } else {
        LOGE() << "Unknown action: " << action;
        return 1;
    }

    return 0;
}
