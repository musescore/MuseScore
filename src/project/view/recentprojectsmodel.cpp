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
#include "recentprojectsmodel.h"

#include "translation.h"
#include "actions/actiontypes.h"
#include "dataformatter.h"
#include "io/fileinfo.h"

#include "engraving/infrastructure/io/mscio.h"

#include "log.h"

using namespace mu::project;
using namespace mu::actions;

namespace {
const QString SCORE_NAME_KEY("name");
const QString SCORE_PATH_KEY("path");
const QString SCORE_SUFFIX_KEY("suffix");
const QString SCORE_THUMBNAIL_KEY("thumbnail");
const QString SCORE_TIME_SINCE_MODIFIED_KEY("timeSinceModified");
const QString SCORE_ADD_NEW_KEY("isAddNew");
}

RecentProjectsModel::RecentProjectsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    ProjectMetaList recentProjects = recentProjectsProvider()->recentProjectList();
    updateRecentScores(recentProjects);

    recentProjectsProvider()->recentProjectListChanged().onNotify(this, [this]() {
        ProjectMetaList recentProjects = recentProjectsProvider()->recentProjectList();
        updateRecentScores(recentProjects);
    });
}

QVariant RecentProjectsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariantMap score = m_recentScores[index.row()].toMap();

    switch (role) {
    case NameRole: return QVariant::fromValue(score[SCORE_NAME_KEY]);
    case ScoreRole: return QVariant::fromValue(score);
    }

    return QVariant();
}

int RecentProjectsModel::rowCount(const QModelIndex&) const
{
    return m_recentScores.size();
}

QHash<int, QByteArray> RecentProjectsModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { ScoreRole, "score" }
    };
}

void RecentProjectsModel::addNewScore()
{
    dispatcher()->dispatch("file-new");
}

void RecentProjectsModel::openScore()
{
    dispatcher()->dispatch("file-open");
}

void RecentProjectsModel::openRecentScore(const QString& scorePath)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path_t>(io::path_t(scorePath)));
}

void RecentProjectsModel::setRecentScores(const QVariantList& recentScores)
{
    if (m_recentScores == recentScores) {
        return;
    }

    beginResetModel();
    m_recentScores = recentScores;
    endResetModel();
}

void RecentProjectsModel::updateRecentScores(const ProjectMetaList& recentProjectsList)
{
    QVariantList recentScores;

    QVariantMap addItem;
    addItem[SCORE_NAME_KEY] = qtrc("project", "New score");
    addItem[SCORE_ADD_NEW_KEY] = true;
    recentScores << addItem;

    for (const ProjectMeta& meta : recentProjectsList) {
        QVariantMap obj;

        std::string suffix = io::suffix(meta.filePath);
        bool isSuffixInteresting = suffix != engraving::MSCZ;
        obj[SCORE_NAME_KEY] = meta.fileName(isSuffixInteresting).toQString();
        obj[SCORE_PATH_KEY] = meta.filePath.toQString();
        obj[SCORE_SUFFIX_KEY] = QString::fromStdString(suffix);

        if (!meta.thumbnail.isNull()) {
            obj[SCORE_THUMBNAIL_KEY] = !meta.thumbnail.isNull() ? meta.thumbnail : QVariant();
        }

        obj[SCORE_TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(io::FileInfo(meta.filePath).lastModified().date()).toQString();
        obj[SCORE_ADD_NEW_KEY] = false;

        recentScores << obj;
    }

    setRecentScores(recentScores);
}
