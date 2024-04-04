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

#ifndef MUSE_EXTENSIONS_APIV1_UTIL_H
#define MUSE_EXTENSIONS_APIV1_UTIL_H

#include <QDir>
#include <QProcess>

namespace muse::extensions::apiv1 {
//---------------------------------------------------------
///   \class FileIO
///   Provides a simple API to perform file reading and
///   writing operations. To use this class in a plugin put
///   \code import FileIO 3.0 \endcode
///   to the top part of the .qml plugin file.
///   After that FileIO object can be declared and used
///   in QML code:
///   \code
///   import MuseScore 3.0
///   import FileIO 3.0
///   MuseScore {
///       FileIO {
///           id: exampleFile
///           source: "/tmp/example.txt"
///       }
///       onRun: {
///           var test = exampleFile.read();
///           console.log(test); // will print the file content
///       }
///   }
///   \endcode
//---------------------------------------------------------

class FileIO : public QObject
{
    Q_OBJECT

public:
    /** Path to the file which is operated on */
    Q_PROPERTY(QString source
               READ source
               WRITE setSource
               )
    /// \cond PLUGIN_API \private \endcond
    explicit FileIO(QObject* parent = 0);

    /**
     * Reads file contents and returns a string.
     * In case error occurs, error() signal is emitted
     * and an empty string is returned.
     */
    Q_INVOKABLE QString read();
    /** Returns true if the file exists */
    Q_INVOKABLE bool exists();
    /**
     * Writes a string to the file.
     * \warning This function overwrites all the contents of
     * the file pointed by FileIO::source so it becomes lost.
     * \returns `true` if an operation finished successfully.
     */
    Q_INVOKABLE bool write(const QString& data);
    /** Removes the file */
    Q_INVOKABLE bool remove();
    /** Returns user's home directory */
    Q_INVOKABLE QString homePath() { QDir dir; return dir.homePath(); }
    /** Returns a path suitable for a temporary file */
    Q_INVOKABLE QString tempPath() { QDir dir; return dir.tempPath(); }
    /** Returns the file's last modification time */
    Q_INVOKABLE int modifiedTime();

    /// \cond MS_INTERNAL
    QString source() { return mSource; }

public slots:
    void setSource(const QString& source) { mSource = source; }
    /// \endcond

signals:
    /**
     * Emitted on file operations errors.
     * Implement onError() in your FileIO object to handle this signal.
     * \param msg A short textual description of the error occurred.
     */
    void error(const QString& msg);

private:
    QString mSource;
};

//---------------------------------------------------------
//   MsProcess
//   @@ QProcess
///   \brief Start an external program.\ Available in QML
///   as \p QProcess.
///   \details Using this will most probably result in the
///   plugin to be platform dependant. \since MuseScore 3.2
//---------------------------------------------------------

class MsProcess : public QProcess
{
    Q_OBJECT

public:
    MsProcess(QObject* parent = 0)
        : QProcess(parent) {}

public slots:
    /**
     * Execute an external command.
     * \param command A command line to execute.
     * \warning This function is deprecated. Use \ref startWithArgs instead.
     */
    Q_INVOKABLE void start(const QString& command);
    /**
     * Execute an external command.
     * \param program A program to execute.
     * \param args An array of arguments passed to the program.
     * \since MuseScore 4.3
     */
    Q_INVOKABLE void startWithArgs(const QString& program, const QStringList& args);
    //@ --
    Q_INVOKABLE bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
    //@ --
    Q_INVOKABLE QByteArray readAllStandardOutput() { return QProcess::readAllStandardOutput(); }
};
}

#endif
