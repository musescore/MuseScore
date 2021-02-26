//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "appshellconfiguration.h"

#include "config.h"
#include "settings.h"

static const std::string module_name("appshell");

static const std::string ONLINE_HANDBOOK_URL("https://musescore.org/redirect/help?tag=handbook&locale=");
static const std::string ASK_FOR_HELP_URL("https://musescore.org/redirect/post/question?locale=");
static const std::string BUG_REPORT_URL("https://musescore.org/redirect/post/bug-report?locale=");
static const std::string LEAVE_FEEDBACK_URL("https://musescore.com/content/editor-feedback?");

static const std::string UTM_MEDIUM_MENU("menu");
static const std::string SYSTEM_LANGUAGE("system");

static const Settings::Key NOTATION_NAVIGATOR_VISIBLE_KEY(module_name, "ui/application/startup/showNavigator");
static const Settings::Key NOTATION_STATUSBAR_VISIBLE_KEY(module_name, "ui/application/showStatusBar");

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::framework;

void AppShellConfiguration::init()
{
    settings()->setDefaultValue(NOTATION_NAVIGATOR_VISIBLE_KEY, Val(false));
    settings()->valueChanged(NOTATION_NAVIGATOR_VISIBLE_KEY).onReceive(nullptr, [this](const Val&) {
        m_notationNavigatorVisibleChanged.send(isNotationNavigatorVisible().val);
    });

    settings()->setDefaultValue(NOTATION_STATUSBAR_VISIBLE_KEY, Val(true));
    settings()->valueChanged(NOTATION_STATUSBAR_VISIBLE_KEY).onReceive(nullptr, [this](const Val&) {
        m_notationStatusBarVisibleChanged.send(isNotationStatusBarVisible().val);
    });
    IWorkspaceSettings::Key workspaceKey{ workspace::WorkspaceTag::UiArrangement, NOTATION_STATUSBAR_VISIBLE_KEY.key };
    workspaceSettings()->valueChanged(workspaceKey).onReceive(nullptr, [this](const Val&) {
        m_notationStatusBarVisibleChanged.send(isNotationStatusBarVisible().val);
    });
}

bool AppShellConfiguration::isAppUpdatable() const
{
#ifdef APP_UPDATABLE
    return true;
#else
    return false;
#endif
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

mu::ValCh<mu::io::paths> AppShellConfiguration::recentScorePaths() const
{
    return userScoresConfiguration()->recentScorePaths();
}

mu::ValCh<bool> AppShellConfiguration::isNotationStatusBarVisible() const
{
    ValCh<bool> visible;
    visible.ch = m_notationStatusBarVisibleChanged;
    visible.val = settings()->value(NOTATION_STATUSBAR_VISIBLE_KEY).toBool();

    return visible;
}

void AppShellConfiguration::setIsNotationStatusBarVisible(bool visible) const
{
    if (workspaceSettings()->isManage(workspace::WorkspaceTag::UiArrangement)) {
        IWorkspaceSettings::Key workspaceKey{ workspace::WorkspaceTag::UiArrangement, NOTATION_STATUSBAR_VISIBLE_KEY.key };
        workspaceSettings()->setValue(workspaceKey, Val(visible));
    } else {
        settings()->setValue(NOTATION_STATUSBAR_VISIBLE_KEY, Val(visible));
    }
}

mu::ValCh<bool> AppShellConfiguration::isNotationNavigatorVisible() const
{
    ValCh<bool> visible;
    visible.ch = m_notationNavigatorVisibleChanged;
    visible.val = settings()->value(NOTATION_NAVIGATOR_VISIBLE_KEY).toBool();

    return visible;
}

void AppShellConfiguration::setIsNotationNavigatorVisible(bool visible) const
{
    settings()->setValue(NOTATION_NAVIGATOR_VISIBLE_KEY, Val(visible));
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
    std::string languageCode = languagesConfiguration()->currentLanguageCode().val.toStdString();
    if (languageCode == SYSTEM_LANGUAGE) {
        languageCode = QLocale::system().name().toStdString();
    }

    return languageCode;
}
