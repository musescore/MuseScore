//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "extensionunpacker.h"

#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>
#include <QVersionNumber>

#include "thirdparty/qzip/qzipreader_p.h"
#include "log.h"
#include "../extensionserrors.h"

using namespace mu;
using namespace mu::extensions;

Ret ExtensionUnpacker::unpack(const QString& source, const QString& destination) const
{
    Ret destinationWritable = isDirectoryWritable(destination);
    if (!destinationWritable) {
        return destinationWritable;
    }

    MQZipReader zipFile(source);

    qint64 totalZipSize = 0;
    for (const MQZipReader::FileInfo& fileInfo : zipFile.fileInfoList()) {
        totalZipSize += fileInfo.size;
    }

    Ret freeSpace = hasFreeSpace(destination, totalZipSize);
    if (!freeSpace) {
        return freeSpace;
    }

    RetVal2<QString, QVersionNumber> meta = extensionMeta(&zipFile);
    if (!meta.ret) {
        return meta.ret;
    }

    QString extensionId = meta.val1;
    QString extensionDirPath(destination + "/" + extensionId);
    QVersionNumber version = meta.val2;

    Ret actualVersion = checkActualVersion(destination, extensionId, version);
    if (!actualVersion) {
        bool previousVersionError = actualVersion.code() == static_cast<int>(Err::UnpackPreviousVersionExists);
        if (previousVersionError) {
            Ret remove = removePreviousVersion(extensionDirPath);
            if (!remove) {
                return remove;
            }
        } else {
            return actualVersion;
        }
    }

    Ret unzipExtension = unzip(&zipFile, extensionDirPath);
    return unzipExtension;
}

Ret ExtensionUnpacker::isDirectoryWritable(const QString& directoryPath) const
{
    QFileInfo destinationDirInfo(directoryPath);
    if (!destinationDirInfo.isWritable()) {
        return make_ret(Err::UnpackDestinationReadOnly);
    }

    return make_ret(Err::NoError);
}

Ret ExtensionUnpacker::hasFreeSpace(const QString& directoryPath, qint64 neededSpace) const
{
    QStorageInfo destinationStorageInfo(directoryPath);
    if (neededSpace > destinationStorageInfo.bytesAvailable()) {
        return make_ret(Err::UnpackNoFreeSpace);
    }

    return make_ret(Err::NoError);
}

RetVal2<QString, QVersionNumber> ExtensionUnpacker::extensionMeta(const MQZipReader* zip) const
{
    QByteArray mdba = zip->fileData("metadata.json");

    QJsonDocument loadDoc = QJsonDocument::fromJson(mdba);
    QJsonObject mdObject = loadDoc.object();
    QString extensionId = mdObject["id"].toString();
    QVersionNumber version = QVersionNumber::fromString(mdObject["version"].toString());

    mu::RetVal2<QString, QVersionNumber> result;
    if (extensionId.isEmpty() || version.isNull()) {
        result.ret = make_ret(Err::UnpackInvalidStructure);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val1 = extensionId;
    result.val2 = version;

    return result;
}

Ret ExtensionUnpacker::checkActualVersion(const QString& destination, const QString& extensionId,
                                          const QVersionNumber& version) const
{
    QString actualExtensionMetaDir = destination + "/" + extensionId;
    if (!fileSystem()->exists(actualExtensionMetaDir)) {
        return make_ret(Err::NoError);
    }

    QString actualExtensionMetaFilePath = destination + "/" + extensionId + "/" + "metadata.json";

    RetVal<QByteArray> fileBytes = fileSystem()->readFile(actualExtensionMetaFilePath);
    if (!fileBytes.ret) {
        LOGE() << "Error open old extension meta file" << actualExtensionMetaFilePath;
        return make_ret(Err::UnpackInvalidOldExtension);
    }

    QJsonDocument loadDoc = QJsonDocument::fromJson(fileBytes.val);
    QJsonObject mdObject = loadDoc.object();
    QVersionNumber actualVersion = QVersionNumber::fromString(mdObject["version"].toString());

    if (version <= actualVersion) {
        return make_ret(Err::UnpackNoActualVersion);
    }

    return make_ret(Err::NoError);
}

Ret ExtensionUnpacker::removePreviousVersion(const QString& path) const
{
    if (!fileSystem()->remove(path)) {
        LOGE() << "Error remove previous version" << path;
        return make_ret(Err::UnpackErrorRemovePreviousVersion);
    }

    return make_ret(Err::NoError);
}

Ret ExtensionUnpacker::unzip(const MQZipReader* zip, const QString& destination) const
{
    QDir destinationDir(destination);
    if (!destinationDir.exists()) {
        destinationDir.mkpath(destinationDir.absolutePath());
    }

    if (!zip->extractAll(destination)) {
        return make_ret(Err::UnpackError);
    }

    return make_ret(Err::NoError);
}
