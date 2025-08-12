/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "async/asyncable.h"
#include "progress.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "../imusesoundsconfiguration.h"
#include "../imusesoundscheckupdateservice.h"

#include "../imusesoundscheckupdatescenario.h"

namespace mu::musesounds {
class MuseSoundsCheckUpdateScenario : public IMuseSoundsCheckUpdateScenario, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<IMuseSoundsConfiguration> configuration = { this };

    muse::Inject<IMuseSoundsCheckUpdateService> service = { this };

public:
    MuseSoundsCheckUpdateScenario(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    bool needCheckForUpdate() const override;
    muse::async::Promise<muse::Ret> checkForUpdate(bool manual) override;

    bool hasUpdate() const override;
    muse::Ret showUpdate() override;

private:
    bool isCheckInProgress() const;

    bool shouldIgnoreUpdate(const muse::update::ReleaseInfo& info) const;
    void setIgnoredUpdate(const std::string& version);

    void th_checkForUpdate();

    muse::Ret showReleaseInfo(const muse::update::ReleaseInfo& info);
    void tryOpenMuseHub(muse::ValList actions) const;

    bool m_checkInProgress = false;
    muse::ProgressPtr m_checkProgressChannel = nullptr;
};
}
