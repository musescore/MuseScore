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
#ifndef MU_APPSHELL_STARTUPSCENARIO_H
#define MU_APPSHELL_STARTUPSCENARIO_H

#include "../../istartupscenario.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iappshellconfiguration.h"
#include "../../isessionsmanager.h"
#include "project/iprojectautosaver.h"
#include "audioplugins/iregisteraudiopluginsscenario.h"

namespace mu::appshell {
class StartupScenario : public IStartupScenario, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<IAppShellConfiguration> configuration = { this };
    muse::Inject<ISessionsManager> sessionsManager = { this };
    muse::Inject<project::IProjectAutoSaver> projectAutoSaver = { this };
    muse::Inject<muse::audioplugins::IRegisterAudioPluginsScenario> registerAudioPluginsScenario = { this };

public:

    StartupScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void setStartupType(const std::optional<std::string>& type) override;

    bool isStartWithNewFileAsSecondaryInstance() const override;

    const project::ProjectFile& startupScoreFile() const override;
    void setStartupScoreFile(const std::optional<project::ProjectFile>& file) override;

    void runOnSplashScreen() override;
    void runAfterSplashScreen() override;
    bool startupCompleted() const override;

private:
    void onStartupPageOpened(StartupModeType modeType);

    StartupModeType resolveStartupModeType() const;
    muse::Uri startupPageUri(StartupModeType modeType) const;

    void openScore(const project::ProjectFile& file);

    void restoreLastSession();
    void removeProjectsUnsavedChanges(const muse::io::paths_t& projectsPaths);

    std::string m_startupTypeStr;
    project::ProjectFile m_startupScoreFile;
    bool m_startupCompleted = false;
};
}

#endif // MU_APPSHELL_STARTUPSCENARIO_H
