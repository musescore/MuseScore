/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include "musesounds/imusesamplercheckstatusscenario.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "rcommand/icommanddispatcher.h"
#include "musesounds/imusesamplercheckstatusservice.h"
#include "musesounds/imusesoundsconfiguration.h"
#include "global/iprocess.h"
#include "global/iglobalconfiguration.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "update/iupdateconfiguration.h"

namespace mu::musesounds {
class MuseSamplerCheckStatusScenario : public IMuseSamplerCheckStatusScenario, public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<muse::update::IUpdateConfiguration> updateConfiguration;
    muse::GlobalInject<muse::IProcess> process;
    muse::GlobalInject<IMuseSoundsConfiguration> configuration;
    muse::GlobalInject<IMuseSamplerCheckStatusService> service;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };

public:
    MuseSamplerCheckStatusScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    bool alreadyChecked() const override;
    void checkAndShowStatusIfNeed() override;

private:
    void showLoadingFailedNotification();
    void showCriticalUpdateNotification();
    void showNewVersionNotification();

    void openMuseHub(bool quit);
    void openMuseHubWebsite(bool quit);

    bool m_alreadyChecked = false;
};
}
