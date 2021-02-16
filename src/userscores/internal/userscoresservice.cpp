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
#include "userscoresservice.h"

#include "log.h"
#include "settings.h"

using namespace mu::userscores;
using namespace mu::notation;

void UserScoresService::init()
{
    configuration()->recentScoreList().ch.onReceive(this, [this](const QStringList& recentScoresPathList) {
        m_recentScoreListChanged.send(parseRecentList(recentScoresPathList));
    });
}

mu::ValCh<std::vector<Meta> > UserScoresService::recentScoreList() const
{
    TRACEFUNC;
    ValCh<std::vector<Meta> > result;
    result.ch = m_recentScoreListChanged;
    result.val = parseRecentList(configuration()->recentScoreList().val);

    return result;
}

std::vector<Meta> UserScoresService::parseRecentList(const QStringList& recentScoresPathList) const
{
    std::vector<Meta> result;
    for (const QString& path : recentScoresPathList) {
        RetVal<Meta> meta = msczMetaReader()->readMeta(path);
        if (!meta.ret) {
            LOGE() << "Score reader error" << path;
            continue;
        }

        result.push_back(meta.val);
    }

    return result;
}
