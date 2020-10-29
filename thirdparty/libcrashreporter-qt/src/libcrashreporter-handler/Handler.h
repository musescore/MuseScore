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

#include <QtGlobal>

class QString;
class QCoreApplication;

namespace google_breakpad
{
    class ExceptionHandler;
}

namespace CrashReporter
{

#ifdef Q_OS_LINUX
static bool GetCrashInfo( const void* crash_context, size_t crash_context_size, void* context );
#endif

class Handler
{
    const char* m_crashReporterChar; // yes! It MUST be const char[]
    const wchar_t* m_crashReporterWChar;
#ifdef Q_OS_LINUX
    int m_pid;
    int m_signalNumber;
    const char* m_applicationName;
    const char* m_executablePath;
    const char* m_applicationVersion;
    int m_threadId;

    friend bool CrashReporter::GetCrashInfo( const void* crash_context, size_t crash_context_size, void* context );
#endif

public:
    Handler(const QString& dumpFolderPath, bool active, const QString& crashReporter );
    virtual ~Handler();

    static void setActive( bool enabled );
    static bool isActive();

    void setCrashReporter( const QString& crashReporter );
    const char* crashReporterChar() const { return m_crashReporterChar; }
    const wchar_t* crashReporterWChar() const { return m_crashReporterWChar; }

#ifdef Q_OS_LINUX
    void setApplicationData( const QCoreApplication* app );
    int pid() const { return m_pid; }
    int signalNumber() const { return m_signalNumber; }
    const char* applicationName() const { return m_applicationName; }
    const char* executablePath() const { return m_executablePath; }
    const char* applicationVersion() const { return m_applicationVersion; }
    int threadId() const { return m_threadId; }
#endif

private:
    google_breakpad::ExceptionHandler* m_crash_handler;
};

}
#undef char
