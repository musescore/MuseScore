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
#include "windowsrecentfilescontroller.h"

#include <Windows.h>
#include <shlobj.h>

using namespace mu::userscores;

void WindowsRecentFilesController::addRecentFile(const io::path& path)
{
    std::wstring pathString = path.toStdWString();
    SHAddToRecentDocs(SHARD_PATHW, pathString.c_str());
}

void WindowsRecentFilesController::clearRecentFiles()
{
    // The "docs" seem to say that this should clear the recent files list
    // But in practice it's not sure if that's the case
    SHAddToRecentDocs(0, NULL);
}
