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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "settings.h"
#include "async/async.h"
#include "log.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::project;
using namespace mu::notation;

static const std::string module_name("project");

static const Settings::Key RECENT_PROJECTS_PATHS(module_name, "project/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key USER_PROJECTS_PATH(module_name, "application/paths/myScores");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "project/preferredScoreCreationMode");
static const Settings::Key MIGRATION_OPTIONS(module_name, "project/migration");
static const Settings::Key AUTOSAVE_ENABLED_KEY(module_name, "project/autoSaveEnabled");
static const Settings::Key AUTOSAVE_INTERVAL_KEY(module_name, "project/autoSaveInterval");

const QString ProjectConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void ProjectConfiguration::init()
{
    settings()->setDefaultValue(USER_TEMPLATES_PATH, Val(globalConfiguration()->userDataPath() + "/Templates"));
    settings()->valueChanged(USER_TEMPLATES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userTemplatesPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userTemplatesPath());

    settings()->setDefaultValue(USER_PROJECTS_PATH, Val(globalConfiguration()->userDataPath() + "/Scores"));
    settings()->valueChanged(USER_PROJECTS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userScoresPathChanged.send(val.toPath());
    });
    fileSystem()->makePath(userProjectsPath());

    settings()->valueChanged(RECENT_PROJECTS_PATHS).onReceive(nullptr, [this](const Val& val) {
        io::paths paths = parsePaths(val);
        m_recentProjectPathsChanged.send(paths);
    });

    Val preferredScoreCreationMode = Val(static_cast<int>(PreferredScoreCreationMode::FromInstruments));
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);

    settings()->setDefaultValue(AUTOSAVE_ENABLED_KEY, Val(true));
    settings()->valueChanged(AUTOSAVE_ENABLED_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveEnabledChanged.send(val.toBool());
    });

    settings()->setDefaultValue(AUTOSAVE_INTERVAL_KEY, Val(3));
    settings()->valueChanged(AUTOSAVE_INTERVAL_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveIntervalChanged.send(val.toInt());
    });
}

io::paths ProjectConfiguration::recentProjectPaths() const
{
    TRACEFUNC;

    io::paths allPaths = parsePaths(settings()->value(RECENT_PROJECTS_PATHS));
    io::paths actualPaths;

    for (const io::path& path: allPaths) {
        if (fileSystem()->exists(path)) {
            actualPaths.push_back(path);
        }
    }

    //! NOTE Save actual recent project paths
    if (allPaths != actualPaths) {
        ProjectConfiguration* self = const_cast<ProjectConfiguration*>(this);
        async::Async::call(nullptr, [self, actualPaths]() {
            self->setRecentProjectPaths(actualPaths);
        });
    }

    return actualPaths;
}

void ProjectConfiguration::setRecentProjectPaths(const io::paths& recentScorePaths)
{
    QStringList paths;

    for (const io::path& path : recentScorePaths) {
        paths << path.toQString();
    }

    Val value(paths.join(",").toStdString());
    settings()->setSharedValue(RECENT_PROJECTS_PATHS, value);
}

async::Channel<io::paths> ProjectConfiguration::recentProjectPathsChanged() const
{
    return m_recentProjectPathsChanged;
}

io::paths ProjectConfiguration::parsePaths(const Val& value) const
{
    if (value.isNull()) {
        return io::paths();
    }

    QString pathsStr = value.toQString();
    if (pathsStr.isEmpty()) {
        return {};
    }

    QStringList paths = pathsStr.split(",");
    return io::pathsFromStrings(paths);
}

io::path ProjectConfiguration::myFirstProjectPath() const
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

io::path ProjectConfiguration::userProjectsPath() const
{
    return settings()->value(USER_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setUserProjectsPath(const io::path& path)
{
    settings()->setSharedValue(USER_PROJECTS_PATH, Val(path));
}

async::Channel<io::path> ProjectConfiguration::userProjectsPathChanged() const
{
    return m_userScoresPathChanged;
}

io::path ProjectConfiguration::defaultSavingFilePath(const io::path& fileName) const
{
    return userProjectsPath() + "/" + fileName + DEFAULT_FILE_SUFFIX;
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

MigrationOptions ProjectConfiguration::migrationOptions() const
{
    QString json = settings()->value(MIGRATION_OPTIONS).toQString();
    QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();

    MigrationOptions opt;
    opt.appVersion = obj.value("appVersion").toInt(0);
    opt.isApplyMigration = obj.value("isApplyMigration").toBool(false);
    opt.isAskAgain = obj.value("isAskAgain").toBool(true);

    opt.isApplyLeland = obj.value("isApplyLeland").toBool(false);
    opt.isApplyEdwin = obj.value("isApplyEdwin").toBool(false);

    return opt;
}

void ProjectConfiguration::setMigrationOptions(const MigrationOptions& opt)
{
    QJsonObject obj;
    obj["appVersion"] = opt.appVersion;
    obj["isApplyMigration"] = opt.isApplyMigration;
    obj["isAskAgain"] = opt.isAskAgain;

    obj["isApplyLeland"] = opt.isApplyLeland;
    obj["isApplyEdwin"] = opt.isApplyEdwin;

    QString json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    settings()->setSharedValue(MIGRATION_OPTIONS, Val(json));
}

bool ProjectConfiguration::isAutoSaveEnabled() const
{
    return settings()->value(AUTOSAVE_ENABLED_KEY).toBool();
}

void ProjectConfiguration::setAutoSaveEnabled(bool enabled)
{
    settings()->setSharedValue(AUTOSAVE_ENABLED_KEY, Val(enabled));
}

async::Channel<bool> ProjectConfiguration::autoSaveEnabledChanged() const
{
    return m_autoSaveEnabledChanged;
}

int ProjectConfiguration::autoSaveIntervalMinutes() const
{
    return settings()->value(AUTOSAVE_INTERVAL_KEY).toInt();
}

void ProjectConfiguration::setAutoSaveInterval(int minutes)
{
    settings()->setSharedValue(AUTOSAVE_INTERVAL_KEY, Val(minutes));
}

async::Channel<int> ProjectConfiguration::autoSaveIntervalChanged() const
{
    return m_autoSaveIntervalChanged;
}
