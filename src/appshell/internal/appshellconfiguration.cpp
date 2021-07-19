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
#include "appshellconfiguration.h"

#include "config.h"
#include "settings.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::framework;

static const std::string module_name("appshell");

static const Settings::Key STARTUP_SESSION_TYPE(module_name, "application/startup/sessionStart");
static const Settings::Key STARTUP_SCORE_PATH(module_name, "application/startup/startScore");

static const Settings::Key CHECK_FOR_UPDATE_KEY(module_name, "application/checkForUpdate");

static const std::string ONLINE_HANDBOOK_URL("https://musescore.org/redirect/help?tag=handbook&locale=");
static const std::string ASK_FOR_HELP_URL("https://musescore.org/redirect/post/question?locale=");
static const std::string BUG_REPORT_URL("https://musescore.org/redirect/post/bug-report?locale=");
static const std::string LEAVE_FEEDBACK_URL("https://musescore.com/content/editor-feedback?");
static const std::string MUSESCORE_URL("https://www.musescore.org/");
static const std::string MUSICXML_LICENSE_URL("https://www.w3.org/community/about/process/final/");
static const std::string MUSICXML_LICENSE_DEED_URL("https://www.w3.org/community/about/process/fsa-deed/");

static const std::string UTM_MEDIUM_MENU("menu");

static const QString NOTATION_NAVIGATOR_VISIBLE_KEY("showNavigator");
static const Settings::Key SPLASH_SCREEN_VISIBLE_KEY(module_name, "ui/application/startup/showSplashScreen");
static const Settings::Key TOURS_VISIBLE_KEY(module_name, "ui/application/startup/showTours");

void AppShellConfiguration::init()
{
    settings()->setDefaultValue(STARTUP_SESSION_TYPE, Val(static_cast<int>(StartupSessionType::StartEmpty)));
    settings()->setDefaultValue(STARTUP_SCORE_PATH, Val(userScoresConfiguration()->myFirstScorePath().toStdString()));

    settings()->setDefaultValue(CHECK_FOR_UPDATE_KEY, Val(isAppUpdatable()));
}

StartupSessionType AppShellConfiguration::startupSessionType() const
{
    return static_cast<StartupSessionType>(settings()->value(STARTUP_SESSION_TYPE).toInt());
}

void AppShellConfiguration::setStartupSessionType(StartupSessionType type)
{
    settings()->setSharedValue(STARTUP_SESSION_TYPE, Val(static_cast<int>(type)));
}

mu::io::path AppShellConfiguration::startupScorePath() const
{
    return settings()->value(STARTUP_SCORE_PATH).toString();
}

void AppShellConfiguration::setStartupScorePath(const io::path& scorePath)
{
    settings()->setSharedValue(STARTUP_SCORE_PATH, Val(scorePath.toStdString()));
}

bool AppShellConfiguration::isAppUpdatable() const
{
#ifdef APP_UPDATABLE
    return true;
#else
    return false;
#endif
}

bool AppShellConfiguration::needCheckForUpdate() const
{
    return settings()->value(CHECK_FOR_UPDATE_KEY).toBool();
}

void AppShellConfiguration::setNeedCheckForUpdate(bool needCheck)
{
    settings()->setSharedValue(CHECK_FOR_UPDATE_KEY, Val(needCheck));
}

std::string AppShellConfiguration::handbookUrl() const
{
    std::string utm = utmParameters(UTM_MEDIUM_MENU);
    std::string languageCode = currentLanguageCode();

    return ONLINE_HANDBOOK_URL + languageCode + "&" + utm;
}

std::string AppShellConfiguration::askForHelpUrl() const
{
    std::string languageCode = currentLanguageCode();
    return ASK_FOR_HELP_URL + languageCode;
}

std::string AppShellConfiguration::bugReportUrl() const
{
    std::string utm = utmParameters(UTM_MEDIUM_MENU);
    std::string languageCode = currentLanguageCode();

    return BUG_REPORT_URL + languageCode + "&" + utm + "&" + sha();
}

std::string AppShellConfiguration::leaveFeedbackUrl() const
{
    std::string utm = utmParameters(UTM_MEDIUM_MENU);

    return LEAVE_FEEDBACK_URL + utm;
}

std::string AppShellConfiguration::museScoreUrl() const
{
    std::string languageCode = currentLanguageCode();
    return MUSESCORE_URL + languageCode;
}

std::string AppShellConfiguration::museScoreForumUrl() const
{
    return MUSESCORE_URL + "forum";
}

std::string AppShellConfiguration::museScoreContributionUrl() const
{
    return MUSESCORE_URL + "contribute";
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
    return VERSION;
}

std::string AppShellConfiguration::museScoreRevision() const
{
    return MUSESCORE_REVISION;
}

mu::ValCh<mu::io::paths> AppShellConfiguration::recentScorePaths() const
{
    return userScoresConfiguration()->recentScorePaths();
}

bool AppShellConfiguration::isNotationNavigatorVisible() const
{
    return uiConfiguration()->isVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, false);
}

void AppShellConfiguration::setIsNotationNavigatorVisible(bool visible) const
{
    uiConfiguration()->setIsVisible(NOTATION_NAVIGATOR_VISIBLE_KEY, visible);
}

mu::async::Notification AppShellConfiguration::isNotationNavigatorVisibleChanged() const
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

bool AppShellConfiguration::needShowTours() const
{
    return settings()->value(TOURS_VISIBLE_KEY).toBool();
}

void AppShellConfiguration::setNeedShowTours(bool show)
{
    settings()->setSharedValue(TOURS_VISIBLE_KEY, Val(show));
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

void AppShellConfiguration::revertToFactorySettings(bool keepDefaultSettings) const
{
    settings()->reset(keepDefaultSettings);
}

std::string AppShellConfiguration::utmParameters(const std::string& utmMedium) const
{
    return "utm_source=desktop&utm_medium=" + utmMedium
           + "&utm_content=" + MUSESCORE_REVISION
           + "&utm_campaign=MuseScore" + VERSION;
}

std::string AppShellConfiguration::sha() const
{
    return "sha=" + notationConfiguration()->notationRevision();
}

std::string AppShellConfiguration::currentLanguageCode() const
{
    QString languageCode = languagesConfiguration()->currentLanguageCode().val;
    QLocale locale(languageCode);

    return locale.bcp47Name().toStdString();
}
