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
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "userscores/preferredScoreCreationMode");
static const Settings::Key NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY(module_name, "userscores/unsavedScoreWarning");

const QString UserScoresConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void UserScoresConfiguration::init()
{
    settings()->setDefaultValue(USER_TEMPLATES_PATH, Val(globalConfiguration()->userDataPath() + "/Templates"));
    settings()->valueChanged(USER_TEMPLATES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userTemplatesPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userTemplatesPath());

    settings()->setDefaultValue(USER_SCORES_PATH, Val(globalConfiguration()->userDataPath() + "/Scores"));
    settings()->valueChanged(USER_SCORES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userScoresPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userScoresPath());

    settings()->valueChanged(RECENT_SCORE_PATHS).onReceive(nullptr, [this](const Val& val) {
        io::paths paths = parsePaths(val);
        m_recentScorePathsChanged.send(paths);
    });

    Val preferredScoreCreationMode = Val(static_cast<int>(PreferredScoreCreationMode::FromInstruments));
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);

    io::paths paths = actualRecentScorePaths();
    setRecentScorePaths(paths);

    settings()->setDefaultValue(NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY, Val(true));
}

io::paths UserScoresConfiguration::actualRecentScorePaths() const
{
    io::paths allPaths = parsePaths(settings()->value(RECENT_SCORE_PATHS));
    io::paths actualPaths;

    for (const io::path& path: allPaths) {
        if (fileSystem()->exists(path)) {
            actualPaths.push_back(path);
        }
    }

    return actualPaths;
}

ValCh<io::paths> UserScoresConfiguration::recentScorePaths() const
{
    TRACEFUNC;

    ValCh<io::paths> result;
    result.ch = m_recentScorePathsChanged;
    result.val = actualRecentScorePaths();

    return result;
}

void UserScoresConfiguration::setRecentScorePaths(const io::paths& recentScorePaths)
{
    QStringList paths;

    for (const io::path& path : recentScorePaths) {
        paths << path.toQString();
    }

    Val value(paths.join(",").toStdString());
    settings()->setSharedValue(RECENT_SCORE_PATHS, value);
}

io::paths UserScoresConfiguration::parsePaths(const Val& value) const
{
    if (value.isNull()) {
        return io::paths();
    }

    QStringList paths = value.toQString().split(",");
    return io::pathsFromStrings(paths);
}

io::path UserScoresConfiguration::myFirstScorePath() const
{
    return appTemplatesPath() + "/My_First_Score.mscx";
}

io::path UserScoresConfiguration::appTemplatesPath() const
{
    return globalConfiguration()->appDataPath() + "/templates";
}

io::paths UserScoresConfiguration::availableTemplatesPaths() const
{
    io::paths dirs;

    io::path defaultTemplatesPath = this->appTemplatesPath();
    dirs.push_back(defaultTemplatesPath);

    io::path userTemplatesPath = this->userTemplatesPath();
    if (!userTemplatesPath.empty() && userTemplatesPath != defaultTemplatesPath) {
        dirs.push_back(userTemplatesPath);
    }

    io::paths temps = extensionsConfiguration()->templatesPaths();
    dirs.insert(dirs.end(), temps.begin(), temps.end());

    return dirs;
}

io::path UserScoresConfiguration::userTemplatesPath() const
{
    return settings()->value(USER_TEMPLATES_PATH).toPath();
}

void UserScoresConfiguration::setUserTemplatesPath(const io::path& path)
{
    settings()->setSharedValue(USER_TEMPLATES_PATH, Val(path));
}

async::Channel<io::path> UserScoresConfiguration::userTemplatesPathChanged() const
{
    return m_userTemplatesPathChanged;
}

io::path UserScoresConfiguration::userScoresPath() const
{
    return settings()->value(USER_SCORES_PATH).toPath();
}

void UserScoresConfiguration::setUserScoresPath(const io::path& path)
{
    settings()->setSharedValue(USER_SCORES_PATH, Val(path));
}

async::Channel<io::path> UserScoresConfiguration::userScoresPathChanged() const
{
    return m_userScoresPathChanged;
}

io::path UserScoresConfiguration::defaultSavingFilePath(const io::path& fileName) const
{
    return userScoresPath() + "/" + fileName + DEFAULT_FILE_SUFFIX;
}

QColor UserScoresConfiguration::templatePreviewBackgroundColor() const
{
    return notationConfiguration()->backgroundColor();
}

async::Notification UserScoresConfiguration::templatePreviewBackgroundChanged() const
{
    return notationConfiguration()->backgroundChanged();
}

UserScoresConfiguration::PreferredScoreCreationMode UserScoresConfiguration::preferredScoreCreationMode() const
{
    return static_cast<PreferredScoreCreationMode>(settings()->value(PREFERRED_SCORE_CREATION_MODE_KEY).toInt());
}

void UserScoresConfiguration::setPreferredScoreCreationMode(PreferredScoreCreationMode mode)
{
    settings()->setSharedValue(PREFERRED_SCORE_CREATION_MODE_KEY, Val(static_cast<int>(mode)));
}

bool UserScoresConfiguration::needShowWarningAboutUnsavedScore() const
{
    return settings()->value(NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY).toBool();
}

void UserScoresConfiguration::setNeedShowWarningAboutUnsavedScore(bool value)
{
    settings()->setSharedValue(NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY, Val(value));
}
