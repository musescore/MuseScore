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
#include "playbackstate.h"

using namespace muse;
using namespace mu::context;

void PlaybackState::setPlayer(audio::IPlayerPtr player)
{
    if (m_player) {
        m_player->playbackStatusChanged().resetOnReceive(this);
        m_player->playbackPositionChanged().resetOnReceive(this);
    }

    m_player = player;

    if (!m_player) {
        return;
    }

    //! NOTE The redirect is needed so that consumers do not have to worry about resubscribing if the player changes

    m_playbackStatusChanged.send(m_player->playbackStatus());
    m_playbackPositionChanged.send(m_player->playbackPosition());

    m_player->playbackStatusChanged().onReceive(this, [this](audio::PlaybackStatus st) {
        m_playbackStatusChanged.send(st);
    });

    m_player->playbackPositionChanged().onReceive(this, [this](audio::secs_t pos) {
        m_playbackPositionChanged.send(pos);
    });
}

audio::PlaybackStatus PlaybackState::playbackStatus() const
{
    return m_player ? m_player->playbackStatus() : audio::PlaybackStatus::Stopped;
}

async::Channel<audio::PlaybackStatus> PlaybackState::playbackStatusChanged() const
{
    return m_playbackStatusChanged;
}

audio::secs_t PlaybackState::playbackPosition() const
{
    return m_player ? m_player->playbackPosition() : audio::secs_t(0.0);
}

async::Channel<audio::secs_t> PlaybackState::playbackPositionChanged() const
{
    return m_playbackPositionChanged;
}
