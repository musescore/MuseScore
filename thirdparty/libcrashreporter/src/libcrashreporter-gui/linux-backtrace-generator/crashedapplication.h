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
#ifndef CRASHEDAPPLICATION_H
#define CRASHEDAPPLICATION_H

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

class CrashedApplication : public QObject
{
    Q_OBJECT
public:
    CrashedApplication( int pid,
                        int signalNumber,
                        QString name,
                        QFileInfo executable,
                        QString fakeBaseName,
                        QString version,
                        int thread,
                        QDateTime dateTime,
                        QObject *parent = 0);

    ~CrashedApplication() override;

    /** Returns the crashed program's name, possibly translated (ex. "The KDE Crash Handler") */
    QString name() const;

    /** Returns a QFileInfo with information about the executable that crashed */
    QFileInfo executable() const;

    /** When an application is run via kdeinit, the executable() method returns kdeinit4, but
     * we still need a way to know which is the application that was loaded by kdeinit. So,
     * this method returns the base name of the executable that would have been launched if
     * the app had not been loaded by kdeinit (ex. "plasma-desktop"). If the application was
     * not launched via kdeinit, this method returns executable().baseName();
     */
    QString fakeExecutableBaseName() const;

    /** Returns the version of the crashed program */
    QString version() const;

    /** Returns the pid of the crashed program */
    int pid() const;

    /** Returns the signal number that the crashed program received */
    int signalNumber() const;

    /** Returns the name of the signal (ex. SIGSEGV) */
    QString signalName() const;

    int thread() const;

    const QDateTime& datetime() const;

protected:
    int m_pid;
    int m_signalNumber;
    QString m_name;
    QFileInfo m_executable;
    QString m_fakeBaseName;
    QString m_version;
    int m_thread;
    QDateTime m_datetime;
};

QString getSuggestedKCrashFilename(const CrashedApplication* app);

#endif // CRASHEDAPPLICATION_H
