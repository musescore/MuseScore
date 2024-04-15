/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "recentfilescontrollerstub.h"

using namespace mu;
using namespace mu::project;

const RecentFilesList& RecentFilesControllerStub::recentFilesList() const
{
    static RecentFilesList l;
    return l;
}

muse::async::Notification RecentFilesControllerStub::recentFilesListChanged() const
{
    static muse::async::Notification n;
    return n;
}

void RecentFilesControllerStub::prependRecentFile(const RecentFile&)
{
}

void RecentFilesControllerStub::moveRecentFile(const muse::io::path_t&, const RecentFile&)
{
}

void RecentFilesControllerStub::clearRecentFiles()
{
}

muse::async::Promise<QPixmap> RecentFilesControllerStub::thumbnail(const RecentFile&) const
{
    return muse::async::Promise<QPixmap>([](auto /*resolve*/, auto reject) {
        return reject(int(Ret::Code::UnknownError), "stub");
    });
}
