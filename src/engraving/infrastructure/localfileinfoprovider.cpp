/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

using namespace muse;
using namespace mu;
using namespace mu::engraving;

LocalFileInfoProvider::LocalFileInfoProvider(const muse::io::path_t& path)
    : m_path(path)
{
}

muse::io::path_t LocalFileInfoProvider::path() const
{
    return io::absolutePath(m_path);
}

muse::io::path_t LocalFileInfoProvider::fileName(bool includingExtension) const
{
    return io::filename(m_path, includingExtension);
}

muse::io::path_t LocalFileInfoProvider::absoluteDirPath() const
{
    return io::absoluteDirpath(m_path);
}

String LocalFileInfoProvider::displayName() const
{
    return String::fromUtf8(fileName(/*includingExtension*/ false).c_str());
}

DateTime LocalFileInfoProvider::birthTime() const
{
    return fileSystem()->birthTime(m_path);
}

DateTime LocalFileInfoProvider::lastModified() const
{
    return fileSystem()->lastModified(m_path);
}
