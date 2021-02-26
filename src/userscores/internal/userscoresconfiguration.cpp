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

static const Settings::Key RECENT_SCORE_PATHS(module_name, "userscores/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key USER_SCORES_PATH(module_name, "application/paths/myScores");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "userscores/preferedScoreCreationMode");

const QString UserScoresConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void UserScoresConfiguration::init()
{
    settings()->setDefaultValue(USER_SCORES_PATH, Val(globalConfiguration()->sharePath().toStdString() + "Scores"));
    settings()->valueChanged(RECENT_SCORE_PATHS).onReceive(nullptr, [this](const Val& val) {
        io::paths paths = parsePaths(val);
        m_recentScorePathsChanged.send(paths);
    });

    Val preferredScoreCreationMode = Val(static_cast<int>(PreferredScoreCreationMode::FromInstruments));
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);
}

ValCh<io::paths> UserScoresConfiguration::recentScorePaths() const
{
    TRACEFUNC;

    ValCh<io::paths> result;
    result.ch = m_recentScorePathsChanged;
    result.val = parsePaths(settings()->value(RECENT_SCORE_PATHS));

    return result;
}

void UserScoresConfiguration::setRecentScorePaths(const io::paths& recentScorePaths)
{
    QStringList paths;

    for (const io::path& path : recentScorePaths) {
        paths << path.toQString();
    }

    Val value(paths.join(",").toStdString());
    settings()->setValue(RECENT_SCORE_PATHS, value);
}

io::paths UserScoresConfiguration::parsePaths(const Val& value) const
{
    if (value.isNull()) {
        return io::paths();
    }

    QStringList paths = value.toQString().split(",");
    io::paths result;

    for (const QString& path : paths) {
        result.push_back(path);
    }

    return result;
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

UserScoresConfiguration::PreferredScoreCreationMode UserScoresConfiguration::preferredScoreCreationMode() const
{
    return static_cast<PreferredScoreCreationMode>(settings()->value(PREFERRED_SCORE_CREATION_MODE_KEY).toInt());
}

void UserScoresConfiguration::setPreferredScoreCreationMode(PreferredScoreCreationMode mode)
{
    settings()->setValue(PREFERRED_SCORE_CREATION_MODE_KEY, Val(static_cast<int>(mode)));
}
