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
#include <QJsonArray>

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
static const Settings::Key SHOULD_ASK_SAVE_LOCATION_TYPE(module_name, "project/shouldAskSaveLocationType");
static const Settings::Key LAST_USED_SAVE_LOCATION_TYPE(module_name, "project/lastUsedSaveLocationType");
static const Settings::Key SHOULD_WARN_BEFORE_PUBLISHING(module_name, "project/shouldWarnBeforePublishing");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "project/preferredScoreCreationMode");
static const Settings::Key MIGRATION_OPTIONS(module_name, "project/migration");
static const Settings::Key AUTOSAVE_ENABLED_KEY(module_name, "project/autoSaveEnabled");
static const Settings::Key AUTOSAVE_INTERVAL_KEY(module_name, "project/autoSaveInterval");
static const Settings::Key SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT(module_name, "project/shouldDestinationFolderBeOpenedOnExport");

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
        io::paths paths = parseRecentProjectsPaths(val);
        m_recentProjectPathsChanged.send(paths);
    });

    Val preferredScoreCreationMode = Val(PreferredScoreCreationMode::FromInstruments);
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);

    settings()->setDefaultValue(SHOULD_ASK_SAVE_LOCATION_TYPE, Val(true));
    settings()->setDefaultValue(LAST_USED_SAVE_LOCATION_TYPE, Val(SaveLocationType::Undefined));

    settings()->setDefaultValue(SHOULD_WARN_BEFORE_PUBLISHING, Val(true));

    settings()->setDefaultValue(AUTOSAVE_ENABLED_KEY, Val(true));
    settings()->valueChanged(AUTOSAVE_ENABLED_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveEnabledChanged.send(val.toBool());
    });

    settings()->setDefaultValue(AUTOSAVE_INTERVAL_KEY, Val(3));
    settings()->valueChanged(AUTOSAVE_INTERVAL_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveIntervalChanged.send(val.toInt());
    });

    settings()->setDefaultValue(SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT, Val(false));
}

io::paths ProjectConfiguration::recentProjectPaths() const
{
    TRACEFUNC;

    io::paths allPaths = parseRecentProjectsPaths(settings()->value(RECENT_PROJECTS_PATHS));
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
    TRACEFUNC;

    QJsonArray jsonArray;
    for (const io::path& path : recentScorePaths) {
        jsonArray << path.toQString();
    }

    QJsonDocument jsonDoc(jsonArray);

    Val value(jsonDoc.toJson(QJsonDocument::Compact).constData());
    settings()->setSharedValue(RECENT_PROJECTS_PATHS, value);
}

async::Channel<io::paths> ProjectConfiguration::recentProjectPathsChanged() const
{
    return m_recentProjectPathsChanged;
}

io::paths ProjectConfiguration::parseRecentProjectsPaths(const Val& value) const
{
    TRACEFUNC;
    io::paths result;

    if (value.isNull()) {
        return result;
    }

    QByteArray json = value.toQString().toUtf8();
    if (json.isEmpty()) {
        return result;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        return result;
    }

    if (!jsonDoc.isArray()) {
        return result;
    }

    for (const QJsonValue& val : jsonDoc.array()) {
        result.push_back(val.toString());
    }

    return result;
}

io::path ProjectConfiguration::myFirstProjectPath() const
{
    return appTemplatesPath() + "/My_First_Score.mscx";
}

io::path ProjectConfiguration::appTemplatesPath() const
{
    return globalConfiguration()->appDataPath() + "/templates";
}

io::paths ProjectConfiguration::availableTemplateDirs() const
{
    io::paths dirs;

    io::path defaultTemplatesPath = this->appTemplatesPath();
    dirs.push_back(defaultTemplatesPath);

    io::path userTemplatesPath = this->userTemplatesPath();
    if (!userTemplatesPath.empty() && userTemplatesPath != defaultTemplatesPath) {
        dirs.push_back(userTemplatesPath);
    }

    return dirs;
}

