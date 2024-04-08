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

#include <QDateTime>
#include <QUrl>

#include "util.h"

#include "log.h"

using namespace muse;

namespace muse::extensions::apiv1 {
//---------------------------------------------------------
//   FileIO
//---------------------------------------------------------

FileIO::FileIO(QObject* parent)
    : QObject(parent)
{
}

QString FileIO::read()
{
    if (mSource.isEmpty()) {
        emit error("source is empty");
        return QString();
    }
    QUrl url(mSource);
    QString source(mSource);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFile file(source);
    QString fileContent;
    if (file.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream t(&file);
        do {
            line = t.readLine();
            fileContent += line + "\n";
        } while (!line.isNull());
        file.close();
    } else {
        emit error("Unable to open the file");
        return QString();
    }
    return fileContent;
}

bool FileIO::write(const QString& data)
{
    if (mSource.isEmpty()) {
        return false;
    }

    QUrl url(mSource);

    QString source = (url.isValid() && url.isLocalFile()) ? url.toLocalFile() : mSource;

    QFile file(source);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << data;
    file.close();
    return true;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

bool FileIO::remove()
{
    if (mSource.isEmpty()) {
        return false;
    }

    QFile file(mSource);
    return file.remove();
}

bool FileIO::exists()
{
    QFile file(mSource);
    return file.exists();
}

int FileIO::modifiedTime()
{
    if (mSource.isEmpty()) {
        emit error("source is empty");
        return 0;
    }
    QUrl url(mSource);
    QString source(mSource);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFileInfo fileInfo(source);
    return fileInfo.lastModified().toSecsSinceEpoch();
}

void MsProcess::start(const QString& command)
{
    QT_WARNING_PUSH;
    QT_WARNING_DISABLE_DEPRECATED;
    DEPRECATED_USE("startWithArgs(program, [arg1, arg2, ...])");
    QProcess::start(command);
    QT_WARNING_POP;
}

void MsProcess::startWithArgs(const QString& program, const QStringList& args)
{
    QProcess::start(program, args, ReadWrite);
}
}
