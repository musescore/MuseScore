/*
    Copyright (C) 2009  George Kiagiadakis <gkiagia@users.sourceforge.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "debugger.h"

#include "crashedapplication.h"
#include "crqt-kmacroexpander.h"

#include <QStandardPaths>
#include <QDir>

//static
QList<Debugger> Debugger::availableInternalDebuggers(const QString & backend)
{
    return availableDebuggers(QStringLiteral("debuggers/internal"), backend);
}

//static
QList<Debugger> Debugger::availableExternalDebuggers(const QString & backend)
{
    return availableDebuggers(QStringLiteral("debuggers/external"), backend);
}

bool Debugger::isValid() const
{
    return true;
}

bool Debugger::isInstalled() const
{
    QString tryexec = tryExec();
    return !tryexec.isEmpty() && !QStandardPaths::findExecutable(tryexec).isEmpty();
}

QString Debugger::name() const
{
    return isValid() ? "gdb" : QString();
}

QString Debugger::codeName() const
{
    //fall back to the "TryExec" string if "CodeName" is not specified.
    //for most debuggers those strings should be the same
    return isValid() ? "gdb" : QString();
}

QString Debugger::tryExec() const
{
    return isValid() ? "gdb" : QString();
}

QStringList Debugger::supportedBackends() const
{
    return isValid() ? QStringList { "KCrash" } : QStringList();
}

void Debugger::setUsedBackend(const QString & backendName)
{
    if (supportedBackends().contains(backendName)) {
        m_backend = backendName;
    }
}

QString Debugger::command() const
{
    return "gdb -nw -n -batch -x %tempfile -p %pid %execpath";
}

QString Debugger::backtraceBatchCommands() const
{
    return "set width 200\nthread\nthread apply all bt";
}

bool Debugger::runInTerminal() const
{
    return false;
}

//static
void Debugger::expandString(QString & str,
                            const CrashedApplication* appInfo,
                            ExpandStringUsage usage,
                            const QString & tempFile)
{
    QHash<QString, QString> map;
    map[QLatin1String("progname")] = appInfo->name();
    map[QLatin1String("execname")] = appInfo->fakeExecutableBaseName();
    map[QLatin1String("execpath")] = appInfo->executable().absoluteFilePath();
    map[QLatin1String("signum")] = QString::number(appInfo->signalNumber());
    map[QLatin1String("signame")] = appInfo->signalName();
    map[QLatin1String("pid")] = QString::number(appInfo->pid());
    map[QLatin1String("tempfile")] = tempFile;
    map[QLatin1String("thread")] = QString::number(appInfo->thread());

    if (usage == ExpansionUsageShell) {
        str = KMacroExpander::expandMacrosShellQuote(str, map);
    } else {
        str = KMacroExpander::expandMacros(str, map);
    }
}

//static
QList<Debugger> Debugger::availableDebuggers(const QString & path, const QString & backend)
{
    QString debuggerDir = QStandardPaths::locate(QStandardPaths::DataLocation, path, QStandardPaths::LocateDirectory);
    QStringList debuggers = QDir(debuggerDir).entryList(QDir::Files);

    QList<Debugger> result;
    foreach (const QString & debuggerFile, debuggers) {
        Debugger debugger;
        if (debugger.supportedBackends().contains(backend)) {
            debugger.setUsedBackend(backend);
            result.append(debugger);
        }
    }
    return result;
}
