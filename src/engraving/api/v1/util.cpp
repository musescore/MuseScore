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

#include <QDateTime>
#include <QFileInfo>
#include <QQmlEngine>
#include <QQmlContext>

#include "util.h"

#include "log.h"
#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "extensions/iextensionsconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "audio/main/iaudioconfiguration.h"

using namespace muse;

namespace mu::engraving::apiv1 {

// User's MuseScore documents directory (default location for scores, plugins, etc.)
static QString getUserDataPath() 
{
    // Get global configuration to access allowed paths
    auto globalConfig = muse::modularity::globalIoc()->resolve<IGlobalConfiguration>("extensions");
    if (!globalConfig) {
        LOGE() << "Failed to resolve IGlobalConfiguration";
        return QString();
    }

    return QString::fromStdString(globalConfig->userDataPath().toStdString());
}

// User-configured Plugins directory (Preferences → Folders → Plugins)
static QString getPluginsUserPath()
{
    auto extensionsConfig = muse::modularity::globalIoc()->resolve<extensions::IExtensionsConfiguration>("extensions");
    if (!extensionsConfig) {
        LOGE() << "Failed to resolve IExtensionsConfiguration";
        return QString();
    }

    return QString::fromStdString(extensionsConfig->pluginsUserPath().toStdString());
}

// User-configured Scores directory (Preferences → Folders → Scores)
static QString getUserProjectsPath()
{
    auto projectConfig = muse::modularity::globalIoc()->resolve<mu::project::IProjectConfiguration>("project");
    if (!projectConfig) {
        LOGE() << "Failed to resolve IProjectConfiguration";
        return QString();
    }

    return QString::fromStdString(projectConfig->userProjectsPath().toStdString());
}

// User-configured Templates directory (Preferences → Folders → Templates)
static QString getUserTemplatesPath()
{
    auto projectConfig = muse::modularity::globalIoc()->resolve<mu::project::IProjectConfiguration>("project");
    if (!projectConfig) {
        LOGE() << "Failed to resolve IProjectConfiguration";
        return QString();
    }

    return QString::fromStdString(projectConfig->userTemplatesPath().toStdString());
}

// User-configured Styles directory (Preferences → Folders → Styles)
static QString getUserStylesPath()
{
    auto notationConfig = muse::modularity::globalIoc()->resolve<mu::notation::INotationConfiguration>("notation");
    if (!notationConfig) {
        LOGE() << "Failed to resolve INotationConfiguration";
        return QString();
    }

    return QString::fromStdString(notationConfig->userStylesPath().toStdString());
}

// User-configured SoundFonts directories (Preferences → Folders → SoundFonts)
static QStringList getUserSoundFontDirectories()
{
    QStringList paths;
    auto audioConfig = muse::modularity::globalIoc()->resolve<audio::IAudioConfiguration>("audio");
    
    if (!audioConfig) {
        LOGE() << "Failed to resolve IAudioConfiguration";
        return paths; // empty list
    }

    for (const auto& path : audioConfig->userSoundFontDirectories()) {
        if (!path.empty()) {
            paths << QString::fromStdString(path.toStdString());
        }
    }

    return paths;
}

//---------------------------------------------------------
//   isPathAllowed
//   Check if the file path is within allowed directories
//---------------------------------------------------------
static bool isPathAllowed(const QString& filePath)
{
    // Get the canonical (absolute, symlinks resolved) path
    QFileInfo fileInfo(filePath);
    QString canonicalPath = fileInfo.canonicalFilePath();

    // If file doesn't exist yet, we need to clean the path manually
    // because absoluteFilePath() does NOT resolve .. and . elements
    if (canonicalPath.isEmpty()) {
        // Use QDir::cleanPath to resolve .. and . elements
        canonicalPath = QDir::cleanPath(fileInfo.absoluteFilePath());
    }

    // Build list of allowed base paths from all user-configurable directories
    QStringList allowedPaths;

    // Note: userAppDataPath() is NOT included because it contains sensitive data:
    // - User credentials (musescorecom_cred.dat)
    // - System configuration (shortcuts.xml, midi_mappings.xml)
    // - Application logs
    // Plugins should not write to this directory

    // 1. System temp directory (for temporary files)
    QDir tempDir;
    allowedPaths << tempDir.tempPath();

    // 2. User's MuseScore documents directory (default location for Scores, Plugins, SoundFonts, Styles, Templates)
    if (QString path = getUserDataPath(); !path.isEmpty()) {
        allowedPaths << path;
    }

    // 3. User-configured Plugins directory (Preferences → Folders → Plugins)
    if (QString path = getPluginsUserPath(); !path.isEmpty()) {
        allowedPaths << path;
    }

    // 4. User-configured Scores directory (Preferences → Folders → Scores)
    if (QString path = getUserProjectsPath(); !path.isEmpty()) {
        allowedPaths << path;
    }

    // 5. User-configured Templates directory (Preferences → Folders → Templates)
    if (QString path = getUserTemplatesPath(); !path.isEmpty()) {
        allowedPaths << path;
    }

    // 6. User-configured Styles directory (Preferences → Folders → Styles)
    if (QString path = getUserStylesPath(); !path.isEmpty()) {
        allowedPaths << path;
    }

    // 7. User-configured SoundFonts directories (Preferences → Folders → SoundFonts)
    allowedPaths << getUserSoundFontDirectories();

    // Check if the canonical path starts with any allowed base path
    bool allowed = false;
    for (const QString& basePath : allowedPaths) {
        if (canonicalPath.startsWith(basePath)) {
            allowed = true;
            break;
        }
    }

    if (!allowed) {
        LOGW() << "File write blocked: path '" << canonicalPath << "' is outside allowed directories";
        LOGW() << "Allowed directories: " << allowedPaths.join(", ");
    }

    return allowed;
}

//---------------------------------------------------------
//   FileIO
//---------------------------------------------------------

FileIO::FileIO(QObject* parent)
    : QObject(parent)
{
}

// Allow plugins to get valid write directories
QString FileIO::userDataPath() {
    return getUserDataPath();
}

QString FileIO::pluginsUserPath() {
    return getPluginsUserPath();
}

QString FileIO::userProjectsPath() {
    return getUserProjectsPath();
}

QString FileIO::userTemplatesPath() {
    return getUserTemplatesPath();
}

QString FileIO::userStylesPath() {
    return getUserStylesPath();
}

QStringList FileIO::userSoundFontDirectories() {
    return getUserSoundFontDirectories();
}

// The running plugin's directory
QString FileIO::pluginDirectoryPath() {
    QQmlContext* context = QQmlEngine::contextForObject(this);

    if (!context) {
        return QString();
    }

    QUrl url = context->baseUrl();

    if (!url.isLocalFile()) {
        return QString();
    }

    return QFileInfo(url.toLocalFile()).absolutePath();
}

QString FileIO::read()
{
    if (m_source.isEmpty()) {
        emit error("source is empty");
        return QString();
    }
    QUrl url(m_source);
    QString source(m_source);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFile file(source);
    QString fileContent;
    if (file.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream t(&file);
        do {
            line = t.readLine();
            fileContent += line + "\n";
        } while (!line.isNull());
        file.close();
    } else {
        emit error("Unable to open the file");
        return QString();
    }
    return fileContent;
}

bool FileIO::write(const QString& data)
{
    if (m_source.isEmpty()) {
        return false;
    }

    QUrl url(m_source);

    QString source = (url.isValid() && url.isLocalFile()) ? url.toLocalFile() : m_source;

    // Security: Check if path is within allowed directories
    if (!isPathAllowed(source)) {
        emit error("File write blocked: path is outside allowed directories");
        return false;
    }

    QFile file(source);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << data;
    file.close();
    return true;
}

bool FileIO::writeBinary(const QString& data)
{
    if (m_source.isEmpty()) {
        return false;
    }

    QUrl url(m_source);

    QString source = (url.isValid() && url.isLocalFile()) ? url.toLocalFile() : m_source;

    // Security: Check if path is within allowed directories
    if (!isPathAllowed(source)) {
        emit error("File write blocked: path is outside allowed directories");
        return false;
    }

    QFile file(source);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    // Write binary data without text encoding
    // Each character in the QString represents a byte (0-255)
    QByteArray bytes = data.toLatin1();
    qint64 written = file.write(bytes);
    file.close();

    return written == bytes.size();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

bool FileIO::remove()
{
    if (m_source.isEmpty()) {
        return false;
    }

    QFile file(m_source);
    return file.remove();
}

bool FileIO::exists()
{
    QFile file(m_source);
    return file.exists();
}

int FileIO::modifiedTime()
{
    if (m_source.isEmpty()) {
        emit error("source is empty");
        return 0;
    }
    QUrl url(m_source);
    QString source(m_source);
    if (url.isValid() && url.isLocalFile()) {
        source = url.toLocalFile();
    }
    QFileInfo fileInfo(source);
    return fileInfo.lastModified().toSecsSinceEpoch();
}

void MsProcess::start(const QString& command)
{
    QT_WARNING_PUSH;
    QT_WARNING_DISABLE_DEPRECATED;
    DEPRECATED_USE("startWithArgs(program, [arg1, arg2, ...])");
    QProcess::start(command);
    QT_WARNING_POP;
}

void MsProcess::startWithArgs(const QString& program, const QStringList& args)
{
    QProcess::start(program, args, ReadWrite);
}
}
