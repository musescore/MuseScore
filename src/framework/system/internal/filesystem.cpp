/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "filesystem.h"

#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "systemerrors.h"
#include "log.h"

using namespace mu;
using namespace mu::system;

Ret FileSystem::exists(const io::path& path) const
{
    QFileInfo fileInfo(path.toQString());
    if (!fileInfo.exists()) {
        return make_ret(Err::FSNotExist);
    }

    return make_ret(Err::NoError);
}

Ret FileSystem::remove(const io::path& path_) const
{
    QString path = path_.toQString();
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        return fileInfo.isDir() ? removeDir(path) : removeFile(path);
    }

    return make_ret(Err::NoError);
}

Ret FileSystem::copy(const io::path& src, const io::path& dst, bool replace) const
{
    QFileInfo srcFileInfo(src.toQString());
    if (!srcFileInfo.exists()) {
        return make_ret(Err::FSNotExist);
    }

    QFileInfo dstFileInfo(dst.toQString());
    if (dstFileInfo.exists()) {
        if (!replace) {
            return make_ret(Err::FSIsExist);
        }

        Ret ret = remove(dst);
        if (!ret) {
            return ret;
        }
    }

    Ret ret = copyRecursively(src, dst);
    return ret;
}

Ret FileSystem::move(const io::path& src, const io::path& dst, bool replace) const
{
    QFileInfo srcFileInfo(src.toQString());
    if (!srcFileInfo.exists()) {
        return make_ret(Err::FSNotExist);
    }

    QFileInfo dstFileInfo(dst.toQString());
    if (dstFileInfo.exists()) {
        if (!replace) {
            return make_ret(Err::FSIsExist);
        }

        Ret ret = remove(dst);
        if (!ret) {
            return ret;
        }
    }

    if (srcFileInfo.isDir()) {
        if (!QDir().rename(src.toQString(), dst.toQString())) {
            return make_ret(Err::FSMoveErrors);
        }
    } else {
        if (!QFile::rename(src.toQString(), dst.toQString())) {
            return make_ret(Err::FSMoveErrors);
        }
    }

    return make_ret(Ret::Code::Ok);
}

RetVal<QByteArray> FileSystem::readFile(const io::path& filePath) const
{
    RetVal<QByteArray> result;
    Ret ret = exists(filePath);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    QFile file(filePath.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        result.ret = make_ret(Err::FSReadError);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val = file.readAll();

    file.close();

    return result;
}

Ret FileSystem::writeToFile(const io::path& filePath, const QByteArray& data) const
{
    Ret ret = make_ret(Err::NoError);

    QFile file(filePath.toQString());
    if (!file.open(QIODevice::WriteOnly)) {
        ret = make_ret(Err::FSWriteError);
        return ret;
    }

    file.write(data);
    file.close();

    return ret;
}

Ret FileSystem::makePath(const io::path& path) const
{
    if (!QDir().mkpath(path.toQString())) {
        return make_ret(Err::FSMakingError);
    }

    return make_ret(Err::NoError);
}

RetVal<uint64_t> FileSystem::fileSize(const io::path& path) const
{
    RetVal<uint64_t> rv;
    rv.ret = exists(path);
    if (!rv.ret) {
        return rv;
    }

    QFileInfo fi(path.toQString());
    rv.val = static_cast<uint64_t>(fi.size());
    return rv;
}

RetVal<io::paths> FileSystem::scanFiles(const io::path& rootDir, const QStringList& filters, ScanMode mode) const
{
    RetVal<io::paths> result;
    Ret ret = exists(rootDir);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    QDirIterator::IteratorFlags flags = (mode == ScanMode::IncludeSubdirs ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    QDirIterator it(rootDir.toQString(), filters, QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

    while (it.hasNext()) {
        result.val.push_back(it.next());
    }

    result.ret = make_ret(Err::NoError);
    return result;
}

Ret FileSystem::removeFile(const io::path& path) const
{
    QFile file(path.toQString());
    if (!file.remove()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}

Ret FileSystem::removeDir(const io::path& path) const
{
    QDir dir(path.toQString());
    if (!dir.removeRecursively()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}

Ret FileSystem::copyRecursively(const io::path& src, const io::path& dst) const
{
    QString srcPath = src.toQString();
    QString dstPath = dst.toQString();

    QFileInfo srcFileInfo(srcPath);
    if (srcFileInfo.isDir()) {
        QDir dstDir(dstPath);
        dstDir.cdUp();
        if (!dstDir.mkdir(QFileInfo(dstPath).fileName())) {
            return make_ret(Err::FSMakingError);
        }
        QDir srcDir(srcPath);
        const QStringList fileNames = srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString& fileName : fileNames) {
            const QString newSrcPath = srcPath + QLatin1Char('/') + fileName;
            const QString newDstPath = dstPath + QLatin1Char('/') + fileName;
            Ret ret = copyRecursively(newSrcPath, newDstPath);
            if (!ret) {
                return ret;
            }
        }
    } else {
        if (!QFile::copy(srcPath, dstPath)) {
            return make_ret(Err::FSCopyError);
        }
    }

    return make_ret(Err::NoError);
}

void FileSystem::setAttribute(const io::path& path, Attribute attribute) const
{
    switch (attribute) {
    case Attribute::Hidden: {
#ifdef Q_OS_WIN
        const QString nativePath = QDir::toNativeSeparators(path.toQString());
        SetFileAttributes((LPCTSTR)nativePath.unicode(), FILE_ATTRIBUTE_HIDDEN);
#endif
    } break;
    }
    UNUSED(path);
}
