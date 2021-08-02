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
#ifndef MU_USERSCORES_USERSCORESSERVICE_H
#define MU_USERSCORES_USERSCORESSERVICE_H

#include "iuserscoresservice.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "project/iprojectconfiguration.h"
#include "notation/imsczmetareader.h"

namespace mu::userscores {
class UserScoresService : public IUserScoresService, public async::Asyncable
{
    INJECT(userscores, project::IProjectConfiguration, configuration)
    INJECT(userscores, notation::IMsczMetaReader, msczMetaReader)

public:
    void init();

    ValCh<notation::MetaList> recentScoreList() const override;

private:
    void updateRecentScoreList();

    ValCh<notation::MetaList> m_recentScoreList;
};
}

#endif // MU_USERSCORES_USERSCORESSERVICE_H
