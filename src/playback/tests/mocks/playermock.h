/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "audio/main/iplayer.h"

namespace muse::audio {
class PlayerMock : public IPlayer
{
public:
    MOCK_METHOD(async::Promise<Ret>, prepareToPlay, (), (override));

    MOCK_METHOD(void, play, (const secs_t delay), (override));
    MOCK_METHOD(void, seek, (const secs_t newPosition, const bool flushSound), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, resume, (const secs_t delay), (override));

    MOCK_METHOD(PlaybackStatus, playbackStatus, (), (const, override));
    MOCK_METHOD(async::Channel<PlaybackStatus>, playbackStatusChanged, (), (const, override));

    MOCK_METHOD(void, setDuration, (const secs_t duration), (override));
    MOCK_METHOD(async::Promise<bool>, setLoop, (const secs_t from, const secs_t to), (override));
    MOCK_METHOD(void, resetLoop, (), (override));

    MOCK_METHOD(secs_t, playbackPosition, (), (const, override));
    MOCK_METHOD(async::Channel<secs_t>, playbackPositionChanged, (), (const, override));
};
}
