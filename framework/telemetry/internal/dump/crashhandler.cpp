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

#include "crashhandler.h"

#include <thirdparty/google_crashpad_client/client/crashpad_client.h>
#include <thirdparty/google_crashpad_client/client/crash_report_database.h>
#include <thirdparty/google_crashpad_client/client/settings.h>

#include "log.h"

using namespace mu::telemetry;
using namespace crashpad;

CrashHandler::~CrashHandler()
{
    delete m_client;
}

bool CrashHandler::start(const io::path& handlerFilePath, const io::path& dumpsDir, const std::string& serverUrl)
{
    if (!fileSystem()->exists(handlerFilePath)) {
        LOGE() << "crash handler not exists, path: " << handlerFilePath;
        return false;
    }

    // Cache directory that will store crashpad information and minidumps
    base::FilePath database(dumpsDir.toStdString());

    // Path to the out-of-process handler executable
    // base::FilePath handler("cpad_handler");
    base::FilePath handler(handlerFilePath.toStdString());

    // Optional annotations passed via --annotations to the handler
    std::map<std::string, std::string> annotations;
    // Optional arguments to pass to the handler
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");
    arguments.push_back("--no-upload-gzip");

    std::unique_ptr<CrashReportDatabase> db = crashpad::CrashReportDatabase::Initialize(database);
    if (db != nullptr && db->GetSettings() != nullptr) {
        db->GetSettings()->SetUploadsEnabled(true);
    }

    m_client = new CrashpadClient();
    bool success = m_client->StartHandler(
        handler,
        database,
        database,
        serverUrl,
        annotations,
        arguments,
        true, // restartable
        false // asynchronous_start
        );

    return success;
}
