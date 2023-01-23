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

#include "crashhandler.h"

#include <QDir>

#include <thirdparty/google_crashpad_client/client/crashpad_client.h>
#include <thirdparty/google_crashpad_client/client/crash_report_database.h>
#include <thirdparty/google_crashpad_client/client/settings.h>

#include "log.h"
#include "global/muversion.h"

using namespace mu::diagnostics;
using namespace crashpad;

CrashHandler::~CrashHandler()
{
    delete m_client;
}

bool CrashHandler::start(const io::path_t& handlerFilePath, const io::path_t& dumpsDir, const std::string& serverUrl)
{
    if (!fileSystem()->exists(handlerFilePath)) {
        LOGE() << "crash handler not exists, path: " << handlerFilePath;
        return false;
    }

    // Cache directory that will store crashpad information and minidumps
#ifdef _MSC_VER
    base::FilePath database(dumpsDir.toStdWString());
#else
    base::FilePath database(dumpsDir.toStdString());
#endif

    // Path to the out-of-process handler executable
#ifdef _MSC_VER
    base::FilePath handler(handlerFilePath.toStdWString());
#else
    base::FilePath handler(handlerFilePath.toStdString());
#endif

    // Optional annotations passed via --annotations to the handler
    std::map<std::string, std::string> annotations = {
        { "sentry[release]", framework::MUVersion::fullVersion().toStdString() }
    };
    // Optional arguments to pass to the handler
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");
    arguments.push_back("--no-upload-gzip");

    std::unique_ptr<CrashReportDatabase> db = crashpad::CrashReportDatabase::Initialize(database);
    if (db != nullptr && db->GetSettings() != nullptr) {
        db->GetSettings()->SetUploadsEnabled(true);
    }

    removePendingLockFiles(dumpsDir);

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

void CrashHandler::removePendingLockFiles(const io::path_t& dumpsDir)
{
#ifdef _MSC_VER
    //! NOTE Different directory structure and no lock file on Windows
    (void)dumpsDir;
    return;
#else
    io::path_t pendingDir = dumpsDir + "/pending";
    RetVal<io::paths_t> rv = fileSystem()->scanFiles(pendingDir, { "*.lock" }, io::ScanMode::FilesInCurrentDir);
    if (!rv.ret) {
        LOGE() << "failed get pending lock files, err: " << rv.ret.toString();
        return;
    }

    for (const io::path_t& p : rv.val) {
        if (!fileSystem()->remove(p)) {
            LOGE() << "failed remove file: " << p;
        }
    }
#endif
}
