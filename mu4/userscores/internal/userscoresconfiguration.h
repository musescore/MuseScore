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
#ifndef MU_USERSCORES_USERSCORECONFIGURATION_H
#define MU_USERSCORES_USERSCORECONFIGURATION_H

#include "iuserscoresconfiguration.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "extensions/iextensionsconfiguration.h"

namespace mu {
namespace userscores {
class UserScoresConfiguration : public IUserScoresConfiguration
{
    INJECT(usescores, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(userscores, extensions::IExtensionsConfiguration, extensionsConfiguration)

public:
    void init();

    ValCh<QStringList> recentScoreList() const override;
    void setRecentScoreList(const QStringList& recentScoreList) override;

    QStringList templatesDirPaths() const override;

private:
    QStringList parseRecentList(const std::string& recents) const;

    async::Channel<QStringList> m_recentListChanged;
};
}
}

#endif // MU_USERSCORES_USERSCORECONFIGURATION_H
