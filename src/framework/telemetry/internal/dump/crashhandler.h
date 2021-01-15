//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_TELEMETRY_CRASHHANDLER_H
#define MU_TELEMETRY_CRASHHANDLER_H

#include <string>

#include "io/path.h"
#include "modularity/ioc.h"
#include "system/ifilesystem.h"

namespace crashpad {
class CrashpadClient;
}

namespace mu::telemetry {
class CrashHandler
{
    INJECT(telemetry, framework::IFileSystem, fileSystem)

public:
    CrashHandler() = default;
    ~CrashHandler();

    bool start(const io::path& handlerFilePath,const io::path& dumpsDir,const std::string& serverUrl);

private:
    void removePendingLockFiles(const io::path& dumpsDir);

    crashpad::CrashpadClient* m_client = nullptr;
};
}

#endif // MU_TELEMETRY_CRASHHANDLER_H
