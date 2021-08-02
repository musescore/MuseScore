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
#include "userscoresservice.h"

#include "log.h"
#include "settings.h"

using namespace mu::userscores;
using namespace mu::notation;

void UserScoresService::init()
{
    updateRecentScoreList();

    configuration()->recentProjectPathsChanged().onReceive(this, [this](const io::paths&) {
        updateRecentScoreList();
    });
}

void UserScoresService::updateRecentScoreList()
{
    io::paths paths = configuration()->recentProjectPaths();
    MetaList metaList = msczMetaReader()->readMetaList(paths);
    m_recentScoreList.set(metaList);
}

mu::ValCh<MetaList> UserScoresService::recentScoreList() const
{
    return m_recentScoreList;
}
