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
#include "userscoresconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::userscores;

static std::string module_name("userscores");

static const Settings::Key RECENT_LIST(module_name, "scores/recentList");

void UserScoresConfiguration::init()
{
    settings()->valueChanged(RECENT_LIST).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "RECENT_LIST changed: " << val.toString();

        QStringList recentList = parseRecentList(val.toString());
        m_recentListChanged.send(recentList);
    });
}

ValCh<QStringList> UserScoresConfiguration::recentScoreList() const
{
    ValCh<QStringList> result;
    result.ch = m_recentListChanged;
    result.val = parseRecentList(settings()->value(RECENT_LIST).toString());

    return result;
}

void UserScoresConfiguration::setRecentScoreList(const QStringList& recentList)
{
    Val value(recentList.join(",").toStdString());
    settings()->setValue(RECENT_LIST, value);
}

QStringList UserScoresConfiguration::parseRecentList(const std::string& recents) const
{
    if (recents.empty()) {
        return QStringList();
    }

    return QString::fromStdString(recents).split(",");
}
