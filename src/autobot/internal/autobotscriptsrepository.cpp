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
#include "autobotscriptsrepository.h"

#include "log.h"

using namespace mu;
using namespace mu::autobot;

RetVal<Scripts> AutobotScriptsRepository::scripts() const
{
    using namespace mu::system;

    io::path scriptsPath = configuration()->scriptsPath();
    LOGD() << "scriptsPath: " << scriptsPath;
    RetVal<io::paths> paths = fileSystem()->scanFiles(scriptsPath, { "*.muas" }, IFileSystem::ScanMode::OnlyCurrentDir);
    if (!paths.ret) {
        return RetVal<Scripts>(paths.ret);
    }

    std::sort(paths.val.begin(), paths.val.end());

    Scripts scripts;
    for (const io::path& p : paths.val) {
        Script s;
        s.path = p;
        s.title = io::basename(p).toQString();

        scripts.push_back(std::move(s));
    }

    return RetVal<Scripts>::make_ok(std::move(scripts));
}
