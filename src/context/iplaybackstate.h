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

#include <memory>

#include "global/async/channel.h"
#include "audio/audiotypes.h"

namespace mu::context {
//! NOTE The current player is in the global context.
//! We need to get the current state from it (playing position, status)
//! But direct manage (play, seek..) is bad idea, need send actions (to playback controller)
//! So, this interface limits the player interface to the context so that it cannot be misused.
class IPlaybackState
{
public:
    virtual ~IPlaybackState() = default;

    virtual muse::audio::PlaybackStatus playbackStatus() const = 0;
    virtual muse::async::Channel<muse::audio::PlaybackStatus> playbackStatusChanged() const = 0;

    virtual muse::audio::secs_t playbackPosition() const = 0;
    virtual muse::async::Channel<muse::audio::secs_t> playbackPositionChanged() const = 0;
};

using IPlaybackStatePtr = std::shared_ptr<IPlaybackState>;
}
