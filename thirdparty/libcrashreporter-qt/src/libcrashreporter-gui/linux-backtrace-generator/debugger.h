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
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QtCore/QString>

class CrashedApplication;

class Debugger
{
public:
    static QList<Debugger> availableInternalDebuggers(const QString & backend);
    static QList<Debugger> availableExternalDebuggers(const QString & backend);

    /** Returns true if this Debugger instance is valid, or false otherwise.
     * Debugger instances are valid only if they have been constructed from
     * availableInternalDebuggers() or availableExternalDebuggers(). If they
     * have been constructed directly using the Debugger constructor, they are invalid.
     */
    bool isValid() const;

    /** Returns true if this debugger is installed. This is determined by
     * looking for the executable that tryExec() returns. If it is in $PATH,
     * this method returns true.
     */
    bool isInstalled() const;

    /** Returns the translatable name of the debugger (eg. "GDB") */
    QString name() const;

    /** Returns the code name of the debugger (eg. "gdb"). */
    QString codeName() const;

    /** Returns the executable name that drkonqi should check if it exists
     * to determine whether the debugger is installed
     */
    QString tryExec() const;

    /** Returns a list with the drkonqi backends that this debugger supports */
    QStringList supportedBackends() const;

    /** Sets the backend to be used. This function must be called before using
     * command(), backtraceBatchCommands() or runInTerminal().
     */
    void setUsedBackend(const QString & backendName);

    /** Returns the command that should be run to use the debugger */
    QString command() const;

    /** Returns the commands that should be given to the debugger when
     * run in batch mode in order to generate a backtrace
     */
    QString backtraceBatchCommands() const;

    /** If this is an external debugger, it returns whether it should be run in a terminal or not */
    bool runInTerminal() const;


    enum ExpandStringUsage {
        ExpansionUsagePlainText,
        ExpansionUsageShell
    };

    static void expandString(QString & str,
                             const CrashedApplication* appInfo,
                             ExpandStringUsage usage = ExpansionUsagePlainText,
                             const QString & tempFile = QString());

private:
    static QList<Debugger> availableDebuggers(const QString &path, const QString & backend);
    QString m_backend;
};

#endif
