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
#include "scoresmodel.h"

#include "log.h"
#include "actions/actiontypes.h"

using namespace mu::scores;
using namespace mu::actions;
using namespace mu::domain::notation;

ScoresModel::ScoresModel(QObject *parent) : QObject(parent)
{
    ValCh<QStringList> recentList = scoresConfiguration()->recentList();
    updateRecentList(recentList.val);

    recentList.ch.onReceive(this, [this](const QStringList& list) {
        updateRecentList(list);
    });
}

void ScoresModel::openScore()
{
    dispatcher()->dispatch("file-open");
}

void ScoresModel::importScore()
{
    dispatcher()->dispatch("file-import");
}

void ScoresModel::openRecentScore(int index)
{
    if (index < 0 || index > m_recentList.size()) {
        LOGD() << "Out of range recent list";
        return;
    }

    bool isNewScore = (index == 0);

    if (isNewScore) {
        openNewScoreCreator();
    } else {
        io::path openingScorePath = io::pathFromQString(m_recentList.at(index).toMap().value("path").toString());
        dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(openingScorePath));
    }
}

QVariantList ScoresModel::recentList()
{
    return m_recentList;
}

void ScoresModel::setRecentList(const QVariantList &recentList)
{
    if (m_recentList == recentList) {
        return;
    }

    m_recentList = recentList;
    emit recentListChanged(recentList);
}

void ScoresModel::updateRecentList(const QStringList &recentList)
{
    QVariantList recentVariantList;

    for (const QString& recent : recentList) {

        RetVal<Meta> meta = msczMetaReader()->readMeta(io::pathFromQString(recent));

        if (!meta.ret) {
            LOGW() << "Score reader error" << recent;
            continue;
        }

        QVariantMap obj;
        obj["path"] = recent;
        obj["title"] = meta.val.title;
        obj["thumbnail"] = meta.val.thumbnail;

        recentVariantList << obj;
    }

    QVariantMap obj;
    obj["title"] = "New Score";

    recentVariantList.prepend(QVariant::fromValue(obj));

    setRecentList(recentVariantList);
}

void ScoresModel::openNewScoreCreator()
{
    launcher()->open("musescore://scores/newscore");
}
