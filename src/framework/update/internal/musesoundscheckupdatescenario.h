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
#ifndef MUSE_UPDATE_MUSESOUNDSCHECKUPDATESCENARIO_H
#define MUSE_UPDATE_MUSESOUNDSCHECKUPDATESCENARIO_H

#include "async/asyncable.h"
#include "progress.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "../iupdateconfiguration.h"
#include "../imusesoundscheckupdateservice.h"

#include "../iappupdateservice.h"
#include "../imusesoundscheckupdatescenario.h"

namespace muse::update {
class MuseSoundsCheckUpdateScenario : public IMuseSoundsCheckUpdateScenario, public Injectable, public async::Asyncable
{
    Inject<IInteractive> interactive = { this };
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    Inject<IUpdateConfiguration> configuration = { this };

    Inject<IMuseSoundsCheckUpdateService> service = { this };

public:
    MuseSoundsCheckUpdateScenario(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void delayedInit();

    void checkForUpdate() override;

private:
    bool isCheckStarted() const;

    bool shouldIgnoreUpdate(const ReleaseInfo& info) const;
    void setIgnoredUpdate(const std::string& version);

    void doCheckForUpdate(bool manual);
    void th_checkForUpdate();

    void showReleaseInfo(const ReleaseInfo& info);
    void tryOpenMuseHub(ValList actions) const;

    bool m_checkProgress = false;
    ProgressPtr m_checkProgressChannel = nullptr;
};
}

#endif // MUSE_UPDATE_MUSESOUNDSCHECKUPDATESCENARIO_H
