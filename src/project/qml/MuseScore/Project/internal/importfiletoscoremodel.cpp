/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "importfiletoscoremodel.h"

#include <QFileInfo>
#include <QUrl>

#include "io/path.h"

using namespace mu::project;
using namespace muse;

static QString localPath(const QString& pathOrUrl)
{
    QUrl url(pathOrUrl);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }

    return pathOrUrl;
}

ImportFileToScoreModel::ImportFileToScoreModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

QString ImportFileToScoreModel::errorMessage() const
{
    return m_errorMessage;
}

void ImportFileToScoreModel::setErrorMessage(const QString& message)
{
    if (m_errorMessage == message) {
        return;
    }

    m_errorMessage = message;
    emit errorMessageChanged();
}

QStringList ImportFileToScoreModel::selectFiles()
{
    const std::vector<std::string> filter {
        qtrc("project", "Importable files").toStdString() + " (*.pdf *.mp3 *.jpg *.jpeg *.png)"
    };

    io::paths_t files = interactive()->selectOpeningFilesSync(qtrc("project", "Choose file").toStdString(), io::path_t(), filter);
    if (files.empty()) {
        return QStringList();
    }

    QStringList paths;
    paths.reserve(files.size());
    for (const io::path_t& file : files) {
        paths << file.toQString();
    }

    return checkFiles(paths) ? paths : QStringList();
}

bool ImportFileToScoreModel::checkFiles(const QStringList& pathsOrUrls)
{
    if (pathsOrUrls.isEmpty()) {
        return false;
    }

    bool hasMp3 = false;
    bool hasPdf = false;
    bool hasImage = false;
    bool hasUnsupported = false;

    for (const QString& pathOrUrl : pathsOrUrls) {
        const QString ext = QFileInfo(localPath(pathOrUrl)).suffix().toLower();
        if (!hasMp3 && ext == "mp3") {
            hasMp3 = true;
        } else if (!hasPdf && ext == "pdf") {
            hasPdf = true;
        } else if (!hasImage && (ext == "jpg" || ext == "jpeg" || ext == "png")) {
            hasImage = true;
        } else if (ext != "mp3" && ext != "pdf" && ext != "jpg" && ext != "jpeg" && ext != "png") {
            hasUnsupported = true;
            break;
        }
    }

    const int typeGroupCount = int(hasMp3) + int(hasPdf) + int(hasImage);
    if (hasUnsupported || typeGroupCount != 1) {
        setErrorMessage(qtrc("project", "Unsupported file selection. Please choose one PDF, one or more JPG/PNG images, or one MP3 file."));
        return false;
    }

    if ((hasMp3 || hasPdf) && pathsOrUrls.size() > 1) {
        setErrorMessage(qtrc("project", "Only one file can be imported at a time for this file type."));
        return false;
    }

    setErrorMessage(QString());
    return true;
}

QStringList ImportFileToScoreModel::localPaths(const QStringList& pathsOrUrls) const
{
    QStringList paths;
    paths.reserve(pathsOrUrls.size());
    for (const QString& pathOrUrl : pathsOrUrls) {
        paths << localPath(pathOrUrl);
    }

    return paths;
}