io::path ProjectConfiguration::templateCategoriesJsonPath(const io::path& templatesDir) const
{
    return templatesDir + "/categories.json";
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

io::path ProjectConfiguration::cloudProjectsPath() const
{
    return globalConfiguration()->userAppDataPath() + "/cloud_projects";
}

bool ProjectConfiguration::isCloudProject(const io::path& path) const
{
    return io::dirpath(path) == cloudProjectsPath();
}

io::path ProjectConfiguration::defaultSavingFilePath(INotationProjectPtr project, const QString& filenameAddition,
                                                     const QString& suffix) const
{
    io::path folderPath;
    io::path filename;
    QString theSuffix = suffix;

    io::path projectPath = project->path();

    if (project->isNewlyCreated()) {
        if (io::isAbsolute(projectPath)) {
            folderPath = io::dirpath(projectPath);
        }

        filename = io::filename(projectPath, false);
    } else if (project->isCloudProject()) {
        // TODO(save-to-cloud)
    } else {
        folderPath = io::dirpath(projectPath);
        filename = io::filename(projectPath, false);

        if (theSuffix.isEmpty()) {
            theSuffix = QString::fromStdString(io::suffix(projectPath));
        }
    }

    if (folderPath.empty()) {
        folderPath = userProjectsPath();
    }

    if (filename.empty()) {
        filename = project->metaInfo().title;
    }

    if (filename.empty()) {
        filename = qtrc("project", "Untitled");
    }

    if (theSuffix.isEmpty()) {
        theSuffix = DEFAULT_FILE_SUFFIX;
    }

    return folderPath
           .appendingComponent(filename + filenameAddition)
           .appendingSuffix(theSuffix);
}

bool ProjectConfiguration::shouldAskSaveLocationType() const
{
    return settings()->value(SHOULD_ASK_SAVE_LOCATION_TYPE).toBool();
}

void ProjectConfiguration::setShouldAskSaveLocationType(bool shouldAsk)
{
    settings()->setSharedValue(SHOULD_ASK_SAVE_LOCATION_TYPE, Val(shouldAsk));
}

SaveLocationType ProjectConfiguration::lastUsedSaveLocationType() const
{
    return settings()->value(LAST_USED_SAVE_LOCATION_TYPE).toEnum<SaveLocationType>();
}

void ProjectConfiguration::setLastUsedSaveLocationType(SaveLocationType type)
{
    settings()->setSharedValue(LAST_USED_SAVE_LOCATION_TYPE, Val(type));
}

bool ProjectConfiguration::shouldWarnBeforePublishing() const
{
    return settings()->value(SHOULD_WARN_BEFORE_PUBLISHING).toBool();
}

void ProjectConfiguration::setShouldWarnBeforePublishing(bool shouldWarn)
{
    settings()->setSharedValue(SHOULD_WARN_BEFORE_PUBLISHING, Val(shouldWarn));
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
    return settings()->value(PREFERRED_SCORE_CREATION_MODE_KEY).toEnum<PreferredScoreCreationMode>();
}

void ProjectConfiguration::setPreferredScoreCreationMode(PreferredScoreCreationMode mode)
{
    settings()->setSharedValue(PREFERRED_SCORE_CREATION_MODE_KEY, Val(mode));
}

MigrationOptions ProjectConfiguration::migrationOptions(MigrationType type) const
{
    auto it = m_migrationOptions.find(type);
    if (it != m_migrationOptions.end()) {
        return it->second;
    }

    QString json = settings()->value(MIGRATION_OPTIONS).toQString();
    QJsonArray array = QJsonDocument::fromJson(json.toUtf8()).array();

    MigrationOptions result;

    for (const QVariant& obj : array.toVariantList()) {
        QVariantMap map = obj.toMap();
        auto migrationType = static_cast<MigrationType>(map["migrationType"].toInt());
        QVariantMap optionsObj = map["options"].toMap();

        MigrationOptions opt;
        opt.appVersion = optionsObj.value("appVersion", 0).toInt();
        opt.isApplyMigration = optionsObj.value("isApplyMigration", false).toBool();
        opt.isAskAgain = optionsObj.value("isAskAgain", false).toBool();
        opt.isApplyLeland = optionsObj.value("isApplyLeland", false).toBool();
        opt.isApplyEdwin = optionsObj.value("isApplyEdwin", false).toBool();
        opt.isApplyAutoSpacing = optionsObj.value("isApplyAutoSpacing", false).toBool();

        m_migrationOptions[migrationType] = opt;

        if (migrationType == type) {
            result = opt;
        }
    }

    return result;
}

void ProjectConfiguration::setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent)
{
    m_migrationOptions[type] = opt;

    if (!persistent) {
        return;
    }

    QVariantList objList;

    for (auto it = m_migrationOptions.cbegin(); it != m_migrationOptions.cend(); ++it) {
        const MigrationOptions& o = it->second;

        QVariantMap options;
        options["appVersion"] = o.appVersion;
        options["isApplyMigration"] = o.isApplyMigration;
        options["isAskAgain"] = o.isAskAgain;
        options["isApplyLeland"] = o.isApplyLeland;
        options["isApplyEdwin"] = o.isApplyEdwin;

        QVariantMap map;
        map["migrationType"] = static_cast<int>(it->first);
        map["options"] = options;

        objList << map;
    }

    QJsonArray array = QJsonArray::fromVariantList(objList);
    QString json = QJsonDocument(array).toJson(QJsonDocument::Compact);
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

io::path ProjectConfiguration::newProjectTemporaryPath() const
{
    return globalConfiguration()->userAppDataPath() + "/new_project" + DEFAULT_FILE_SUFFIX;
}

bool ProjectConfiguration::isAccessibleEnabled() const
{
    return accessibilityConfiguration()->enabled();
}

bool ProjectConfiguration::shouldDestinationFolderBeOpenedOnExport() const
{
    return settings()->value(SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT).toBool();
}

void ProjectConfiguration::setShouldDestinationFolderBeOpenedOnExport(bool shouldDestinationFolderBeOpenedOnExport)
{
    settings()->setSharedValue(SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT, Val(shouldDestinationFolderBeOpenedOnExport));
}
