/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "imusesamplercheckupdateservice.h"

#include "modularity/ioc.h"
#include "musesampler/imusesamplerinfo.h"
#include "musesampler/imusesamplerconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "imusesoundsconfiguration.h"
#include "async/asyncable.h"

#include "async/channel.h"

namespace mu::musesounds {
class MuseSamplerCheckUpdateService : public IMuseSamplerCheckUpdateService, public muse::Injectable, public muse::async::Asyncable
{
    Inject<muse::musesampler::IMuseSamplerInfo> museSampler = { this };
    Inject<muse::musesampler::IMuseSamplerConfiguration> museSamplerConfiguration = { this };
    Inject<muse::network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<IMuseSoundsConfiguration> configuration = { this };

public:
    MuseSamplerCheckUpdateService(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    bool canCheckForUpdate() const override;
    bool incompatibleLocalVersion() const override;

    muse::async::Promise<muse::RetVal<bool> > checkForUpdate() override;
};
}
