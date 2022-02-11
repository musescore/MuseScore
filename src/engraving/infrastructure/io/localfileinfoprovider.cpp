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
#include "localfileinfoprovider.h"

#include <QFileInfo>

using namespace mu;
using namespace mu::engraving;

LocalFileInfoProvider::LocalFileInfoProvider(const io::path& path)
    : m_path(path)
{
}

io::path LocalFileInfoProvider::path() const
{
    return io::absolutePath(m_path);
}

io::path LocalFileInfoProvider::fileName(bool includingExtension) const
{
    return io::filename(m_path, includingExtension);
}

io::path LocalFileInfoProvider::absoluteDirPath() const
{
    return io::absoluteDirpath(m_path);
}

QDateTime LocalFileInfoProvider::birthTime() const
{
    return QFileInfo(m_path.toQString()).birthTime();
}

QDateTime LocalFileInfoProvider::lastModified() const
{
    return QFileInfo(m_path.toQString()).lastModified();
}
