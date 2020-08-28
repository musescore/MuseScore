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
#include "filesystem.h"

#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "systemerrors.h"

using namespace mu;
using namespace mu::framework;

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

Ret FileSystem::makePath(const io::path& path) const
{
    if (!QDir().mkpath(path.toQString())) {
        return make_ret(Err::FSMakingError);
    }

    return make_ret(Err::NoError);
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

Ret FileSystem::removeFile(const QString& path) const
{
    QFile file(path);
    if (!file.remove()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}

Ret FileSystem::removeDir(const QString& path) const
{
    QDir dir(path);
    if (!dir.removeRecursively()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}
