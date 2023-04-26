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

#include "engraving/infrastructure/mscio.h"

#include "log.h"

using namespace mu::project;
using namespace mu::actions;

static const QString NAME_KEY("name");
static const QString PATH_KEY("path");
static const QString SUFFIX_KEY("suffix");
static const QString THUMBNAIL_KEY("thumbnail");
static const QString TIME_SINCE_MODIFIED_KEY("timeSinceModified");
static const QString ADD_NEW_KEY("isAddNew");
static const QString NO_RESULT_FOUND_KEY("isNoResultFound");
static const QString IS_CLOUD_KEY("isCloud");

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
    case NameRole: return QVariant::fromValue(score[NAME_KEY]);
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

void RecentProjectsModel::openScoreManager()
{
    interactive()->openUrl(museScoreComCloudService()->scoreManagerUrl());
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
    addItem[NAME_KEY] = qtrc("project", "New score");
    addItem[ADD_NEW_KEY] = true;
    addItem[NO_RESULT_FOUND_KEY] = false;
    addItem[IS_CLOUD_KEY] = false;
    recentScores << addItem;

    for (const ProjectMeta& meta : recentProjectsList) {
        QVariantMap obj;

        std::string suffix = io::suffix(meta.filePath);
        bool isSuffixInteresting = suffix != engraving::MSCZ;
        obj[NAME_KEY] = meta.fileName(isSuffixInteresting).toQString();
        obj[PATH_KEY] = meta.filePath.toQString();
        obj[SUFFIX_KEY] = QString::fromStdString(suffix);
        obj[IS_CLOUD_KEY] = configuration()->isCloudProject(meta.filePath);

        if (!meta.thumbnail.isNull()) {
            obj[THUMBNAIL_KEY] = !meta.thumbnail.isNull() ? meta.thumbnail : QVariant();
        }

        obj[TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(io::FileInfo(meta.filePath).lastModified().date()).toQString();
        obj[ADD_NEW_KEY] = false;
        obj[NO_RESULT_FOUND_KEY] = false;

        recentScores << obj;
    }

    QVariantMap noResultsFoundItem;
    noResultsFoundItem[NAME_KEY] = "";
    noResultsFoundItem[ADD_NEW_KEY] = false;
    noResultsFoundItem[NO_RESULT_FOUND_KEY] = true;
    noResultsFoundItem[IS_CLOUD_KEY] = false;
    recentScores << noResultsFoundItem;

    setRecentScores(recentScores);
}
