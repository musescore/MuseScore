#define __STDC_FORMAT_MACROS
#include "crashhandler.h"

#include <time.h>

#include "log.h"

#ifdef _WIN32

#include <client/windows/handler/exception_handler.h>
#ifndef _UNICODE
#define _UNICODE
#endif
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

#elif defined __linux__

#include <client/linux/handler/exception_handler.h>
#include <client/linux/handler/minidump_descriptor.h>
#include <string.h>
#include <libgen.h>

#elif defined __APPLE__

#include <client/mac/handler/exception_handler.h>
#include <string.h>

#endif

#ifdef _WIN32
// === Windows ===
static bool minidumpCallback(const wchar_t* dump_path,
                             const wchar_t* minidump_id,
                             void* context,
                             EXCEPTION_POINTERS* exinfo,
                             MDRawAssertionInfo* assertion,
                             bool succeeded)
{
    UNUSED(context);
    UNUSED(exinfo);
    UNUSED(assertion);

    // Rename ===
    wchar_t oldpath[_MAX_PATH] = { 0 };
    wcscat(oldpath, dump_path);
    wcscat(oldpath, L"/");
    wcscat(oldpath, minidump_id);
    wcscat(oldpath, L".dmp");

    wchar_t newpath[_MAX_PATH] = { 0 };
    wcscat(newpath, dump_path);
    wcscat(newpath, L"/MuseScore-");
    time_t t = time(NULL);
    struct tm* now = localtime(&t);
    wchar_t buf[96] = { 0 };
    wcsftime(buf, sizeof buf, L"%g%m%d_%H%M%S", now);
    wcscat(newpath, buf);
    //    wcscat(newpath, "_");
    //    wcscat(newpath, VERCORE);
    wcscat(newpath, L".dmp");

    _wrename(oldpath, newpath);
    // ===

    LOGE() << "Oops! Application crashed. Crash dump written in: " << std::wstring(newpath);

    return succeeded;
}

#elif defined __linux__
// === Linux ===
static bool minidumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    UNUSED(context);

    // Rename
    const char* oldpath = descriptor.path();
    char newpath[256] = { 0 };
    strcat(newpath, descriptor.directory().c_str());
    strcat(newpath, "/MuseScore-");
    time_t t = time(NULL);
    struct tm* now = localtime(&t);
    char buf[96] = { 0 };
    strftime(buf, sizeof buf, "%g%m%d_%H%M%S", now);
    strcat(newpath, buf);
    //    strcat(newpath, "_");
    //    strcat(newpath, VERCORE);
    strcat(newpath, ".dmp");

    rename(oldpath, newpath);

    LOGE() << "Oops! Application crashed. Crash dump written in: " << newpath;

    return succeeded;
}

#elif defined __APPLE__
// === MacOS ===
static bool minidumpCallback(const char* dump_dir,
                             const char* minidump_id,
                             void* context,
                             bool succeeded)
{
    UNUSED(context);

    // Rename ===
    char oldpath[256] = { 0 };
    strcat(oldpath, dump_dir);
    strcat(oldpath, "/");
    strcat(oldpath, minidump_id);
    strcat(oldpath, ".dmp");

    char newpath[256] = { 0 };
    strcat(newpath, dump_dir);
    strcat(newpath, "/MuseScore-");
    time_t t = time(NULL);
    struct tm* now = localtime(&t);
    char buf[256] = { 0 };
    strftime(buf, sizeof buf, "%g%m%d_%H%M%S", now);
    strcat(newpath, buf);
    //    strcat(newpath, "_");
    //    strcat(newpath, VERCORE);
    strcat(newpath, ".dmp");

    rename(oldpath, newpath);

    LOGE() << "Oops! Application crashed. Crash dump written in: " << newpath;

    return succeeded;
}

#endif

using namespace mu::devtools;

CrashHandler::CrashHandler(const std::string& dumpDirPath)
    : m_dumpDirPath(dumpDirPath)
{
#ifdef _WIN32

    std::wstring path(dumpDirPath.begin(), dumpDirPath.end());
    m_handler = new google_breakpad::ExceptionHandler(
        path,
        nullptr,                            // FilterCallback
        minidumpCallback,                   // MinidumpCallback
        this,                               // callback_context
        google_breakpad::ExceptionHandler::HANDLER_ALL);

#elif defined __linux__

    m_handler = new google_breakpad::ExceptionHandler(
        google_breakpad::MinidumpDescriptor(dumpDirPath),
        nullptr,                            // FilterCallback
        minidumpCallback,                   // MinidumpCallback
        this,                               // callback_context
        true,                               // write mididump immediately
        -1);

#elif defined __APPLE__

    m_handler = new google_breakpad::ExceptionHandler(
        dumpDirPath,
        nullptr,
        minidumpCallback,
        this,
        true,
        nullptr);

#endif
}
