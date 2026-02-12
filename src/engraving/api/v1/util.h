/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QDir>
#include <QProcess>

namespace mu::engraving::apiv1 {
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
    /// Path to the file which is operated on
    Q_PROPERTY(QString source
               READ source
               WRITE setSource
               )
    /// \cond PLUGIN_API \private \endcond
    explicit FileIO(QObject* parent = 0);

    /// Reads file contents and returns a string.
    /// In case error occurs, error() signal is emitted
    /// and an empty string is returned.
    Q_INVOKABLE QString read();
    /// muse::Returns true if the file exists
    Q_INVOKABLE bool exists();
    /// Writes a string to the file.
    /// \warning This function overwrites all the contents of
    /// the file pointed by FileIO::source so it becomes lost.
    /// \note For security reasons, files can only be written within
    /// the user's MuseScore data directories (userAppDataPath and userDataPath).
    /// Attempts to write outside these directories will be blocked.
    /// \returns `true` if an operation finished successfully.
    Q_INVOKABLE bool write(const QString& data);
    /**
     * Writes binary data to the file.
     * Each character code (0-255) in the string is written as a single byte.
     * Use this for binary files like images, soundfonts, etc.
     * \warning This function overwrites all the contents of
     * the file pointed by FileIO::source so it becomes lost.
     * \note For security reasons, files can only be written within
     * the user's MuseScore data directories (userAppDataPath and userDataPath).
     * Attempts to write outside these directories will be blocked.
     * \returns `true` if an operation finished successfully.
     */
    Q_INVOKABLE bool writeBinary(const QString& data);
    /// Removes the file
    Q_INVOKABLE bool remove();
    /// muse::Returns user's home directory
    Q_INVOKABLE QString homePath() { QDir dir; return dir.homePath(); }
    /// muse::Returns a path suitable for a temporary file
    Q_INVOKABLE QString tempPath() { QDir dir; return dir.tempPath(); }
    // Returns the user's MuseScore documents directory (default location for Scores, Plugins, SoundFonts, Styles, Templates)
    Q_INVOKABLE QString userDataPath();
    // Returns the user-configured Plugins directory (Preferences → Folders → Plugins)
    Q_INVOKABLE QString pluginsUserPath();
    // Returns the user-configured Scores directory (Preferences → Folders → Scores)
    Q_INVOKABLE QString userProjectsPath();
    // Returns the user-configured Templates directory (Preferences → Folders → Templates)
    Q_INVOKABLE QString userTemplatesPath();
    // Returns the user-configured Styles directory (Preferences → Folders → Styles)
    Q_INVOKABLE QString userStylesPath();
    // Returns the user-configured SoundFonts directories (Preferences → Folders → SoundFonts)
    Q_INVOKABLE QStringList userSoundFontDirectories();
    // Returns the plugin's folder's path
    Q_INVOKABLE QString pluginDirectoryPath();
    // Returns the project's path (ex: .../Desktop/project.mscz)
    Q_INVOKABLE QString projectPath();
    // Returns whether or not the project is stored as a folder (with a .mscx file)
    Q_INVOKABLE bool isProjectDirectory();
    // Returns the project's folder's path
    Q_INVOKABLE QString projectDirectoryPath();
    /// Returns true if the plugin is allowed to write to the path
    Q_INVOKABLE bool isPathWriteable(const QString& path);

    /// muse::Returns the file's last modification time
    Q_INVOKABLE int modifiedTime();

    /// \cond MS_INTERNAL
    QString source() { return m_source; }

public slots:
    void setSource(const QString& source) { m_source = source; }
    /// \endcond

signals:
    /// Emitted on file operations errors.
    /// Implement onError() in your FileIO object to handle this signal.
    /// \param msg A short textual description of the error occurred.
    void error(const QString& msg);

private:
    QString m_source;
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
    /// Execute an external command.
    /// \param command A command line to execute.
    /// \warning This function is deprecated. Use \ref startWithArgs instead.
    Q_INVOKABLE void start(const QString& command);
    /// Execute an external command.
    /// \param program A program to execute.
    /// \param args An array of arguments passed to the program.
    /// \since MuseScore 4.3
    Q_INVOKABLE void startWithArgs(const QString& program, const QStringList& args);
    /// --
    Q_INVOKABLE bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
    /// --
    Q_INVOKABLE QByteArray readAllStandardOutput() { return QProcess::readAllStandardOutput(); }
};
}
