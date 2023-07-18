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

#ifndef MU_DIAGNOSTICS_CRASHHANDLER_H
#define MU_DIAGNOSTICS_CRASHHANDLER_H

#include <string>

#include "io/path.h"
#include "modularity/ioc.h"
#include "io/ifilesystem.h"

namespace crashpad {
class CrashpadClient;
}

namespace mu::diagnostics {
class CrashHandler
{
    INJECT(io::IFileSystem, fileSystem)

public:
    CrashHandler() = default;
    ~CrashHandler();

    bool start(const io::path_t& handlerFilePath, const io::path_t& dumpsDir, const std::string& serverUrl);

private:
    void removePendingLockFiles(const io::path_t& dumpsDir);

    crashpad::CrashpadClient* m_client = nullptr;
};
}

#endif // MU_DIAGNOSTICS_CRASHHANDLER_H
