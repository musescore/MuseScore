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
#ifndef MU_APPSHELL_APPSHELLCONFIGURATION_H
#define MU_APPSHELL_APPSHELLCONFIGURATION_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "framework/system/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "ui/iuiconfiguration.h"
#include "project/iprojectconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "playback/iplaybackconfiguration.h"
#include "languages/ilanguagesconfiguration.h"

#include "iappshellconfiguration.h"

namespace mu::appshell {
class AppShellConfiguration : public IAppShellConfiguration, public async::Asyncable
{
    INJECT(appshell, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(appshell, system::IFileSystem, fileSystem)
    INJECT(appshell, mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)
    INJECT(appshell, project::IProjectConfiguration, projectConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, playback::IPlaybackConfiguration, playbackConfiguration)
    INJECT(appshell, languages::ILanguagesConfiguration, languagesConfiguration)

public:
    void init();

    bool hasCompletedFirstLaunchSetup() const override;
    void setHasCompletedFirstLaunchSetup(bool has) override;

    StartupModeType startupModeType() const override;
    void setStartupModeType(StartupModeType type) override;

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
    std::string museScoreForumUrl() const override;
    std::string museScoreContributionUrl() const override;
    std::string musicXMLLicenseUrl() const override;
    std::string musicXMLLicenseDeedUrl() const override;

    std::string museScoreVersion() const override;
    std::string museScoreRevision() const override;

    bool isNotationNavigatorVisible() const override;
    void setIsNotationNavigatorVisible(bool visible) const override;
    async::Notification isNotationNavigatorVisibleChanged() const override;

    bool needShowSplashScreen() const override;
    void setNeedShowSplashScreen(bool show) override;

    void startEditSettings() override;
    void applySettings() override;
    void rollbackSettings() override;

    void revertToFactorySettings(bool keepDefaultSettings = false) const override;

    io::paths sessionProjectsPaths() const override;
    Ret setSessionProjectsPaths(const io::paths& paths) override;

private:
    std::string utmParameters(const std::string& utmMedium) const;
    std::string sha() const;

    std::string currentLanguageCode() const;

    io::path sessionDataPath() const;
    io::path sessionFilePath() const;

    RetVal<QByteArray> readSessionState() const;
    Ret writeSessionState(const QByteArray& data);

    io::paths parseSessionProjectsPaths(const QByteArray& json) const;
};
}

#endif // MU_APPSHELL_APPSHELLCONFIGURATION_H
