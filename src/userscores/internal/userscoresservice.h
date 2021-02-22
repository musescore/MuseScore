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
#ifndef MU_USERSCORES_USERSCORESSERVICE_H
#define MU_USERSCORES_USERSCORESSERVICE_H

#include "iuserscoresservice.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iuserscoresconfiguration.h"
#include "notation/imsczmetareader.h"

namespace mu::userscores {
class UserScoresService : public IUserScoresService, public async::Asyncable
{
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, notation::IMsczMetaReader, msczMetaReader)

public:
    void init();

    ValCh<std::vector<notation::Meta> > recentScoreList() const override;

private:
    std::vector<notation::Meta> parseRecentList(const QStringList& recentScoresPathList) const;

    async::Channel<std::vector<notation::Meta> > m_recentScoreListChanged;
};
}

#endif // MU_USERSCORES_USERSCORESSERVICE_H
