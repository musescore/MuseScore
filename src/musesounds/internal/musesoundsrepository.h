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

#include "modularity/ioc.h"
#include "network/inetworkmanagercreator.h"
#include "imusesoundsconfiguration.h"

#include "imusesoundsrepository.h"

namespace muse {
class JsonDocument;
}

namespace mu::musesounds {
class MuseSoundsRepository : public IMuseSoundsRepository, public muse::Injectable
{
    Inject<muse::network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<IMuseSoundsConfiguration> configuration = { this };

public:
    MuseSoundsRepository(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    const SoundCatalogueInfoList& soundsCatalogueList() const override;
    muse::async::Notification soundsCatalogueListChanged() const override;

private:
    QByteArray soundsRequestJson() const;
    void th_requestSounds(const muse::UriQuery& soundsUri, std::function<void(muse::RetVal<SoundCatalogueInfoList>)> callBack) const;

    SoundCatalogueInfoList parseSounds(const muse::JsonDocument& soundsDoc) const;

    mutable std::mutex m_mutex;
    SoundCatalogueInfoList m_soundsСatalogs;
    muse::async::Notification m_soundsСatalogsChanged;
};
}
