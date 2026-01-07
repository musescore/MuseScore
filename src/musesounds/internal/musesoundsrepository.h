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

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "network/inetworkmanagercreator.h"
#include "imusesoundsconfiguration.h"

#include "imusesoundsrepository.h"

namespace muse {
class JsonDocument;
}

namespace mu::musesounds {
class MuseSoundsRepository : public IMuseSoundsRepository, public muse::Injectable, public muse::async::Asyncable
{
    muse::GlobalInject<muse::network::INetworkManagerCreator> networkManagerCreator;
    muse::GlobalInject<IMuseSoundsConfiguration> configuration;

public:
    MuseSoundsRepository(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    const SoundCatalogueInfoList& soundsCatalogueList() const override;
    muse::async::Notification soundsCatalogueListChanged() const override;

private:
    SoundCatalogueInfoList parseSounds(const muse::JsonDocument& soundsDoc) const;

    SoundCatalogueInfoList m_soundsСatalogs;
    muse::async::Notification m_soundsСatalogsChanged;
    muse::network::INetworkManagerPtr m_networkManager;
};
}
