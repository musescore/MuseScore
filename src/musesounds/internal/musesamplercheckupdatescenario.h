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
#include "global/iprocess.h"
#include "global/iglobalconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "interactive/iinteractive.h"
#include "multiwindows/imultiwindowsprovider.h"

namespace mu::musesounds {
class MuseSamplerCheckUpdateScenario : public IMuseSamplerCheckUpdateScenario, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<muse::IProcess> process;
    muse::ContextInject<IMuseSamplerCheckUpdateService> service = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    MuseSamplerCheckUpdateScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

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
