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

using namespace mu::userscores;
using namespace mu::actions;
using namespace mu::domain::notation;

namespace {
const QString SCORE_TITLE_KEY("title");
const QString SCORE_THUMBNAIL_KEY("thumbnail");
const QString SCORE_DAYS_AGO_COUNT("daysAgoCount");
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

void RecentScoresModel::newScore()
{
    openRecentScore(0);
}

void RecentScoresModel::openScore()
{
    dispatcher()->dispatch("file-open");
}

void RecentScoresModel::openRecentScore(int index)
{
    if (index < 0 || index > m_recentScores.size()) {
        LOGD() << "Out of range recent list";
        return;
    }

    bool isNewScore = (index == 0);

    if (isNewScore) {
        dispatcher()->dispatch("file-new");
    } else {
        io::path openingScorePath = io::pathFromQString(m_recentScores.at(index).toMap().value("path").toString());
        dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(openingScorePath));
    }
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
        RetVal<Meta> meta = msczMetaReader()->readMeta(io::pathFromQString(path));

        if (!meta.ret) {
            LOGW() << "Score reader error" << path;
            continue;
        }

        QVariantMap obj;
        int daysAgoCount = QDateTime::currentDateTime().daysTo(meta.val.birthDateTime);

        obj[SCORE_TITLE_KEY] = meta.val.title;
        obj[SCORE_THUMBNAIL_KEY] = meta.val.thumbnail;
        obj[SCORE_DAYS_AGO_COUNT] = daysAgoCount;

        recentScores << obj;
    }

    QVariantMap obj;
    obj[SCORE_TITLE_KEY] = qtrc("scores", "New Score");

    recentScores.prepend(QVariant::fromValue(obj));

    setRecentScores(recentScores);
}
