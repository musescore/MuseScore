//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Nikolaos Hatzopoulos (nickhatz@csu.fullerton.edu)
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

#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#pragma once

#if defined(Q_OS_WIN32)
#include "client/windows/handler/exception_handler.h"
#endif

#include <iostream>
#include <fstream>
#include <windows.h>

using std::string;
using std::wstring;
using std::pair;
using std::endl;
using std::ifstream;

namespace Breakpad {

    int AnnotateCrashReport(string aKey, string aData);
    int PrintMyCrashReport();
    bool launcher(wstring program, wstring minidump_path, wstring metadata_path);
    void writeMyCrashReport(wstring mypath);
    wstring str2wstr(string mystr);
    string wstr2str(wstring mystr);
    string get_musescore_path();
    bool file_exists(string name);
    string get_crash_reporter_path();

    class CrashHandlerPrivate;
    class CrashHandler
    {
    public:

        static CrashHandler* instance();
        void Init(wstring  reportPath);
        void setReportCrashesToSystem(bool report);
        bool writeMinidump();

    private:
        CrashHandler();
        ~CrashHandler();
        Q_DISABLE_COPY(CrashHandler)
        CrashHandlerPrivate* d;
    };
}

#endif // CRASH_HANDLER_H
