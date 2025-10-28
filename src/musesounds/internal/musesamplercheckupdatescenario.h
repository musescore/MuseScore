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

#include "imusesamplercheckupdatescenario.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "imusesamplercheckupdateservice.h"
#include "global/iinteractive.h"
#include "global/iprocess.h"
#include "global/iglobalconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"

namespace mu::musesounds {
class MuseSamplerCheckUpdateScenario : public IMuseSamplerCheckUpdateScenario, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<IMuseSamplerCheckUpdateService> service = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::IProcess> process = { this };
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };

public:
    MuseSamplerCheckUpdateScenario(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    bool alreadyChecked() const override;
    void checkAndShowUpdateIfNeed() override;

private:
    void showCriticalUpdateNotification();
    void showNewVersionNotification();

    void openMuseHubAndQuit();
    void openMuseHubWebsiteAndQuit();

    bool m_alreadyChecked = false;
};
}
