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
#include "appshellconfiguration.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "settings.h"

#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace muse;
using namespace mu;
using namespace mu::appshell;
using namespace mu::notation;

static const std::string module_name("appshell");

static const Settings::Key HAS_COMPLETED_FIRST_LAUNCH_SETUP(module_name, "application/hasCompletedFirstLaunchSetup");

static const Settings::Key WELCOME_DIALOG_SHOW_ON_STARTUP_KEY(module_name, "application/welcomeDialogShowOnStartup");
static const Settings::Key WELCOME_DIALOG_LAST_SHOWN_VERSION_KEY(module_name, "application/welcomeDialogLastShownVersion");
static const Settings::Key WELCOME_DIALOG_LAST_SHOWN_INDEX(module_name, "application/welcomeDialogLastShownIndex");

static const Settings::Key STARTUP_MODE_TYPE(module_name, "application/startup/modeStart");
static const Settings::Key STARTUP_SCORE_PATH(module_name, "application/startup/startScore");

static const std::string MUSESCORE_ONLINE_HANDBOOK_URL("https://handbook.musescore.org");

static const std::string MUSESCORE_ASK_FOR_HELP_URL_PATH("/redirect/post/question");
static const std::string MUSESCORE_FORUM_URL_PATH("/forum");
static const std::string MUSESCORE_CONTRIBUTE_URL_PATH("/contribute");
static const std::string MUSEHUB_FREE_MUSE_SOUNDS_URL("https://www.musehub.com/free-musesounds"
                                                      "?utm_source=mss-app-dialog-ms-free"
                                                      "&utm_medium=mss-app-dialog-ms-free"
                                                      "&utm_campaign=mss-app-dialog-ms-free"
                                                      "&utm_id=mss-app-dialog");
static const std::string MUSICXML_URL("https://w3.org");
static const std::string MUSICXML_LICENSE_URL(MUSICXML_URL + "/community/about/process/final/");
static const std::string MUSICXML_LICENSE_DEED_URL(MUSICXML_URL + "/community/about/process/fsa-deed/");

static const std::string UTM_MEDIUM_MENU("menu");

static const QString NOTATION_NAVIGATOR_VISIBLE_KEY("showNavigator");
static const Settings::Key SPLASH_SCREEN_VISIBLE_KEY(module_name, "ui/application/startup/showSplashScreen");

static const muse::io::path_t SESSION_FILE("/session.json");
static const std::string SESSION_RESOURCE_NAME("SESSION");

void AppShellConfiguration::init()
{
    settings()->setDefaultValue(HAS_COMPLETED_FIRST_LAUNCH_SETUP, Val(false));

    settings()->setDefaultValue(WELCOME_DIALOG_SHOW_ON_STARTUP_KEY, Val(true));
    settings()->valueChanged(WELCOME_DIALOG_SHOW_ON_STARTUP_KEY).onReceive(this, [this](const Val&) {
        m_welcomeDialogShowOnStartupChanged.notify();
    });

    settings()->setDefaultValue(WELCOME_DIALOG_LAST_SHOWN_VERSION_KEY, Val("0.0.0"));
    settings()->setDefaultValue(WELCOME_DIALOG_LAST_SHOWN_INDEX, Val(-1));

    settings()->setDefaultValue(STARTUP_MODE_TYPE, Val(StartupModeType::StartEmpty));
    settings()->valueChanged(STARTUP_MODE_TYPE).onReceive(this, [this](const Val&) {
        m_startupModeTypeChanged.notify();
    });

    settings()->setDefaultValue(STARTUP_SCORE_PATH, Val(projectConfiguration()->myFirstProjectPath().toStdString()));
    settings()->valueChanged(STARTUP_SCORE_PATH).onReceive(this, [this](const Val&) {
        m_startupScorePathChanged.notify();
    });

    fileSystem()->makePath(sessionDataPath());
}

bool AppShellConfiguration::hasCompletedFirstLaunchSetup() const
{
#ifdef Q_OS_WASM
    return true;
#else
    return settings()->value(HAS_COMPLETED_FIRST_LAUNCH_SETUP).toBool();
#endif
}

void AppShellConfiguration::setHasCompletedFirstLaunchSetup(bool has)
{
    settings()->setSharedValue(HAS_COMPLETED_FIRST_LAUNCH_SETUP, Val(has));
}

