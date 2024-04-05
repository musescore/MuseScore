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
#ifndef MUSE_LEARN_ILEARNSERVICE_H
#define MUSE_LEARN_ILEARNSERVICE_H

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "learntypes.h"

namespace muse::learn {
class ILearnService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILearnService)

public:
    virtual ~ILearnService() = default;

    virtual void refreshPlaylists() = 0;

    virtual Playlist startedPlaylist() const = 0;
    virtual async::Channel<Playlist> startedPlaylistChanged() const = 0;

    virtual Playlist advancedPlaylist() const = 0;
    virtual async::Channel<Playlist> advancedPlaylistChanged() const = 0;
};
}

#endif // MUSE_LEARN_ILEARNSERVICE_H
