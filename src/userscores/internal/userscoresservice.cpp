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
    updateRecentScoreList();

    configuration()->recentScorePaths().ch.onReceive(this, [this](const io::paths&) {
        updateRecentScoreList();
    });
}

void UserScoresService::updateRecentScoreList()
{
    io::paths paths = configuration()->recentScorePaths().val;
    MetaList metaList = msczMetaReader()->readMetaList(paths);
    m_recentScoreList.set(metaList);
}

mu::ValCh<MetaList> UserScoresService::recentScoreList() const
{
    return m_recentScoreList;
}
