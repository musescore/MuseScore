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
#include "projectconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::project;
using namespace mu::notation;

static const std::string module_name("project");

static const Settings::Key RECENT_SCORE_PATHS(module_name, "userscores/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key USER_SCORES_PATH(module_name, "application/paths/myScores");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "userscores/preferredScoreCreationMode");
static const Settings::Key NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY(module_name, "userscores/unsavedScoreWarning");

const QString ProjectConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void ProjectConfiguration::init()
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

io::paths ProjectConfiguration::actualRecentScorePaths() const
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

ValCh<io::paths> ProjectConfiguration::recentScorePaths() const
{
    TRACEFUNC;

    ValCh<io::paths> result;
    result.ch = m_recentScorePathsChanged;
    result.val = actualRecentScorePaths();

    return result;
}

void ProjectConfiguration::setRecentScorePaths(const io::paths& recentScorePaths)
{
    QStringList paths;

    for (const io::path& path : recentScorePaths) {
        paths << path.toQString();
    }

    Val value(paths.join(",").toStdString());
    settings()->setSharedValue(RECENT_SCORE_PATHS, value);
}

io::paths ProjectConfiguration::parsePaths(const Val& value) const
{
    if (value.isNull()) {
        return io::paths();
    }

    QStringList paths = value.toQString().split(",");
    return io::pathsFromStrings(paths);
}

io::path ProjectConfiguration::myFirstScorePath() const
{
    return appTemplatesPath() + "/My_First_Score.mscx";
}

io::path ProjectConfiguration::appTemplatesPath() const
{
    return globalConfiguration()->appDataPath() + "/templates";
}

io::paths ProjectConfiguration::availableTemplatesPaths() const
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

io::path ProjectConfiguration::userTemplatesPath() const
{
    return settings()->value(USER_TEMPLATES_PATH).toPath();
}

void ProjectConfiguration::setUserTemplatesPath(const io::path& path)
{
    settings()->setSharedValue(USER_TEMPLATES_PATH, Val(path));
}

async::Channel<io::path> ProjectConfiguration::userTemplatesPathChanged() const
{
    return m_userTemplatesPathChanged;
}

io::path ProjectConfiguration::userScoresPath() const
{
    return settings()->value(USER_SCORES_PATH).toPath();
}

void ProjectConfiguration::setUserScoresPath(const io::path& path)
{
    settings()->setSharedValue(USER_SCORES_PATH, Val(path));
}

async::Channel<io::path> ProjectConfiguration::userScoresPathChanged() const
{
    return m_userScoresPathChanged;
}

io::path ProjectConfiguration::defaultSavingFilePath(const io::path& fileName) const
{
    return userScoresPath() + "/" + fileName + DEFAULT_FILE_SUFFIX;
}

QColor ProjectConfiguration::templatePreviewBackgroundColor() const
{
    return notationConfiguration()->backgroundColor();
}

async::Notification ProjectConfiguration::templatePreviewBackgroundChanged() const
{
    return notationConfiguration()->backgroundChanged();
}

ProjectConfiguration::PreferredScoreCreationMode ProjectConfiguration::preferredScoreCreationMode() const
{
    return static_cast<PreferredScoreCreationMode>(settings()->value(PREFERRED_SCORE_CREATION_MODE_KEY).toInt());
}

void ProjectConfiguration::setPreferredScoreCreationMode(PreferredScoreCreationMode mode)
{
    settings()->setSharedValue(PREFERRED_SCORE_CREATION_MODE_KEY, Val(static_cast<int>(mode)));
}

bool ProjectConfiguration::needShowWarningAboutUnsavedScore() const
{
    return settings()->value(NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY).toBool();
}

void ProjectConfiguration::setNeedShowWarningAboutUnsavedScore(bool value)
{
    settings()->setSharedValue(NEED_SHOW_WARNING_ABOUT_UNSAVED_SCORE_KEY, Val(value));
}
