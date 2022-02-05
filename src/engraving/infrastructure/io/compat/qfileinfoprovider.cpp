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
#include "qfileinfoprovider.h"

using namespace mu::engraving;

QFileInfoProvider::QFileInfoProvider(const QFileInfo& fileInfo)
    : m_fileInfo(fileInfo)
{
}

QFileInfoProvider::QFileInfoProvider(const QString& filePath)
    : m_fileInfo(filePath)
{
}

QString QFileInfoProvider::absoluteDirPath() const
{
    return m_fileInfo.absolutePath();
}

QString QFileInfoProvider::absoluteFilePath() const
{
    return m_fileInfo.absoluteFilePath();
}

QString QFileInfoProvider::fileName() const
{
    return m_fileInfo.fileName();
}

QString QFileInfoProvider::completeBaseName() const
{
    return m_fileInfo.completeBaseName();
}

QDateTime QFileInfoProvider::birthTime() const
{
    return m_fileInfo.birthTime();
}

QDateTime QFileInfoProvider::lastModified() const
{
    return m_fileInfo.lastModified();
}
