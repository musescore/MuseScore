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
#include "appshellconfiguration.h"

#include "config.h"

static const std::string ONLINE_HANDBOOK_URL("https://musescore.org/redirect/help?tag=handbook&locale=");

using namespace mu::appshell;

bool AppShellConfiguration::isAppUpdatable() const
{
#if defined(APP_UPDATABLE)
    return true;
#else
    return false;
#endif
}

std::string AppShellConfiguration::handbookUrl(const std::string& languageCode) const
{
    std::string utm = utmParameters("menu");

    return ONLINE_HANDBOOK_URL + languageCode + "&" + utm;
}

mu::ValCh<QStringList> AppShellConfiguration::recentScoreList() const
{
    return userScoresConfiguration()->recentScoreList();
}

std::string AppShellConfiguration::utmParameters(const std::string& utmMedium) const
{
    return "utm_source=desktop&utm_medium=" + utmMedium
           + "&utm_content=" + MUSESCORE_REVISION
           + "&utm_campaign=MuseScore" + VERSION;
}
