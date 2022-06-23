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
#include "fileinfo.h"

#include <QFileInfo>

using namespace mu;
using namespace mu::io;

FileInfo::FileInfo(const path_t& filePath)
    : m_filePath(filePath.toQString())
{
}

QString FileInfo::path() const
{
    int lastSep = m_filePath.lastIndexOf(QLatin1Char('/'));
    if (lastSep == -1) {
#if defined(Q_OS_WIN)
        if (m_filePath.length() >= 2 && m_filePath.at(1) == QLatin1Char(':')) {
            return m_filePath.left(2);
        }
#endif
        return QString(QLatin1Char('.'));
    }
    if (lastSep == 0) {
        return QString(QLatin1Char('/'));
    }
#if defined(Q_OS_WIN)
    if (lastSep == 2 && m_filePath.at(1) == QLatin1Char(':')) {
        return m_filePath.left(lastSep + 1);
    }
#endif
    return m_filePath.left(lastSep);
}

QString FileInfo::filePath() const
{
    return m_filePath;
}

QString FileInfo::canonicalFilePath() const
{
    return fileSystem()->canonicalFilePath(m_filePath).toQString();
}

QString FileInfo::absolutePath() const
{
    return fileSystem()->absolutePath(m_filePath).toQString();
}

QString FileInfo::fileName() const
{
    int lastSep = m_filePath.lastIndexOf(QLatin1Char('/'));
#if defined(Q_OS_WIN)
    if (lastSep == -1 && m_filePath.length() >= 2 && m_filePath.at(1) == QLatin1Char(':')) {
        return m_filePath.mid(2);
    }
#endif
    return m_filePath.mid(lastSep + 1);
}

QString FileInfo::baseName() const
{
    int lastSep = m_filePath.lastIndexOf(QLatin1Char('/'));
    int from = lastSep + 1;
    int firstDot = m_filePath.indexOf(QLatin1Char('.'), from);
    int to = firstDot > 0 ? firstDot : m_filePath.size();
    int length = to - from;

#if defined(Q_OS_WIN)
    if (lastSep == -1 && m_filePath.length() >= 2 && m_filePath.at(1) == QLatin1Char(':')) {
        return m_filePath.mid(2, length - 2);
    }
#endif
    return m_filePath.mid(from, length);
}

QString FileInfo::completeBaseName() const
{
    int lastSep = m_filePath.lastIndexOf(QLatin1Char('/'));
    int from = lastSep + 1;
    int lastDot = m_filePath.lastIndexOf(QLatin1Char('.'));
    int to = lastDot > 0 ? lastDot : m_filePath.size();
    int length = to - from;

#if defined(Q_OS_WIN)
    if (lastSep == -1 && m_filePath.length() >= 2 && m_filePath.at(1) == QLatin1Char(':')) {
        return m_filePath.mid(2, length - 2);
    }
#endif
    return m_filePath.mid(from, length);
}

QString FileInfo::suffix() const
{
    return doSuffix(m_filePath);
}

QString FileInfo::suffix(const path_t& filePath)
{
    return doSuffix(filePath.toQString());
}

QString FileInfo::doSuffix(const QString& filePath)
{
    int lastDot = filePath.lastIndexOf(QLatin1Char('.'));
    if (lastDot == -1) {
        return QString();
    }

    int lastSep = filePath.lastIndexOf(QLatin1Char('/'));
    if (lastDot < lastSep) {
        return QString();
    }

    return filePath.mid(lastDot + 1);
}

DateTime FileInfo::birthTime() const
{
    return fileSystem()->birthTime(m_filePath);
}

DateTime FileInfo::lastModified() const
{
    return fileSystem()->lastModified(m_filePath);
}

bool FileInfo::isRelative() const
{
    return !isAbsolute();
}

bool FileInfo::isAbsolute() const
{
#ifdef Q_OS_WIN
    return (m_filePath.length() >= 3
            && m_filePath.at(0).isLetter()
            && m_filePath.at(1) == ':'
            && m_filePath.at(2) == '/')
           || (!m_filePath.isEmpty()
               && (m_filePath.at(0) == '/'
                   || m_filePath.at(0) == ':'));
#else
    return !m_filePath.isEmpty() && (m_filePath.at(0) == '/' || m_filePath.at(0) == ':');
#endif
}

bool FileInfo::exists() const
{
    return fileSystem()->exists(m_filePath);
}

bool FileInfo::exists(const path_t& filePath)
{
    return fileSystem()->exists(filePath);
}
