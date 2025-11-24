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

#include "ilearnservice.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ilearnconfiguration.h"
#include "network/inetworkmanagercreator.h"

namespace muse::learn {
class LearnService : public ILearnService, public Injectable, public async::Asyncable
{
    Inject<ILearnConfiguration> configuration = { this };
    Inject<network::INetworkManagerCreator> networkManagerCreator = { this };

public:
    LearnService(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void refreshPlaylists() override;

    Playlist startedPlaylist() const override;
    async::Channel<Playlist> startedPlaylistChanged() const override;

    Playlist advancedPlaylist() const override;
    async::Channel<Playlist> advancedPlaylistChanged() const override;

private:
    void downloadPlaylist(const QUrl& playlistUrl, std::function<void(RetVal<Playlist>)> callBack);

    Playlist m_startedPlaylist;
    async::Channel<Playlist> m_startedPlaylistChannel;

    Playlist m_advancedPlaylist;
    async::Channel<Playlist> m_advancedPlaylistChannel;

    muse::network::INetworkManagerPtr m_networkManager;
};
}
