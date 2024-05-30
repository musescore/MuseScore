/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "global/async/asyncable.h"

#include "../iplaybackstate.h"
#include "audio/iplayer.h"

namespace mu::context {
class PlaybackState : public IPlaybackState, public muse::async::Asyncable
{
public:
    PlaybackState() = default;

    void setPlayer(muse::audio::IPlayerPtr player);

    muse::audio::PlaybackStatus playbackStatus() const override;
    muse::async::Channel<muse::audio::PlaybackStatus> playbackStatusChanged() const override;

    muse::audio::secs_t playbackPosition() const override;
    muse::async::Channel<muse::audio::secs_t> playbackPositionChanged() const override;

private:
    muse::audio::IPlayerPtr m_player;

    muse::async::Channel<muse::audio::PlaybackStatus> m_playbackStatusChanged;
    muse::async::Channel<muse::audio::secs_t> m_playbackPositionChanged;
};
}
