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
#ifndef MU_APPSHELL_APPSHELLCONFIGURATION_H
#define MU_APPSHELL_APPSHELLCONFIGURATION_H

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "userscores/iuserscoresconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "playback/iplaybackconfiguration.h"
#include "languages/ilanguagesconfiguration.h"
#include "global/iworkspacesettings.h"

namespace mu::appshell {
class AppShellConfiguration : public IAppShellConfiguration
{
    INJECT(appshell, userscores::IUserScoresConfiguration, userScoresConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, playback::IPlaybackConfiguration, playbackConfiguration)
    INJECT(appshell, languages::ILanguagesConfiguration, languagesConfiguration)
    INJECT(appshell, framework::IWorkspaceSettings, workspaceSettings)

public:
    void init();

    StartupSessionType startupSessionType() const override;
    void setStartupSessionType(StartupSessionType type) override;

    io::path startupScorePath() const override;
    void setStartupScorePath(const io::path& scorePath) override;

    bool isAppUpdatable() const override;

    bool needCheckForUpdate() const override;
    void setNeedCheckForUpdate(bool needCheck) override;

    std::string handbookUrl() const override;
    std::string askForHelpUrl() const override;
    std::string bugReportUrl() const override;
    std::string leaveFeedbackUrl() const override;
    std::string museScoreUrl() const override;
    std::string museScoreContributionUrl() const override;
    std::string musicXMLLicenseUrl() const override;
    std::string musicXMLLicenseDeedUrl() const override;

    std::string museScoreVersion() const override;
    std::string museScoreRevision() const override;

    ValCh<io::paths> recentScorePaths() const override;

    ValCh<bool> isNotationStatusBarVisible() const override;
    void setIsNotationStatusBarVisible(bool visible) const override;
    ValCh<bool> isNotationNavigatorVisible() const override;
    void setIsNotationNavigatorVisible(bool visible) const override;

    bool needShowSplashScreen() const override;
    void setNeedShowSplashScreen(bool show) override;

    bool needShowTours() const override;
    void setNeedShowTours(bool show) override;

    void startEditSettings() override;
    void applySettings() override;
    void rollbackSettings() override;

    void revertToFactorySettings(bool keepDefaultSettings = false) const override;

private:
    std::string utmParameters(const std::string& utmMedium) const;
    std::string sha() const;

    std::string currentLanguageCode() const;

    async::Channel<bool> m_notationStatusBarVisibleChanged;
    async::Channel<bool> m_notationNavigatorVisibleChanged;
};
}

#endif // MU_APPSHELL_APPSHELLCONFIGURATION_H
