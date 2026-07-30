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

QString ImportFileToScoreModel::guidelinesLinkText() const
{
    return "<a href=\"" + configuration()->scoreUploadingGuidelinesUrl().toString() + "\">"
           + "Uploading guidelines" + "</a>";
}

QStringList ImportFileToScoreModel::selectFiles()
{
    const std::vector<std::string> filters {
        "Importable files (*.pdf *.jpg *.jpeg *.png *.mp3)",
        "All files (*)"
    };

    io::paths_t files = interactive()->selectOpeningFilesSync("Choose file",
                                                              configuration()->defaultOpenProjectsPath(), filters);

    QStringList paths;
    paths.reserve(files.size());
    for (const io::path_t& file : files) {
        paths << file.toQString();
    }

    return paths;
}

void ImportFileToScoreModel::validateFiles(const QStringList& pathsOrUrls)
{
    io::paths_t ioPaths;
    ioPaths.reserve(pathsOrUrls.size());

    QVariantList normalizedPaths;
    normalizedPaths.reserve(pathsOrUrls.size());

    for (const QString& pathOrUrl : pathsOrUrls) {
        QString path = localPath(pathOrUrl);
        ioPaths.push_back(io::path_t(path));
        normalizedPaths << path;
    }

    importFileToScoreScenario()->validateFiles(ioPaths).onResolve(this, [this, normalizedPaths](const RetVal<cloud::ImportType>& result) {
        if (result.ret) {
            emit validationFinished(int(result.val), normalizedPaths);
        }
    });
}
