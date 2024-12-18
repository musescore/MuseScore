/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/infrastructure/mscio.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;

static const std::string module_name("project");

static const Settings::Key COMPAT_RECENT_FILES_DATA(module_name, "project/recentList");
static const Settings::Key USER_TEMPLATES_PATH(module_name, "application/paths/myTemplates");
static const Settings::Key LAST_OPENED_PROJECTS_PATH(module_name, "application/paths/lastOpenedProjectsPath");
static const Settings::Key LAST_SAVED_PROJECTS_PATH(module_name, "application/paths/lastSavedProjectsPath");
static const Settings::Key USER_PROJECTS_PATH(module_name, "application/paths/myScores");
static const Settings::Key SHOULD_ASK_SAVE_LOCATION_TYPE(module_name, "project/shouldAskSaveLocationType");
static const Settings::Key LAST_USED_SAVE_LOCATION_TYPE(module_name, "project/lastUsedSaveLocationType");
static const Settings::Key SHOULD_WARN_BEFORE_PUBLISH(module_name, "project/shouldWarnBeforePublish");
static const Settings::Key SHOULD_WARN_BEFORE_SAVING_PUBLICLY_TO_CLOUD(module_name, "project/shouldWarnBeforeSavingPubliclyToCloud");
static const Settings::Key HOME_SCORES_PAGE_VIEW_TYPE(module_name, "project/homeScoresPageViewType");
static const Settings::Key PREFERRED_SCORE_CREATION_MODE_KEY(module_name, "project/preferredScoreCreationMode");
static const Settings::Key MIGRATION_OPTIONS(module_name, "project/migration");
static const Settings::Key AUTOSAVE_ENABLED_KEY(module_name, "project/autoSaveEnabled");
static const Settings::Key AUTOSAVE_INTERVAL_KEY(module_name, "project/autoSaveInterval");
static const Settings::Key ALSO_SHARE_AUDIO_COM_AFTER_PUBLISH(module_name, "project/alsoShareAudioCom");
static const Settings::Key SHOW_ALSO_SHARE_AUDIO_COM_DIALOG(module_name, "project/showAlsoShareAudioComDialog");
static const Settings::Key HAS_ASKED_ALSO_SHARE_AUDIO_COM(module_name, "project/hasAskedAlsoShareAudioCom");
static const Settings::Key SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT(module_name, "project/shouldDestinationFolderBeOpenedOnExport");
static const Settings::Key OPEN_DETAILED_PROJECT_UPLOADED_DIALOG(module_name, "project/openDetailedProjectUploadedDialog");
static const Settings::Key HAS_ASKED_AUDIO_GENERATION_SETTINGS(module_name, "project/hasAskedAudioGenerationSettings");
static const Settings::Key GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY(module_name, "project/generateAudioTimePeriodType");
static const Settings::Key NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY(module_name, "project/numberOfSavesToGenerateAudio");
static const Settings::Key SHOW_CLOUD_IS_NOT_AVAILABLE_WARNING(module_name, "project/showCloudIsNotAvailableWarning");
static const Settings::Key DISABLE_VERSION_CHECKING(module_name, "project/disableVersionChecking");
static const Settings::Key CREATE_BACKUP_BEFORE_SAVING(module_name, "project/createBackupBeforeSaving");

static const std::string DEFAULT_FILE_SUFFIX(".mscz");
static const std::string DEFAULT_FILE_FILTER("*.mscz");

