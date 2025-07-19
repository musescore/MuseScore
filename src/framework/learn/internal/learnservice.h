/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_LEARN_LEARNSERVICE_H
#define MUSE_LEARN_LEARNSERVICE_H

#include "ilearnservice.h"

#include "modularity/ioc.h"
#include "ilearnconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "iinteractive.h"

class QJsonDocument;

namespace muse::learn {
class LearnService : public ILearnService, public Injectable
{
    Inject<ILearnConfiguration> configuration = { this };
    Inject<network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<IInteractive> interactive = { this };

public:
    LearnService(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void refreshPlaylists() override;

    Playlist startedPlaylist() const override;
    async::Channel<Playlist> startedPlaylistChanged() const override;

    Playlist advancedPlaylist() const override;
    async::Channel<Playlist> advancedPlaylistChanged() const override;

private:
    void th_requestPlaylist(const QUrl& playlistUrl, std::function<void(RetVal<Playlist>)> callBack) const;

    Playlist parsePlaylist(const QJsonDocument& playlistDoc) const;

    mutable std::mutex m_startedPlaylistMutex;
    Playlist m_startedPlaylist;
    async::Channel<Playlist> m_startedPlaylistChannel;

    mutable std::mutex m_advancedPlaylistMutex;
    Playlist m_advancedPlaylist;
    async::Channel<Playlist> m_advancedPlaylistChannel;
};
}

#endif // MUSE_LEARN_LEARNSERVICE_H
