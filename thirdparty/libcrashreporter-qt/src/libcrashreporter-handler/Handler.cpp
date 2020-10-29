/*
 * libcrashreporter-qt
 *
 * Copyright (C) 2010-2011  Christian Muehlhaeuser <muesli@tomahawk-player.org>
 * Copyright (C) 2014       Dominik Schmidt <dev@dominik-schmidt.de>
 * Copyright (C) 2016       Teo Mrnjavac <teo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "Handler.h"

#include <string>
#include <iostream>

#include <QCoreApplication>
#include <QFileInfo>
#include <QString>

#include <QDebug>

#ifdef __APPLE__
#   include <client/mac/handler/exception_handler.h>
#elif defined _WIN32
#   include <client/windows/handler/exception_handler.h>
#elif defined __linux__
#   include <client/linux/handler/exception_handler.h>
#   include <client/linux/handler/minidump_descriptor.h>
#   include <cstdio>
#endif

namespace CrashReporter
{

bool s_active = true;


void
Handler::setActive( bool enabled )
{
    s_active = enabled;
}


bool
Handler::isActive()
{
    return s_active;
}

#ifdef Q_OS_WIN
static bool
LaunchUploader( const wchar_t* dump_dir, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion, bool succeeded )
{
    if ( !succeeded )
        return false;

    // DON'T USE THE HEAP!!!
    // So that indeed means, no QStrings, no qDebug(), no QAnything, seriously!


    const wchar_t* crashReporter = static_cast<Handler*>(context)->crashReporterWChar();
    if ( !s_active || wcslen( crashReporter ) == 0 )
        return false;

    wchar_t command[MAX_PATH * 3 + 6];
    wcscpy( command, crashReporter);
    wcscat( command, L" \"");
    wcscat( command, dump_dir );
    wcscat( command, L"/" );
    wcscat( command, minidump_id );
    wcscat( command, L".dmp\"" );



    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    ZeroMemory( &pi, sizeof(pi) );

    if ( CreateProcess( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
    {
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        TerminateProcess( GetCurrentProcess(), 1 );
    }

    return succeeded;
}


#else


#include <unistd.h>

#ifdef Q_OS_LINUX
static bool
GetCrashInfo( const void* crash_context, size_t crash_context_size, void* context )
{
    // Don't use the heap here.

    // The callback signature passes the crash context as an opaque object, but from
    // what I gather from the comments in exception_handler.h, this cast should always
    // be safe.
    // We need it for the signal number and thread ID.      -- Teo 3/2016
    const google_breakpad::ExceptionHandler::CrashContext* cxt =
        static_cast< const google_breakpad::ExceptionHandler::CrashContext* >( crash_context );

    static_cast< Handler* >( context )->m_signalNumber = cxt->siginfo.si_signo;
    static_cast< Handler* >( context )->m_threadId = cxt->tid;

    // We always return false so Breakpad will continue with generating the minidump the
    // usual way. Had we returned true here, to Breakpad it would mean "I'm done with
    // making the minidump", which is of course not the case here.
    return false;
}
#endif


static bool
#ifdef Q_OS_LINUX
LaunchUploader( const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded )
{
#else // Q_OS_MAC
LaunchUploader( const char* dump_dir, const char* minidump_id, void* context, bool succeeded)
{
#endif

    if ( !succeeded )
    {
        printf("Could not write crash dump file");
        return false;
    }

    // DON'T USE THE HEAP!!!
    // So that indeed means, no QStrings, no qDebug(), no QAnything, seriously!

#ifdef Q_OS_LINUX
    const char* path = descriptor.path();
#else // Q_OS_MAC
    const char* extension = "dmp";

    char path[4096];
    strcpy(path, dump_dir);
    strcat(path, "/");
    strcat(path, minidump_id);
    strcat(path, ".");
    strcat(path,  extension);
#endif

    printf("Dump file was written to: %s\n", path);

    const char* crashReporter = static_cast<Handler*>(context)->crashReporterChar();
    if ( !s_active || strlen( crashReporter ) == 0 )
        return false;

#ifdef Q_OS_LINUX
    const char* applicationName = static_cast<Handler*>(context)->applicationName();
    if ( strlen( applicationName ) == 0 )
        return false;
    const char* executablePath = static_cast<Handler*>(context)->executablePath();
    if ( strlen( executablePath ) == 0 )
        return false;
    const char* applicationVersion = static_cast<Handler*>(context)->applicationVersion();
    if ( strlen( applicationVersion ) == 0 )
        return false;

    char procid[17];
    sprintf( procid, "%d", static_cast<Handler*>(context)->pid() );
    char signum[17];
    sprintf( signum, "%d", static_cast<Handler*>(context)->signalNumber() );
    char tid[17];
    sprintf( tid, "%d", static_cast<Handler*>(context)->threadId() );
#endif

    pid_t pid = fork();
    if ( pid == -1 ) // fork failed
        return false;
    if ( pid == 0 )
    {
        // we are the fork
#ifdef Q_OS_LINUX
        execl( crashReporter,
               crashReporter,
               path,
               procid,
               signum,
               applicationName,
               executablePath,
               applicationVersion,
               tid,
               (char*) 0 );
#else
        execl( crashReporter,
               crashReporter,
               path,
               (char*) 0 );
#endif

        // execl replaces this process, so no more code will be executed
        // unless it failed. If it failed, then we should return false.
        printf( "Error: Can't launch CrashReporter!\n" );
        return false;
    }
#if defined(Q_OS_LINUX) && defined(ENABLE_CRASH_REPORTER)
    // If we're running on Linux, we expect that the CrashReporter component will
    // attach gdb, do its thing and then kill this process, so we hang here for the
    // time being, on purpose.          -- Teo 3/2016
    pause();
#endif

    // we called fork()
    return true;
}

#endif


Handler::Handler( const QString& dumpFolderPath, bool active, const QString& crashReporter  )
{
    s_active = active;

    #if defined Q_OS_LINUX
    m_crash_handler =  new google_breakpad::ExceptionHandler(
                           google_breakpad::MinidumpDescriptor( dumpFolderPath.toStdString() ),
                           NULL,
                           LaunchUploader,
                           this,
                           true,
                           -1 );
    m_crash_handler->set_crash_handler(GetCrashInfo);
    #elif defined Q_OS_MAC
    m_crash_handler =  new google_breakpad::ExceptionHandler( dumpFolderPath.toStdString(), NULL, LaunchUploader, this, true, NULL);
    #elif defined Q_OS_WIN
//     m_crash_handler = new google_breakpad::ExceptionHandler( dumpFolderPath.toStdString(), 0, LaunchUploader, this, true, 0 );
    m_crash_handler = new google_breakpad::ExceptionHandler( dumpFolderPath.toStdWString(), 0, LaunchUploader, this, google_breakpad::ExceptionHandler::HANDLER_ALL );
    #endif

    setCrashReporter( crashReporter );
#ifdef Q_OS_LINUX
    setApplicationData( qApp );
#endif
}


void
Handler::setCrashReporter( const QString& crashReporter )
{
    QString crashReporterPath;
    QString localReporter = QString( "%1/%2" ).arg( QCoreApplication::instance()->applicationDirPath() ).arg( crashReporter );
    QString globalReporter = QString( "%1/../libexec/%2" ).arg( QCoreApplication::instance()->applicationDirPath() ).arg( crashReporter );

    if ( QFileInfo( localReporter ).exists() )
        crashReporterPath = localReporter;
    else if ( QFileInfo( globalReporter ).exists() )
        crashReporterPath = globalReporter;
    else {
        qDebug() << "Could not find \"" << crashReporter << "\" in ../libexec or application path";
        crashReporterPath = crashReporter;
    }


    // cache reporter path as char*
    char* creporter;
    std::string sreporter = crashReporterPath.toStdString();
    creporter = new char[ sreporter.size() + 1 ];
    strcpy( creporter, sreporter.c_str() );
    m_crashReporterChar = creporter;

    qDebug() << "m_crashReporterChar: " << m_crashReporterChar;

    // cache reporter path as wchart_t*
    wchar_t* wreporter;
    std::wstring wsreporter = crashReporterPath.toStdWString();
    wreporter = new wchar_t[ wsreporter.size() + 10 ];
    wcscpy( wreporter, wsreporter.c_str() );
    m_crashReporterWChar = wreporter;
    std::wcout << "m_crashReporterWChar: " << m_crashReporterWChar;
}


#ifdef Q_OS_LINUX
void
Handler::setApplicationData( const QCoreApplication* app )
{
    m_pid = app->applicationPid();

    char* cappname;
    std::string sappname = app->applicationName().toStdString();
    cappname = new char[ sappname.size() + 1 ];
    strcpy( cappname, sappname.c_str() );
    m_applicationName = cappname;

    char* cepath;
    std::string sepath = app->applicationFilePath().toStdString();
    cepath = new char[ sepath.size() + 1 ];
    strcpy( cepath, sepath.c_str() );
    m_executablePath = cepath;

    char* cappver;
    std::string sappver = app->applicationVersion().toStdString();
    cappver = new char[ sappver.size() + 1 ];
    strcpy( cappver, sappver.c_str() );
    m_applicationVersion = cappver;

    // To be set by the handler
    m_signalNumber = -1;
    m_threadId = -1;
}
#endif


Handler::~Handler()
{
    delete m_crash_handler;
}

} // end namespace
