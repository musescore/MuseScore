/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "update/iappupdatescenario.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "update/iupdateconfiguration.h"
#include "update/iappupdateservice.h"

namespace muse::update {
class AppUpdateScenario : public IAppUpdateScenario, public Injectable, public async::Asyncable
{
    GlobalInject<mi::IMultiInstancesProvider> multiInstancesProvider;
    GlobalInject<IUpdateConfiguration> configuration;
    Inject<IInteractive> interactive = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<IAppUpdateService> service = { this };

public:
    AppUpdateScenario(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    bool needCheckForUpdate() const override;
    muse::async::Promise<Ret> checkForUpdate(bool manual) override;

    bool hasUpdate() const override;
    muse::async::Promise<Ret> showUpdate() override;  // NOTE: Resolves to "OK" if the user wants to close and complete install of update...

private:
    muse::async::Promise<Ret> processUpdateError(int errorCode);

    async::Promise<IInteractive::Result> showNoUpdateMsg();
    muse::async::Promise<Ret> showReleaseInfo(const ReleaseInfo& info);
    async::Promise<IInteractive::Result> showServerErrorMsg();

    muse::async::Promise<Ret> downloadRelease();
    muse::async::Promise<Ret> askToCloseAppAndCompleteInstall(const io::path_t& installerPath);

    bool shouldIgnoreUpdate(const ReleaseInfo& info) const;

    bool m_checkInProgress = false;
};
}
