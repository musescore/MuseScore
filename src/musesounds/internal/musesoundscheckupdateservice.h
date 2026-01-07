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

#include "musesounds/imusesoundscheckupdateservice.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "musesounds/imusesoundsconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "global/iglobalconfiguration.h"
#include "languages/ilanguagesconfiguration.h"
#include "global/iinteractive.h"

namespace mu::musesounds {
class MuseSoundsCheckUpdateService : public IMuseSoundsCheckUpdateService, public muse::Injectable, public muse::async::Asyncable
{
    muse::GlobalInject<IMuseSoundsConfiguration> configuration;
    muse::GlobalInject<muse::network::INetworkManagerCreator> networkManagerCreator;
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<muse::languages::ILanguagesConfiguration> languagesConfiguration;
    muse::Inject<muse::IInteractive> interactive = { this };

public:
    MuseSoundsCheckUpdateService(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    muse::Ret needCheckForUpdate() const override;

    muse::async::Promise<muse::RetVal<muse::update::ReleaseInfo> > checkForUpdate() override;
    const muse::RetVal<muse::update::ReleaseInfo>& lastCheckResult() const override;

private:
    muse::RetVal<muse::update::ReleaseInfo> parseRelease(const QByteArray& json) const;

    muse::RetVal<muse::update::ReleaseInfo> m_lastCheckResult;
    muse::network::INetworkManagerPtr m_networkManager;
};
}
