//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

    ValCh<QStringList> recentScoresCh = configuration()->recentScoreList();
    updateRecentScores(recentScoresCh.val);

    recentScoresCh.ch.onReceive(this, [this](const QStringList& list) {
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

void RecentScoresModel::updateRecentScores(const QStringList& recentScoresPathList)
{
    QVariantList recentScores;

    for (const QString& path : recentScoresPathList) {
        RetVal<Meta> meta = msczMetaReader()->readMeta(path);

        if (!meta.ret) {
            LOGW() << "Score reader error" << path;
            continue;
        }

        QVariantMap obj;

        obj[SCORE_TITLE_KEY] = meta.val.title;
        obj[SCORE_PATH_KEY] = path;
        obj[SCORE_THUMBNAIL_KEY] = meta.val.thumbnail;
        obj[SCORE_TIME_SINCE_CREATION_KEY] = DataFormatter::formatTimeSinceCreation(meta.val.creationDate);
        obj[SCORE_ADD_NEW_KEY] = false;

        recentScores << obj;
    }

    QVariantMap obj;
    obj[SCORE_TITLE_KEY] = qtrc("userscores", "New Score");
    obj[SCORE_ADD_NEW_KEY] = true;

    recentScores.prepend(QVariant::fromValue(obj));

    setRecentScores(recentScores);
}