bool AppShellConfiguration::welcomeDialogShowOnStartup() const
{
    return settings()->value(WELCOME_DIALOG_SHOW_ON_STARTUP_KEY).toBool();
}

void AppShellConfiguration::setWelcomeDialogShowOnStartup(bool show)
{
    settings()->setSharedValue(WELCOME_DIALOG_SHOW_ON_STARTUP_KEY, Val(show));
}

async::Notification AppShellConfiguration::welcomeDialogShowOnStartupChanged() const
{
    return m_welcomeDialogShowOnStartupChanged;
}

std::string AppShellConfiguration::welcomeDialogLastShownVersion() const
{
    return settings()->value(WELCOME_DIALOG_LAST_SHOWN_VERSION_KEY).toString();
}

void AppShellConfiguration::setWelcomeDialogLastShownVersion(const std::string& version)
{
    settings()->setSharedValue(WELCOME_DIALOG_LAST_SHOWN_VERSION_KEY, Val(version));
}

int AppShellConfiguration::welcomeDialogLastShownIndex() const
{
    return settings()->value(WELCOME_DIALOG_LAST_SHOWN_INDEX).toInt();
}

void AppShellConfiguration::setWelcomeDialogLastShownIndex(int index)
{
    settings()->setSharedValue(WELCOME_DIALOG_LAST_SHOWN_INDEX, Val(index));
}

StartupModeType AppShellConfiguration::startupModeType() const
{
    return settings()->value(STARTUP_MODE_TYPE).toEnum<StartupModeType>();
}

void AppShellConfiguration::setStartupModeType(StartupModeType type)
{
    settings()->setSharedValue(STARTUP_MODE_TYPE, Val(type));
}

async::Notification AppShellConfiguration::startupModeTypeChanged() const
{
    return m_startupModeTypeChanged;
}

muse::io::path_t AppShellConfiguration::startupScorePath() const
{
    return settings()->value(STARTUP_SCORE_PATH).toString();
}

void AppShellConfiguration::setStartupScorePath(const muse::io::path_t& scorePath)
{
    settings()->setSharedValue(STARTUP_SCORE_PATH, Val(scorePath.toStdString()));
}

async::Notification AppShellConfiguration::startupScorePathChanged() const
{
    return m_startupScorePathChanged;
}

muse::io::path_t AppShellConfiguration::userDataPath() const
{
    return globalConfiguration()->userDataPath();
}

std::string AppShellConfiguration::handbookUrl() const
{
    std::string utm = utmParameters(UTM_MEDIUM_MENU);
    std::string languageCode = currentLanguageCode();

    QStringList params = {
        "tag=handbook",
        "locale=" + QString::fromStdString(languageCode),
        QString::fromStdString(utm)
    };

    return MUSESCORE_ONLINE_HANDBOOK_URL + "?" + params.join("&").toStdString();
}

std::string AppShellConfiguration::askForHelpUrl() const
{
    std::string languageCode = currentLanguageCode();

    QStringList params = {
        "locale=" + QString::fromStdString(languageCode)
    };

    return museScoreUrl() + MUSESCORE_ASK_FOR_HELP_URL_PATH + "?" + params.join("&").toStdString();
}

std::string AppShellConfiguration::museScoreUrl() const
{
    return globalConfiguration()->museScoreUrl();
}

std::string AppShellConfiguration::museScoreForumUrl() const
{
    return museScoreUrl() + MUSESCORE_FORUM_URL_PATH;
}

std::string AppShellConfiguration::museScoreContributionUrl() const
{
    return museScoreUrl() + MUSESCORE_CONTRIBUTE_URL_PATH;
}

std::string AppShellConfiguration::museHubFreeMuseSoundsUrl() const
{
    return MUSEHUB_FREE_MUSE_SOUNDS_URL;
}

std::string AppShellConfiguration::musicXMLLicenseUrl() const
{
    return MUSICXML_LICENSE_URL;
}

std::string AppShellConfiguration::musicXMLLicenseDeedUrl() const
{
    return MUSICXML_LICENSE_DEED_URL;
}

std::string AppShellConfiguration::museScoreVersion() const
{
    return String(application()->version().toString() + u"." + application()->build()).toStdString();
}

