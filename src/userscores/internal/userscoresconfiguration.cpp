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
using namespace mu::notation;

static const std::string module_name("userscores");

static const Settings::Key RECENT_LIST(module_name, "userscores/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key USER_SCORES_PATH(module_name, "application/paths/myScores");

const QString UserScoresConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void UserScoresConfiguration::init()
{
    settings()->setDefaultValue(USER_SCORES_PATH, Val(globalConfiguration()->sharePath().toStdString() + "Scores"));
    settings()->valueChanged(RECENT_LIST).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "RECENT_LIST changed: " << val.toString();

        QStringList recentList = parseRecentList(val.toString());
        m_recentListChanged.send(recentList);
    });
}

ValCh<QStringList> UserScoresConfiguration::recentScoreList() const
{
    TRACEFUNC;
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

io::paths UserScoresConfiguration::templatesDirPaths() const
{
    io::paths dirs;

    dirs.push_back(globalConfiguration()->sharePath() + "templates");
    dirs.push_back(settings()->value(USER_TEMPLATES_PATH).toQString());

    io::paths temps = extensionsConfiguration()->templatesPaths();
    dirs.insert(dirs.end(), temps.begin(), temps.end());

    return dirs;
}

io::path UserScoresConfiguration::scoresPath() const
{
    return settings()->value(USER_SCORES_PATH).toString();
}

io::path UserScoresConfiguration::defaultSavingFilePath(const std::string& fileName) const
{
    return scoresPath() + "/" + fileName + DEFAULT_FILE_SUFFIX;
}

QColor UserScoresConfiguration::templatePreviewBackgroundColor() const
{
    return notationConfiguration()->backgroundColor();
}

async::Channel<QColor> UserScoresConfiguration::templatePreviewBackgroundColorChanged() const
{
    return notationConfiguration()->backgroundColorChanged();
}