void ProjectConfiguration::init()
{
    settings()->setDefaultValue(USER_TEMPLATES_PATH, Val(globalConfiguration()->userDataPath() + "/Templates"));
    settings()->valueChanged(USER_TEMPLATES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userTemplatesPathChanged.send(val.toPath());
    });

    settings()->setDefaultValue(USER_PROJECTS_PATH, Val(globalConfiguration()->userDataPath() + "/Scores"));

    settings()->valueChanged(USER_PROJECTS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userScoresPathChanged.send(val.toPath());
    });

    Val preferredScoreCreationMode = Val(PreferredScoreCreationMode::FromInstruments);
    settings()->setDefaultValue(PREFERRED_SCORE_CREATION_MODE_KEY, preferredScoreCreationMode);

    settings()->setDefaultValue(HOME_SCORES_PAGE_VIEW_TYPE, Val(HomeScoresPageViewType::Grid));

    settings()->setDefaultValue(SHOULD_ASK_SAVE_LOCATION_TYPE, Val(true));
    settings()->setDefaultValue(LAST_USED_SAVE_LOCATION_TYPE, Val(SaveLocationType::Undefined));

    settings()->setDefaultValue(SHOULD_WARN_BEFORE_PUBLISH, Val(true));
    settings()->setDefaultValue(SHOULD_WARN_BEFORE_SAVING_PUBLICLY_TO_CLOUD, Val(true));

    settings()->setDefaultValue(AUTOSAVE_ENABLED_KEY, Val(true));
    settings()->valueChanged(AUTOSAVE_ENABLED_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveEnabledChanged.send(val.toBool());
    });

    settings()->setDefaultValue(AUTOSAVE_INTERVAL_KEY, Val(5));
    settings()->valueChanged(AUTOSAVE_INTERVAL_KEY).onReceive(nullptr, [this](const Val& val) {
        m_autoSaveIntervalChanged.send(val.toInt());
    });

    settings()->setDefaultValue(ALSO_SHARE_AUDIO_COM_AFTER_PUBLISH, Val(true));
    settings()->valueChanged(ALSO_SHARE_AUDIO_COM_AFTER_PUBLISH).onReceive(nullptr, [this](const Val& val) {
        m_alsoShareAudioComChanged.send(val.toBool());
    });

    settings()->setDefaultValue(SHOW_ALSO_SHARE_AUDIO_COM_DIALOG, Val(true));
    settings()->setDefaultValue(HAS_ASKED_ALSO_SHARE_AUDIO_COM, Val(false));

    settings()->setDefaultValue(SHOULD_DESTINATION_FOLDER_BE_OPENED_ON_EXPORT, Val(false));
    settings()->setDefaultValue(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG, Val(true));
    settings()->setDefaultValue(HAS_ASKED_AUDIO_GENERATION_SETTINGS, Val(false));
    settings()->setDefaultValue(GENERATE_AUDIO_TIME_PERIOD_TYPE_KEY, Val(static_cast<int>(GenerateAudioTimePeriodType::Never)));
    settings()->setDefaultValue(NUMBER_OF_SAVES_TO_GENERATE_AUDIO_KEY, Val(10));
    settings()->setDefaultValue(SHOW_CLOUD_IS_NOT_AVAILABLE_WARNING, Val(true));

    settings()->setDefaultValue(DISABLE_VERSION_CHECKING, Val(false));

    settings()->setDefaultValue(CREATE_BACKUP_BEFORE_SAVING, Val(true));
    settings()->setDescription(CREATE_BACKUP_BEFORE_SAVING, muse::trc("project",
                                                                      "Create backup of file on disk before saving new changes"));
    settings()->setCanBeManuallyEdited(CREATE_BACKUP_BEFORE_SAVING, true);

    if (!userTemplatesPath().empty()) {
        fileSystem()->makePath(userTemplatesPath());
    }

    if (!userProjectsPath().empty()) {
        fileSystem()->makePath(userProjectsPath());
    }

    fileSystem()->makePath(cloudProjectsPath());
}

muse::io::path_t ProjectConfiguration::recentFilesJsonPath() const
{
    return globalConfiguration()->userAppDataPath().appendingComponent("recent_files.json");
}

ByteArray ProjectConfiguration::compatRecentFilesData() const
{
    std::string data = settings()->value(COMPAT_RECENT_FILES_DATA).toString();

    return ByteArray(data.data(), data.size());
}

muse::io::path_t ProjectConfiguration::myFirstProjectPath() const
{
    return appTemplatesPath() + "/My_First_Score.mscx";
}

muse::io::path_t ProjectConfiguration::appTemplatesPath() const
{
    return globalConfiguration()->appDataPath() + "/templates";
}

io::paths_t ProjectConfiguration::availableTemplateDirs() const
{
    io::paths_t dirs;

    muse::io::path_t defaultTemplatesPath = this->appTemplatesPath();
    dirs.push_back(defaultTemplatesPath);

    muse::io::path_t userTemplatesPath = this->userTemplatesPath();
    if (!userTemplatesPath.empty() && userTemplatesPath != defaultTemplatesPath) {
        dirs.push_back(userTemplatesPath);
    }

    return dirs;
}

muse::io::path_t ProjectConfiguration::templateCategoriesJsonPath(const muse::io::path_t& templatesDir) const
{
    return templatesDir + "/categories.json";
}

muse::io::path_t ProjectConfiguration::userTemplatesPath() const
{
    return settings()->value(USER_TEMPLATES_PATH).toPath();
}

void ProjectConfiguration::setUserTemplatesPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(USER_TEMPLATES_PATH, Val(path));
}

muse::async::Channel<muse::io::path_t> ProjectConfiguration::userTemplatesPathChanged() const
{
    return m_userTemplatesPathChanged;
}

