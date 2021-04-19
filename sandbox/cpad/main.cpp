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
#include <iostream>

#include <client/crashpad_client.h>
#include <client/crash_report_database.h>
#include <client/settings.h>

using namespace std;

static void crash() { volatile int* a = (int*)(NULL); *a = 1; }

using namespace crashpad;

static CrashpadClient* client = nullptr;

static bool startCrashpad()
{
    // Cache directory that will store crashpad information and minidumps
    base::FilePath database("crashpad");
    // Path to the out-of-process handler executable
    // base::FilePath handler("cpad_handler");
    base::FilePath handler("/Users/musescore/Development/MuseScore/sandbox/build-cpad-Desktop_Qt_5_15_1_clang_64bit-Debug/crashpad_handler");
    // URL used to submit minidumps to
    std::string url("https://sentry.musescore.org/api/3/minidump/?sentry_key=1260147a791c40349bbf717b94dc29c4");
    // Optional annotations passed via --annotations to the handler
    std::map<string, string> annotations;// = {{"a_k1", "a_v1"}};
    // Optional arguments to pass to the handler
    std::vector<string> arguments;
    arguments.push_back("--no-rate-limit");
    arguments.push_back("--no-upload-gzip");

    std::unique_ptr<CrashReportDatabase> db = crashpad::CrashReportDatabase::Initialize(database);
    if (db != nullptr && db->GetSettings() != nullptr) {
        db->GetSettings()->SetUploadsEnabled(true);
    }

    client = new CrashpadClient();
    bool success = client->StartHandler(
                handler,
                database,
                database,
                url,
                annotations,
                arguments,
                /* restartable */ true,
                /* asynchronous_start */ false
                );

    return success;
}

int main()
{
    cout << "cpad start" << endl;

    startCrashpad();
    cout << "after startCrashpad" << endl;

    int count = 1000000;
    for (int i = 0; i < count; ++i) {
        if (i == (count - 10)) {
           // cout << "before crash" << endl;
            crash();
            i = 0;
         //   cout << "after crash" << endl;
        }
    }

    cout << "cpad good buy!" << endl;
    
    return 0;
}
