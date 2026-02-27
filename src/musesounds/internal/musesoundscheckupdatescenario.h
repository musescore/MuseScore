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

#include "musesounds/imusesoundscheckupdatescenario.h"

#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"

#include "musesounds/imusesoundscheckupdateservice.h"
#include "musesounds/imusesoundsconfiguration.h"

#include "interactive/iinteractive.h"

namespace mu::musesounds {
class MuseSoundsCheckUpdateScenario : public IMuseSoundsCheckUpdateScenario, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<IMuseSoundsConfiguration> configuration;
    muse::GlobalInject<IMuseSoundsCheckUpdateService> service;
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    MuseSoundsCheckUpdateScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    bool needCheckForUpdate() const override;
    void checkForUpdate(bool manual) override;

    bool checkInProgress() const override;
    muse::async::Notification checkInProgressChanged() const override;

    bool hasUpdate() const override;
    muse::Ret showUpdate() override;

private:
    bool shouldIgnoreUpdate(const muse::update::ReleaseInfo& info) const;
    void setIgnoredUpdate(const std::string& version);

    muse::Ret showReleaseInfo(const muse::update::ReleaseInfo& info);
    void tryOpenMuseHub(muse::ValList actions) const;

    bool m_checkInProgress = false;
    muse::async::Notification m_checkInProgressChanged;
};
}