muse::io::path_t ProjectConfiguration::lastOpenedProjectsPath() const
{
    return settings()->value(LAST_OPENED_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setLastOpenedProjectsPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(LAST_OPENED_PROJECTS_PATH, Val(path));
}

muse::io::path_t ProjectConfiguration::lastSavedProjectsPath() const
{
    return settings()->value(LAST_SAVED_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setLastSavedProjectsPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(LAST_SAVED_PROJECTS_PATH, Val(path));
}

muse::io::path_t ProjectConfiguration::userProjectsPath() const
{
    return settings()->value(USER_PROJECTS_PATH).toPath();
}

void ProjectConfiguration::setUserProjectsPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(USER_PROJECTS_PATH, Val(path));
}

muse::async::Channel<muse::io::path_t> ProjectConfiguration::userProjectsPathChanged() const
{
    return m_userScoresPathChanged;
}

muse::io::path_t ProjectConfiguration::defaultUserProjectsPath() const
{
    return settings()->defaultValue(USER_PROJECTS_PATH).toPath();
}

bool ProjectConfiguration::shouldAskSaveLocationType() const
{
    return settings()->value(SHOULD_ASK_SAVE_LOCATION_TYPE).toBool();
}

void ProjectConfiguration::setShouldAskSaveLocationType(bool shouldAsk)
{
    settings()->setSharedValue(SHOULD_ASK_SAVE_LOCATION_TYPE, Val(shouldAsk));
}

muse::io::path_t ProjectConfiguration::legacyCloudProjectsPath() const
{
    return globalConfiguration()->userDataPath() + "/Cloud Scores";
}

muse::io::path_t ProjectConfiguration::cloudProjectsPath() const
{
    return globalConfiguration()->userAppDataPath() + "/cloud_scores";
}

bool ProjectConfiguration::isCloudProject(const muse::io::path_t& projectPath) const
{
    muse::io::path_t dirpath = io::dirpath(projectPath);
    return dirpath == legacyCloudProjectsPath() || dirpath == cloudProjectsPath();
}

bool ProjectConfiguration::isLegacyCloudProject(const muse::io::path_t& projectPath) const
{
    return io::dirpath(projectPath) == legacyCloudProjectsPath();
}

muse::io::path_t ProjectConfiguration::cloudProjectPath(int scoreId) const
{
    return cloudProjectsPath().appendingComponent(QString::number(scoreId)).appendingSuffix(DEFAULT_FILE_SUFFIX);
}

int ProjectConfiguration::cloudScoreIdFromPath(const muse::io::path_t& projectPath) const
{
    return io::filename(projectPath, false).toQString().toInt();
}

muse::io::path_t ProjectConfiguration::cloudProjectSavingPath(int scoreId) const
{
    if (scoreId != 0) {
        return cloudProjectPath(scoreId);
    }

    muse::io::path_t path;
    int counter = 0;

    do {
        path = cloudProjectsPath()
               .appendingComponent(QStringLiteral("not_uploaded_") + QString::number(counter++))
               .appendingSuffix(DEFAULT_FILE_SUFFIX);
    } while (fileSystem()->exists(path));

    return path;
}

muse::io::path_t ProjectConfiguration::defaultSavingFilePath(INotationProjectPtr project, const std::string& filenameAddition,
                                                             const std::string& suffix) const
{
    muse::io::path_t folderPath;
    muse::io::path_t filename;
    std::string theSuffix = suffix;

    muse::io::path_t projectPath = project->path();
    bool isLocalProject = !project->isCloudProject();

    if (isLocalProject) {
        if (project->isNewlyCreated()) {
            if (io::isAbsolute(projectPath)) {
                filename = io::filename(projectPath, false);
                folderPath = io::dirpath(projectPath);
            } else {
                filename = io::filename(projectPath, true);
            }
        } else {
            projectPath = engraving::containerPath(projectPath);
            folderPath = io::dirpath(projectPath);
            filename = io::filename(projectPath, false);

            if (theSuffix.empty()) {
                theSuffix = io::suffix(projectPath);
            }
        }
    }

    if (folderPath.empty()) {
        folderPath = userProjectsPath();
    }

    if (folderPath.empty()) {
        folderPath = lastSavedProjectsPath();
    }

    if (folderPath.empty()) {
        folderPath = defaultUserProjectsPath();
    }

    if (filename.empty()) {
        filename = project->metaInfo().title;
    }

    if (filename.empty()) {
        filename = muse::qtrc("project", "Untitled");
    }

    if (theSuffix.empty()) {
        theSuffix = DEFAULT_FILE_SUFFIX;
    }

    return folderPath
           .appendingComponent(filename + filenameAddition)
           .appendingSuffix(theSuffix);
}

SaveLocationType ProjectConfiguration::lastUsedSaveLocationType() const
{
    return settings()->value(LAST_USED_SAVE_LOCATION_TYPE).toEnum<SaveLocationType>();
}

void ProjectConfiguration::setLastUsedSaveLocationType(SaveLocationType type)
{
    settings()->setSharedValue(LAST_USED_SAVE_LOCATION_TYPE, Val(type));
}

bool ProjectConfiguration::shouldWarnBeforePublish() const
{
    return settings()->value(SHOULD_WARN_BEFORE_PUBLISH).toBool();
}

void ProjectConfiguration::setShouldWarnBeforePublish(bool shouldWarn)
{
    return settings()->setSharedValue(SHOULD_WARN_BEFORE_PUBLISH, Val(shouldWarn));
}

bool ProjectConfiguration::shouldWarnBeforeSavingPubliclyToCloud() const
{
    return settings()->value(SHOULD_WARN_BEFORE_SAVING_PUBLICLY_TO_CLOUD).toBool();
}

void ProjectConfiguration::setShouldWarnBeforeSavingPubliclyToCloud(bool shouldWarn)
{
    settings()->setSharedValue(SHOULD_WARN_BEFORE_SAVING_PUBLICLY_TO_CLOUD, Val(shouldWarn));
}

int ProjectConfiguration::homeScoresPageTabIndex() const
{
    return m_homeScoresPageTabIndex;
}

void ProjectConfiguration::setHomeScoresPageTabIndex(int index)
{
    m_homeScoresPageTabIndex = index;
}

IProjectConfiguration::HomeScoresPageViewType ProjectConfiguration::homeScoresPageViewType() const
{
    return settings()->value(HOME_SCORES_PAGE_VIEW_TYPE).toEnum<HomeScoresPageViewType>();
}

void ProjectConfiguration::setHomeScoresPageViewType(HomeScoresPageViewType type)
{
    // Intentionally not directly synced between instances
    // (it would be weird if you switch the view in one instance, and the others suddenly switch too)
    settings()->setLocalValue(HOME_SCORES_PAGE_VIEW_TYPE, Val(type));
}

QColor ProjectConfiguration::templatePreviewBackgroundColor() const
{
    return notationConfiguration()->backgroundColor();
}

muse::async::Notification ProjectConfiguration::templatePreviewBackgroundChanged() const
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

static MigrationType migrationTypeFromString(const QString& str)
{
    if (str == "Pre_3_6") {
        return MigrationType::Pre_3_6;
    }
    if (str == "Ver_3_6") {
        return MigrationType::Ver_3_6;
    }
    LOGE() << "Unknown migration type string: " << str;
    return MigrationType::Unknown;
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

    for (const auto val : array) {
        QJsonObject obj = val.toObject();

        auto migrationType = migrationTypeFromString(obj["migrationType"].toString());
        if (migrationType == MigrationType::Unknown) {
            continue;
        }

        QJsonObject optionsObj = obj["options"].toObject();

        MigrationOptions opt;
        opt.appVersion = optionsObj["appVersion"].toInt(0);
        opt.isApplyMigration = optionsObj["isApplyMigration"].toBool();
        opt.isAskAgain = optionsObj["isAskAgain"].toBool();
        opt.isApplyLeland = optionsObj["isApplyLeland"].toBool();
        opt.isApplyEdwin = optionsObj["isApplyEdwin"].toBool();
        opt.isRemapPercussion = optionsObj["isRemapPercussion"].toBool();

        m_migrationOptions[migrationType] = opt;

        if (migrationType == type) {
            result = opt;
        }
    }

    return result;
}

static QString migrationTypeToString(MigrationType type)
{
    switch (type) {
    case MigrationType::Pre_3_6:
        return QStringLiteral("Pre_3_6");
    case MigrationType::Ver_3_6:
        return QStringLiteral("Ver_3_6");
    case MigrationType::Unknown:
        break;
    }
    return QString();
}

void ProjectConfiguration::setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent)
{
    m_migrationOptions[type] = opt;

    if (!persistent) {
        return;
    }

    QJsonArray array;

    for (auto it = m_migrationOptions.cbegin(); it != m_migrationOptions.cend(); ++it) {
        const MigrationOptions& o = it->second;

        QJsonObject options;
        options["appVersion"] = o.appVersion;
        options["isApplyMigration"] = o.isApplyMigration;
        options["isAskAgain"] = o.isAskAgain;
        options["isApplyLeland"] = o.isApplyLeland;
        options["isApplyEdwin"] = o.isApplyEdwin;
        options["isRemapPercussion"] = o.isRemapPercussion;

        QJsonObject obj;
        obj["migrationType"] = migrationTypeToString(it->first);
        obj["options"] = options;

        array << obj;
    }

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

muse::async::Channel<bool> ProjectConfiguration::autoSaveEnabledChanged() const
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

muse::async::Channel<int> ProjectConfiguration::autoSaveIntervalChanged() const
{
    return m_autoSaveIntervalChanged;
}

bool ProjectConfiguration::alsoShareAudioCom() const
{
    return settings()->value(ALSO_SHARE_AUDIO_COM_AFTER_PUBLISH).toBool();
}

void ProjectConfiguration::setAlsoShareAudioCom(bool share)
{
    settings()->setSharedValue(ALSO_SHARE_AUDIO_COM_AFTER_PUBLISH, Val(share));
}

muse::async::Channel<bool> ProjectConfiguration::alsoShareAudioComChanged() const
{
    return m_alsoShareAudioComChanged;
}

bool ProjectConfiguration::showAlsoShareAudioComDialog() const
{
    return settings()->value(SHOW_ALSO_SHARE_AUDIO_COM_DIALOG).toBool();
}

void ProjectConfiguration::setShowAlsoShareAudioComDialog(bool show)
{
    settings()->setSharedValue(SHOW_ALSO_SHARE_AUDIO_COM_DIALOG, Val(show));
}

bool ProjectConfiguration::hasAskedAlsoShareAudioCom() const
{
    return settings()->value(HAS_ASKED_ALSO_SHARE_AUDIO_COM).toBool();
}

void ProjectConfiguration::setHasAskedAlsoShareAudioCom(bool has)
{
    settings()->setSharedValue(HAS_ASKED_ALSO_SHARE_AUDIO_COM, Val(has));
}

muse::io::path_t ProjectConfiguration::newProjectTemporaryPath() const
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

QUrl ProjectConfiguration::supportForumUrl() const
{
    if (languagesService()->currentLanguage().code.startsWith("en")) {
        // The English support forum
        return QUrl("https://musescore.org/forum/6");
    }

    // The general forum page, where the support forum is linked at the top
    // (except in English; there you have the Announcements forum)
    return QUrl("https://musescore.org/forum");
}

bool ProjectConfiguration::openDetailedProjectUploadedDialog() const
{
    return settings()->value(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG).toBool();
}

void ProjectConfiguration::setOpenDetailedProjectUploadedDialog(bool show)
{
    settings()->setSharedValue(OPEN_DETAILED_PROJECT_UPLOADED_DIALOG, Val(show));
}

bool ProjectConfiguration::hasAskedAudioGenerationSettings() const
{
    return settings()->value(HAS_ASKED_AUDIO_GENERATION_SETTINGS).toBool();
}

void ProjectConfiguration::setHasAskedAudioGenerationSettings(bool has)
{
    settings()->setSharedValue(HAS_ASKED_AUDIO_GENERATION_SETTINGS, Val(has));
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

muse::io::path_t ProjectConfiguration::temporaryMp3FilePathTemplate() const
{
    return globalConfiguration()->userAppDataPath() + "/audioFile_XXXXXX.mp3";
}

muse::io::path_t ProjectConfiguration::projectBackupPath(const muse::io::path_t& projectPath) const
{
    muse::io::path_t projectDir = io::absoluteDirpath(projectPath);
    muse::io::path_t projectName = io::filename(projectPath);

    return projectDir + "/.mscbackup/." + projectName + "~";
}

bool ProjectConfiguration::showCloudIsNotAvailableWarning() const
{
    return settings()->value(SHOW_CLOUD_IS_NOT_AVAILABLE_WARNING).toBool();
}

void ProjectConfiguration::setShowCloudIsNotAvailableWarning(bool show)
{
    settings()->setSharedValue(SHOW_CLOUD_IS_NOT_AVAILABLE_WARNING, Val(show));
}

bool ProjectConfiguration::createBackupBeforeSaving() const
{
    return settings()->value(CREATE_BACKUP_BEFORE_SAVING).toBool();
}

void ProjectConfiguration::setCreateBackupBeforeSaving(bool create)
{
    settings()->setSharedValue(CREATE_BACKUP_BEFORE_SAVING, Val(create));
}

bool ProjectConfiguration::disableVersionChecking() const
{
    return settings()->value(DISABLE_VERSION_CHECKING).toBool();
}

void ProjectConfiguration::setDisableVersionChecking(bool disable)
{
    settings()->setSharedValue(DISABLE_VERSION_CHECKING, Val(disable));
}
