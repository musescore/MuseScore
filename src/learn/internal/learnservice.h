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
#ifndef MU_LEARN_LEARNSERVICE_H
#define MU_LEARN_LEARNSERVICE_H

#include <QObject>

#include "ilearnservice.h"

#include "modularity/ioc.h"
#include "ilearnconfiguration.h"
#include "network/inetworkmanagercreator.h"
#include "iinteractive.h"

namespace mu::learn {
class LearnService : public QObject, public ILearnService
{
    Q_OBJECT

    INJECT(learn, ILearnConfiguration, configuration)
    INJECT(learn, network::INetworkManagerCreator, networkManagerCreator)
    INJECT(learn, framework::IInteractive, interactive)

public:
    LearnService(QObject* parent = nullptr);

    void init();

    Playlist startedPlaylist() const override;
    Playlist advancedPlaylist() const override;

    void openVideo(const std::string& videoId) const override;

private:
    Playlist requestPlaylist(const QUrl& playlistUrl) const;

    void openUrl(const QUrl& url);

    std::vector<std::string> parsePlaylistItemsIds(const QVariantMap& playlistMap) const;
    Playlist parsePlaylist(const QVariantMap& playlistMap) const;

    network::INetworkManagerPtr m_networkManager;
};
}

#endif // MU_LEARN_LEARNSERVICE_H