std::string AppShellConfiguration::museScoreRevision() const
{
    return application()->revision().toStdString();
}

bool AppShellConfiguration::isNotationNavigatorVisible() const
{
    return uiConfiguration()->isVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, false);
}

void AppShellConfiguration::setIsNotationNavigatorVisible(bool visible) const
{
    uiConfiguration()->setIsVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, visible);
}

muse::async::Notification AppShellConfiguration::isNotationNavigatorVisibleChanged() const
{
    return uiConfiguration()->isVisibleChanged(NOTATION_NAVIGATOR_VISIBLE_KEY);
}

bool AppShellConfiguration::needShowSplashScreen() const
{
    return settings()->value(SPLASH_SCREEN_VISIBLE_KEY).toBool();
}

void AppShellConfiguration::setNeedShowSplashScreen(bool show)
{
    settings()->setSharedValue(SPLASH_SCREEN_VISIBLE_KEY, Val(show));
}

const QString& AppShellConfiguration::preferencesDialogLastOpenedPageId() const
{
    return m_preferencesDialogCurrentPageId;
}

void AppShellConfiguration::setPreferencesDialogLastOpenedPageId(const QString& lastOpenedPageId)
{
    m_preferencesDialogCurrentPageId = lastOpenedPageId;
}

void AppShellConfiguration::startEditSettings()
{
    settings()->beginTransaction();
}

void AppShellConfiguration::applySettings()
{
    settings()->commitTransaction();
}

void AppShellConfiguration::rollbackSettings()
{
    settings()->rollbackTransaction();
}

void AppShellConfiguration::revertToFactorySettings(bool keepDefaultSettings, bool notifyAboutChanges, bool notifyOtherInstances) const
{
    settings()->reset(keepDefaultSettings, notifyAboutChanges, notifyOtherInstances);
}

muse::io::paths_t AppShellConfiguration::sessionProjectsPaths() const
{
    RetVal<ByteArray> retVal = readSessionState();
    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
        return {};
    }

    return parseSessionProjectsPaths(retVal.val.toQByteArrayNoCopy());
}

muse::Ret AppShellConfiguration::setSessionProjectsPaths(const muse::io::paths_t& paths)
{
    QJsonArray jsonArray;
    for (const muse::io::path_t& path : paths) {
        jsonArray << QJsonValue(path.toQString());
    }

    QByteArray data = QJsonDocument(jsonArray).toJson();
    return writeSessionState(data);
}

std::string AppShellConfiguration::utmParameters(const std::string& utmMedium) const
{
    return "utm_source=desktop&utm_medium=" + utmMedium
           + "&utm_content=" + application()->revision().toStdString()
           + "&utm_campaign=MuseScore" + application()->version().toStdString();
}

std::string AppShellConfiguration::currentLanguageCode() const
{
    QString languageCode = languagesConfiguration()->currentLanguageCode().val;
    QLocale locale(languageCode);

    return locale.bcp47Name().toStdString();
}

muse::io::path_t AppShellConfiguration::sessionDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/session";
}

muse::io::path_t AppShellConfiguration::sessionFilePath() const
{
    return sessionDataPath() + SESSION_FILE;
}

RetVal<muse::ByteArray> AppShellConfiguration::readSessionState() const
{
    muse::mi::ReadResourceLockGuard lock_guard(multiInstancesProvider.get(), SESSION_RESOURCE_NAME);
    return fileSystem()->readFile(sessionFilePath());
}

muse::Ret AppShellConfiguration::writeSessionState(const QByteArray& data)
{
    muse::mi::WriteResourceLockGuard lock_guard(multiInstancesProvider.get(), SESSION_RESOURCE_NAME);
    return fileSystem()->writeFile(sessionFilePath(), ByteArray::fromQByteArrayNoCopy(data));
}

muse::io::paths_t AppShellConfiguration::parseSessionProjectsPaths(const QByteArray& json) const
{
    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isArray()) {
        LOGE() << "failed parse, err: " << err.errorString();
        return {};
    }

    io::paths_t result;
    const QVariantList pathsList = jsodDoc.array().toVariantList();
    for (const QVariant& pathVal : pathsList) {
        muse::io::path_t path = pathVal.toString().toStdString();
        if (!path.empty()) {
            result.push_back(path);
        }
    }

    return result;
}
