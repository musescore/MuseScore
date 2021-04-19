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
#include "recentscoresmodel.h"

#include "log.h"
#include "translation.h"
#include "actions/actiontypes.h"
#include "dataformatter.h"

using namespace mu::userscores;
using namespace mu::actions;
using namespace mu::notation;

namespace {
const QString SCORE_TITLE_KEY("title");
const QString SCORE_PATH_KEY("path");
const QString SCORE_THUMBNAIL_KEY("thumbnail");
const QString SCORE_TIME_SINCE_CREATION_KEY("timeSinceCreation");
const QString SCORE_ADD_NEW_KEY("isAddNew");
}

RecentScoresModel::RecentScoresModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(RoleTitle, "title");
    m_roles.insert(RoleScore, "score");

    ValCh<MetaList> recentScores = userScoresService()->recentScoreList();
    updateRecentScores(recentScores.val);

    recentScores.ch.onReceive(this, [this](const MetaList& list) {
        updateRecentScores(list);
    });
}

QVariant RecentScoresModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariantMap score = m_recentScores[index.row()].toMap();

    switch (role) {
    case RoleTitle: return QVariant::fromValue(score[SCORE_TITLE_KEY]);
    case RoleScore: return QVariant::fromValue(score);
    }

    return QVariant();
}

int RecentScoresModel::rowCount(const QModelIndex&) const
{
    return m_recentScores.size();
}

QHash<int, QByteArray> RecentScoresModel::roleNames() const
{
    return m_roles;
}

void RecentScoresModel::addNewScore()
{
    dispatcher()->dispatch("file-new");
}

void RecentScoresModel::openScore()
{
    dispatcher()->dispatch("file-open");
}

void RecentScoresModel::openRecentScore(const QString& scorePath)
{
    dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(io::path(scorePath)));
}

void RecentScoresModel::setRecentScores(const QVariantList& recentScores)
{
    if (m_recentScores == recentScores) {
        return;
    }

    beginResetModel();
    m_recentScores = recentScores;
    endResetModel();
}

void RecentScoresModel::updateRecentScores(const MetaList& recentScoresList)
{
    QVariantList recentScores;

    for (const Meta& meta : recentScoresList) {
        QVariantMap obj;

        obj[SCORE_TITLE_KEY] = !meta.title.isEmpty() ? meta.title : meta.fileName.toQString();
        obj[SCORE_PATH_KEY] = meta.filePath.toQString();
        obj[SCORE_THUMBNAIL_KEY] = meta.thumbnail;
        obj[SCORE_TIME_SINCE_CREATION_KEY] = DataFormatter::formatTimeSinceCreation(meta.creationDate);
        obj[SCORE_ADD_NEW_KEY] = false;

        recentScores << obj;
    }

    QVariantMap obj;
    obj[SCORE_TITLE_KEY] = qtrc("userscores", "New Score");
    obj[SCORE_ADD_NEW_KEY] = true;

    recentScores.prepend(QVariant::fromValue(obj));

    setRecentScores(recentScores);
}
