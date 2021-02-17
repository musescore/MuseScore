//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_APPSHELLCONFIGURATION_H
#define MU_APPSHELL_APPSHELLCONFIGURATION_H

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresconfiguration.h"

namespace mu::appshell {
class AppShellConfiguration : public IAppShellConfiguration
{
    INJECT(appshell, userscores::IUserScoresConfiguration, userScoresConfiguration)

public:
    bool isAppUpdatable() const override;

    std::string handbookUrl(const std::string& languageCode) const override;

    ValCh<QStringList> recentScoreList() const override;

private:
    std::string utmParameters(const std::string& utmMedium) const;
};
}

#endif // MU_APPSHELL_APPSHELLCONFIGURATION_H
