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

#pragma once

#include "istartupscenario.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "iappshellconfiguration.h"
#include "isessionsmanager.h"
#include "project/iprojectautosaver.h"
#include "audioplugins/iregisteraudiopluginsscenario.h"

#include "update/iappupdatescenario.h"
#include "musesounds/imusesoundscheckupdatescenario.h"
#include "musesounds/imusesamplercheckupdatescenario.h"

namespace mu::appshell {
class StartupScenario : public IStartupScenario, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<IAppShellConfiguration> configuration;
    muse::GlobalInject<muse::audioplugins::IRegisterAudioPluginsScenario> registerAudioPluginsScenario;
    muse::GlobalInject<mu::musesounds::IMuseSoundsCheckUpdateScenario> museSoundsUpdateScenario;
    muse::GlobalInject<musesounds::IMuseSamplerCheckUpdateScenario> museSamplerCheckForUpdateScenario;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<ISessionsManager> sessionsManager = { this };
    muse::ContextInject<project::IProjectAutoSaver> projectAutoSaver = { this };
    muse::ContextInject<muse::update::IAppUpdateScenario> appUpdateScenario = { this };

public:
    StartupScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void setStartupType(const std::optional<std::string>& type) override;

    bool isStartWithNewFileAsSecondaryInstance() const override;

    const project::ProjectFile& startupScoreFile() const override;
    void setStartupScoreFile(const std::optional<project::ProjectFile>& file) override;

    void runOnSplashScreen() override;
    void runAfterSplashScreen() override;
    bool startupCompleted() const override;

private:
    void registerAudioPlugins();

    StartupModeType resolveStartupModeType() const;

    void onStartupPageOpened(StartupModeType modeType);

    void showStartupDialogsIfNeed(StartupModeType modeType);
    void checkAndShowMuseSamplerUpdateIfNeed();
    bool shouldShowWelcomeDialog(StartupModeType modeType) const;

    void openScore(const project::ProjectFile& file);

    void restoreLastSession();
    void removeProjectsUnsavedChanges(const muse::io::paths_t& projectsPaths);

    std::string m_startupTypeStr;
    project::ProjectFile m_startupScoreFile;
    bool m_startupCompleted = false;
    size_t m_activeUpdateCheckCount = 0;
};
}
