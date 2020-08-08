#include "fsoperations.h"

#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include "systemerrors.h"

using namespace mu;
using namespace mu::framework;

Ret FsOperations::exists(const QString& path) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return make_ret(Err::FSNotExist);
    }

    return make_ret(Err::NoError);
}

Ret FsOperations::remove(const QString& path) const
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        return fileInfo.isDir() ? removeDir(path) : removeFile(path);
    }

    return make_ret(Err::NoError);
}

QString FsOperations::fileName(const QString& filePath) const
{
    return QFileInfo(filePath).fileName();
}

QString FsOperations::baseName(const QString& filePath) const
{
    return QFileInfo(filePath).baseName();
}

QString FsOperations::dirName(const QString& dirPath) const
{
    return QDir(dirPath).dirName();
}

RetVal<QByteArray> FsOperations::readFile(const QString& filePath) const
{
    RetVal<QByteArray> result;
    Ret ret = exists(filePath);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.ret = make_ret(Err::FSReadError);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val = file.readAll();

    file.close();

    return result;
}

Ret FsOperations::makePath(const QString& path) const
{
    if (!QDir().mkpath(path)) {
        return make_ret(Err::FSMakingError);
    }

    return make_ret(Err::NoError);
}

RetVal<QStringList> FsOperations::scanFiles(const QString& rootDir, const QStringList& filters, ScanMode mode) const
{
    RetVal<QStringList> result;
    Ret ret = exists(rootDir);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    QDirIterator::IteratorFlags flags = (mode == ScanMode::IncludeSubdirs ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    QDirIterator it(rootDir, filters, QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable | QDir::Files, flags);

    while (it.hasNext()) {
        result.val << it.next();
    }

    result.ret = make_ret(Err::NoError);
    return result;
}

Ret FsOperations::removeFile(const QString& path) const
{
    QFile file(path);
    if (!file.remove()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}

Ret FsOperations::removeDir(const QString& path) const
{
    QDir dir(path);
    if (!dir.removeRecursively()) {
        return make_ret(Err::FSRemoveError);
    }

    return make_ret(Err::NoError);
}
