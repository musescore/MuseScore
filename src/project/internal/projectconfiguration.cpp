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

#include "engraving/infrastructure/mscio.h"

#include "log.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::project;
using namespace mu::notation;

static const std::string module_name("project");

static const Settings::Key RECENT_PROJECTS_PATHS(module_name, "project/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key DEFAULT_PROJECTS_PATH(module_name, "application/paths/defaultProjectsPath");
static const Settings::Key LAST_OPENED_PROJECTS_PATH(module_name, "application/paths/lastOpenedProjectsPath");
static const Settings::Key LAST_SAVED_PROJECTS_PATH(module_name, "application/paths/lastSavedProjectsPath");
static const Settings::Key USER_PROJECTS_PATH(module_name, "application/paths/myScores");
static const Settings::Key SHOULD_ASK_SAVE_LOCATION_TYPE(module_name, "project/shouldAskSaveLocationType");
static const Settings::Key LAST_USED_SAVE_LOCATION_TYPE(module_name, "project/lastUsedSaveLocationType");
static const Settings::Key SHOULD_WARN_BEFORE_SAVING_PUBLICLY(module_name, "project/shouldWarnBeforeSavingPublicly");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "project/preferredScoreCreationMode");
static const Settings::Key MIGRATION_OPTIONS(module_name, "project/migration");
static const Settings::Key AUTOSAVE_ENABLED_KEY(module_name, "project/autoSaveEnabled");
static const Settings::Key AUTOSAVE_INTERVAL_KEY(module_name, "project/autoSaveInterval");
static const Settings::Key SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT(module_name, "project/shouldDestinationFolderBeOpenedOnExport");
static const Settings::Key OPEN_DETAILED_PROJECT_UPLOADED_DIALOG(module_name, "project/openDetailedProjectUploadedDialog");
static const Settings::Key OPEN_AUDIO_GENERATION_SETTINGS(module_name, "project/openAudioGenerationSettings");
static const Settings::Key GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY(module_name, "project/generateAudioTimePeriodType");
static const Settings::Key NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY(module_name, "project/numberOfSavesToGenerateAudio");

const QString ProjectConfiguration::DEFAULT_FILE_SUFFIX(".mscz");

void ProjectConfiguration::init()
{
    settings()->setDefaultValue(USER_TEMPLATES_PATH, Val(globalConfiguration()->userDataPath() + "/Templates"));
    settings()->valueChanged(USER_TEMPLATES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userTemplatesPathChanged.send(val.toPath());
    });

    settings()->setDefaultValue(DEFAULT_PROJECTS_PATH, Val(globalConfiguration()->userDataPath() + "/Scores"));

    settings()->valueChanged(USER_PROJECTS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userScoresPathChanged.send(val.toPath());
    });

    settings()->valueChanged(RECENT_PROJECTS_PATHS).onReceive(nullptr, [this](const Val& val) {
        io::paths_t paths = parseRecentProjectsPaths(val);
        m_recentProjectPathsChanged.send(paths);
    });

    Val preferredScoreCreationMode = Val(PreferredScoreCreationMode::FromInstruments);
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);

    settings()->setDefaultValue(SHOULD_ASK_SAVE_LOCATION_TYPE, Val(true));
    settings()->setDefaultValue(LAST_USED_SAVE_LOCATION_TYPE, Val(SaveLocationType::Undefined));

    settings()->setDefaultValue(SHOULD_WARN_BEFORE_SAVING_PUBLICLY, Val(true));

    settings()->setDefaultValue(AUTOSAVE_ENABLED_KEY, Val(true));
    settings()->valueChanged(AUTOSAVE_ENABLED_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveEnabledChanged.send(val.toBool());
    });

    settings()->setDefaultValue(AUTOSAVE_INTERVAL_KEY, Val(3));
    settings()->valueChanged(AUTOSAVE_INTERVAL_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveIntervalChanged.send(val.toInt());
    });

    settings()->setDefaultValue(SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT, Val(false));
    settings()->setDefaultValue(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG, Val(true));
    settings()->setDefaultValue(OPEN_AUDIO_GENERATION_SETTINGS, Val(true));
    settings()->setDefaultValue(GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY, Val(static_cast<int>(GenerateAudioTimePeriodType::Never)));
    settings()->setDefaultValue(NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY, Val(10));

    fileSystem()->makePath(userTemplatesPath());
    fileSystem()->makePath(defaultProjectsPath());
    fileSystem()->makePath(cloudProjectsPath());
}

