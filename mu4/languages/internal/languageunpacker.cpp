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
#include "languageunpacker.h"

#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>

#include "thirdparty/qzip/qzipreader_p.h"
#include "log.h"
#include "../ilanguageunpacker.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::languages;

Ret LanguageUnpacker::unpack(const QString& languageCode, const QString& source, const QString& destination) const
{
    Ret destinationWritable = checkDirectoryIsWritable(destination);
    if (!destinationWritable) {
        return destinationWritable;
    }

    MQZipReader zipFile(source);

    qint64 totalZipSize = 0;
    for (const MQZipReader::FileInfo& fileInfo : zipFile.fileInfoList()) {
        totalZipSize += fileInfo.size;
    }

    Ret freeSpace = checkFreeSpace(destination, totalZipSize);
    if (!freeSpace) {
        return freeSpace;
    }

    Ret remove = removePreviousVersion(destination, languageCode);
    if (!remove) {
        return remove;
    }

    Ret unzipLanguage = unzip(&zipFile, destination);
    return unzipLanguage;
}

Ret LanguageUnpacker::checkDirectoryIsWritable(const QString& directoryPath) const
{
    QFileInfo destinationDirInfo(directoryPath);
    if (!destinationDirInfo.isWritable()) {
        return make_ret(Err::UnpackDestinationReadOnly);
    }

    return make_ret(Err::NoError);
}

Ret LanguageUnpacker::checkFreeSpace(const QString& directoryPath, quint64 neededSpace) const
{
    QStorageInfo destinationStorageInfo(directoryPath);
    if (neededSpace > quint64(destinationStorageInfo.bytesAvailable())) {
        return make_ret(Err::UnpackNoFreeSpace);
    }

    return make_ret(Err::NoError);
}

Ret LanguageUnpacker::removePreviousVersion(const QString& path, const QString& languageCode) const
{
    QDir languageDir(path);
    QStringList files = languageDir.entryList({ QString("*%1.qm").arg(languageCode) }, QDir::Files);
    for (const QString& fileName: files) {
        QString filePath(languageDir.absolutePath() + "/" + fileName);
        QFile file(filePath);
        if (!file.remove()) {
            LOGE() << "Error remove file" << filePath << file.errorString();
            return make_ret(Err::UnpackErrorRemovePreviousVersion);
        }
    }

    return make_ret(Err::NoError);
}

Ret LanguageUnpacker::unzip(const MQZipReader* zip, const QString& destination) const
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
