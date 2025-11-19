/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "../iupdatescenario.h"

namespace muse::update {
class UpdateScenario : public IUpdateScenario, public Injectable, public async::Asyncable
{
    Inject<IInteractive> interactive = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    Inject<IUpdateConfiguration> configuration = { this };

    Inject<IAppUpdateService> service = { this };

public:
    UpdateScenario(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    bool needCheckForUpdate() const override;
    muse::async::Promise<Ret> checkForUpdate(bool manual) override;

    bool hasUpdate() const override;
    muse::async::Promise<Ret> showUpdate() override;  // NOTE: Resolves to "OK" if the user wants to close and complete install of update...

private:
    bool isCheckInProgress() const;

    void th_checkForUpdate();

    muse::async::Promise<Ret> processUpdateError(int errorCode);

    async::Promise<IInteractive::Result> showNoUpdateMsg();
    muse::async::Promise<Ret> showReleaseInfo(const ReleaseInfo& info);

    async::Promise<IInteractive::Result> showServerErrorMsg();

    muse::async::Promise<Ret> downloadRelease();
    muse::async::Promise<Ret> askToCloseAppAndCompleteInstall(const io::path_t& installerPath);

    bool shouldIgnoreUpdate(const ReleaseInfo& info) const;

    bool m_checkInProgress = false;
    ProgressPtr m_checkProgressChannel = nullptr;
};
}

#endif // MUSE_UPDATE_UPDATESCENARIO_H