io::paths_t ProjectConfiguration::recentProjectPaths() const
{
    TRACEFUNC;

    io::paths_t allPaths = parseRecentProjectsPaths(settings()->value(RECENT_PROJECTS_PATHS));
    io::paths_t actualPaths;

    for (const io::path_t& path: allPaths) {
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

void ProjectConfiguration::setRecentProjectPaths(const io::paths_t& recentScorePaths)
{
    TRACEFUNC;

    QJsonArray jsonArray;
    for (const io::path_t& path : recentScorePaths) {
        jsonArray << path.toQString();
    }

    QJsonDocument jsonDoc(jsonArray);

    Val value(jsonDoc.toJson(QJsonDocument::Compact).constData());
    settings()->setSharedValue(RECENT_PROJECTS_PATHS, value);
}

async::Channel<io::paths_t> ProjectConfiguration::recentProjectPathsChanged() const
{
    return m_recentProjectPathsChanged;
}

io::paths_t ProjectConfiguration::parseRecentProjectsPaths(const Val& value) const
{
    TRACEFUNC;
    io::paths_t result;

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

io::path_t ProjectConfiguration::myFirstProjectPath() const
{
    return appTemplatesPath() + "/My_First_Score.mscx";
}

io::path_t ProjectConfiguration::appTemplatesPath() const
{
    return globalConfiguration()->appDataPath() + "/templates";
}

io::paths_t ProjectConfiguration::availableTemplateDirs() const
{
    io::paths_t dirs;

    io::path_t defaultTemplatesPath = this->appTemplatesPath();
    dirs.push_back(defaultTemplatesPath);

    io::path_t userTemplatesPath = this->userTemplatesPath();
    if (!userTemplatesPath.empty() && userTemplatesPath != defaultTemplatesPath) {
        dirs.push_back(userTemplatesPath);
    }

    return dirs;
}

io::path_t ProjectConfiguration::templateCategoriesJsonPath(const io::path_t& templatesDir) const
{
    return templatesDir + "/categories.json";
}

io::path_t ProjectConfiguration::userTemplatesPath() const
{
    return settings()->value(USER_TEMPLATES_PATH).toPath();
}

void ProjectConfiguration::setUserTemplatesPath(const io::path_t& path)
{
    settings()->setSharedValue(USER_TEMPLATES_PATH, Val(path));
}

async::Channel<io::path_t> ProjectConfiguration::userTemplatesPathChanged() const
{
    return m_userTemplatesPathChanged;
}

io::path_t ProjectConfiguration::defaultProjectsPath() const
{
    return settings()->value(DEFAULT_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setDefaultProjectsPath(const io::path_t& path)
{
    settings()->setSharedValue(DEFAULT_PROJECTS_PATH, Val(path));
}

io::path_t ProjectConfiguration::lastOpenedProjectsPath() const
{
    return settings()->value(LAST_OPENED_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setLastOpenedProjectsPath(const io::path_t& path)
{
    settings()->setSharedValue(LAST_OPENED_PROJECTS_PATH, Val(path));
}

io::path_t ProjectConfiguration::lastSavedProjectsPath() const
{
    return settings()->value(LAST_SAVED_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setLastSavedProjectsPath(const io::path_t& path)
{
    settings()->setSharedValue(LAST_SAVED_PROJECTS_PATH, Val(path));
}

io::path_t ProjectConfiguration::userProjectsPath() const
{
    return settings()->value(USER_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setUserProjectsPath(const io::path_t& path)
{
    settings()->setSharedValue(USER_PROJECTS_PATH, Val(path));
}

async::Channel<io::path_t> ProjectConfiguration::userProjectsPathChanged() const
{
    return m_userScoresPathChanged;
}

io::path_t ProjectConfiguration::cloudProjectPath(const io::path_t& projectName) const
{
    return cloudProjectsPath() + '/' + projectName + '.' + engraving::MSCZ;
}

bool ProjectConfiguration::isCloudProject(const io::path_t& projectPath) const
{
    return io::dirpath(projectPath) == cloudProjectsPath();
}

io::path_t ProjectConfiguration::cloudProjectsPath() const
{
    return globalConfiguration()->userAppDataPath() + "/cloud_projects";
}

io::path_t ProjectConfiguration::defaultSavingFilePath(INotationProjectPtr project, const QString& filenameAddition,
                                                       const QString& suffix) const
{
    io::path_t folderPath;
    io::path_t filename;
    QString theSuffix = suffix;

    io::path_t projectPath = project->path();

    if (project->isNewlyCreated()) {
        if (io::isAbsolute(projectPath)) {
            folderPath = io::dirpath(projectPath);
        }

        filename = io::filename(projectPath, false);
    } else if (project->isCloudProject()) {
        // TODO(save-to-cloud)
    } else {
        projectPath = engraving::containerPath(projectPath);
        folderPath = io::dirpath(projectPath);
        filename = io::filename(projectPath, false);

        if (theSuffix.isEmpty()) {
            theSuffix = QString::fromStdString(io::suffix(projectPath));
        }
    }

    if (folderPath.empty()) {
        folderPath = userProjectsPath();
    }

    if (folderPath.empty()) {
        folderPath = lastSavedProjectsPath();
    }

    if (folderPath.empty()) {
        folderPath = defaultProjectsPath();
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

bool ProjectConfiguration::shouldWarnBeforeSavingPublicly() const
{
    return settings()->value(SHOULD_WARN_BEFORE_SAVING_PUBLICLY).toBool();
}

void ProjectConfiguration::setShouldWarnBeforeSavingPublicly(bool shouldWarn)
{
    settings()->setSharedValue(SHOULD_WARN_BEFORE_SAVING_PUBLICLY, Val(shouldWarn));
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

io::path_t ProjectConfiguration::newProjectTemporaryPath() const
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

QUrl ProjectConfiguration::scoreManagerUrl() const
{
    return cloudConfiguration()->scoreManagerUrl();
}

bool ProjectConfiguration::openDetailedProjectUploadedDialog() const
{
    return settings()->value(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG).toBool();
}

void ProjectConfiguration::setOpenDetailedProjectUploadedDialog(bool show)
{
    settings()->setSharedValue(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG, Val(show));
}

bool ProjectConfiguration::openAudioGenerationSettings() const
{
    return settings()->value(OPEN_AUDIO_GENERATION_SETTINGS).toBool();
}

void ProjectConfiguration::setOpenAudioGenerationSettings(bool open)
{
    settings()->setSharedValue(OPEN_AUDIO_GENERATION_SETTINGS, Val(open));
}

GenerateAudioTimePeriodType ProjectConfiguration::generateAudioTimePeriodType() const
{
    return static_cast<GenerateAudioTimePeriodType>(settings()->value(GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY).toInt());
}

void ProjectConfiguration::setGenerateAudioTimePeriodType(GenerateAudioTimePeriodType type)
{
    settings()->setSharedValue(GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY, Val(static_cast<int>(type)));
}

int ProjectConfiguration::numberOfSavesToGenerateAudio() const
{
    return settings()->value(NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY).toInt();
}

void ProjectConfiguration::setNumberOfSavesToGenerateAudio(int number)
{
    settings()->setSharedValue(NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY, Val(number));
}

io::path_t ProjectConfiguration::temporaryMp3FilePathTemplate() const
{
    return globalConfiguration()->userAppDataPath() + "/audioFile_XXXXXX.mp3";
}

io::path_t ProjectConfiguration::projectBackupPath(const io::path_t& projectPath) const
{
    io::path_t projectDir = io::absoluteDirpath(projectPath);
    io::path_t projectName = io::filename(projectPath);

    return projectDir + "/.mscbackup/." + projectName + "~";
}
