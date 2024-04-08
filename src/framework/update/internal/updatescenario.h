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
#ifndef MUSE_UPDATE_UPDATESCENARIO_H
#define MUSE_UPDATE_UPDATESCENARIO_H

#include "async/asyncable.h"
#include "progress.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "../iupdateconfiguration.h"

#include "../iappupdateservice.h"
#include "../imusesamplerupdateservice.h"

#include "../iupdatescenario.h"

namespace muse::update {
class UpdateScenario : public IUpdateScenario, public async::Asyncable
{
    Inject<IInteractive> interactive;
    Inject<actions::IActionsDispatcher> dispatcher;
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider;
    Inject<IUpdateConfiguration> configuration;

    Inject<IAppUpdateService> appUpdateService;
    Inject<IMuseSamplerUpdateService> museSamplerUpdateService;

public:
    void delayedInit();

    void checkForAppUpdate() override;
    void checkForMuseSamplerUpdate() override;

private:
    bool isAppCheckStarted() const;
    bool isMuseSamplerCheckStarted() const;

    bool shouldIgnoreMuseSamplerUpdate(const ReleaseInfo& info)const;
    void setIgnoredMuseSamplerUpdate(const std::string& version);

    void doCheckForAppUpdate(bool manual);
    void th_checkForAppUpdate();

    void doCheckForMuseSamplerUpdate(bool manual);
    void th_checkForMuseSamplerUpdate();

    void processUpdateResult(int errorCode);

    void showNoAppUpdateMsg();
    void showAppReleaseInfo(const ReleaseInfo& info);

    void showMuseSamplerReleaseInfo(const ReleaseInfo& info);

    void showServerErrorMsg();

    void downloadRelease();
    void closeAppAndStartInstallation(const io::path_t& installerPath);

    bool m_appCheckProgress = false;
    ProgressPtr m_appCheckProgressChannel = nullptr;

    bool m_museSamplerCheckProgress = false;
    ProgressPtr m_museSamplerCheckProgressChannel = nullptr;
};
}

#endif // MUSE_UPDATE_UPDATESCENARIO_H
