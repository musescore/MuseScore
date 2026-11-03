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

#include "notation/inotationplayback.h"

namespace mu::notation {
class NotationPlaybackMock : public INotationPlayback
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, reload, (), (override));

    MOCK_METHOD(void, setSendEventsOnScoreChange, (const engraving::InstrumentTrackId& trackId, bool send), (override));
    MOCK_METHOD(void, sendEventsForChangedTracks, (), (override));

    MOCK_METHOD(muse::async::Channel<engraving::InstrumentTrackIdSet>, tracksDataChanged, (), (const, override));

    MOCK_METHOD(const engraving::InstrumentTrackId&, metronomeTrackId, (), (const, override));
    MOCK_METHOD(engraving::InstrumentTrackId, chordSymbolsTrackId, (const muse::ID& partId), (const, override));
    MOCK_METHOD(bool, isChordSymbolsTrack, (const engraving::InstrumentTrackId& trackId), (const, override));
    MOCK_METHOD(std::optional<std::string>, trackName, (const engraving::InstrumentTrackId& trackId), (const, override));

    MOCK_METHOD(const muse::mpe::PlaybackData&, trackPlaybackData, (const engraving::InstrumentTrackId& trackId), (const, override));

    MOCK_METHOD(void, triggerEventsForItems, (const std::vector<const engraving::EngravingItem*>& items, muse::mpe::duration_t duration,
                                              bool flushSound), (override));
    MOCK_METHOD(void, triggerMetronome, (muse::midi::tick_t tick), (override));
    MOCK_METHOD(void, triggerCountIn, (muse::midi::tick_t tick, muse::secs_t& countInDuration), (override));
    MOCK_METHOD(void, triggerControllers, (const muse::mpe::ControllerChangeEventList& list, engraving::staff_idx_t staffIdx, int tick),
                (override));

    MOCK_METHOD(engraving::InstrumentTrackIdSet, existingTrackIdSet, (), (const, override));
    MOCK_METHOD(muse::async::Channel<engraving::InstrumentTrackId>, trackAdded, (), (const, override));
    MOCK_METHOD(muse::async::Channel<engraving::InstrumentTrackId>, trackRemoved, (), (const, override));

    MOCK_METHOD(muse::audio::secs_t, totalPlayTime, (), (const, override));
    MOCK_METHOD(muse::async::Channel<muse::audio::secs_t>, totalPlayTimeChanged, (), (const, override));

    MOCK_METHOD(muse::audio::secs_t, playedTickToSec, (muse::midi::tick_t tick), (const, override));
    MOCK_METHOD(muse::midi::tick_t, secToPlayedTick, (muse::audio::secs_t sec), (const, override));
    MOCK_METHOD(muse::midi::tick_t, secToTick, (muse::audio::secs_t sec), (const, override));

    MOCK_METHOD(muse::RetVal<muse::midi::tick_t>, playPositionTickByRawTick, (muse::midi::tick_t tick), (const, override));
    MOCK_METHOD(muse::RetVal<muse::midi::tick_t>, playPositionTickByElement, (const engraving::EngravingItem * element), (const, override));

    MOCK_METHOD(void, addLoopBoundary, (LoopBoundaryType boundaryType, muse::midi::tick_t tick), (override));
    MOCK_METHOD(void, setLoopBoundariesEnabled, (bool enabled), (override));
    MOCK_METHOD(bool, isLoopEnabled, (), (const, override));
    MOCK_METHOD(muse::async::Channel<bool>, loopEnabledChanged, (), (const, override));
    MOCK_METHOD(const LoopBoundaries&, loopBoundaries, (), (const, override));
    MOCK_METHOD(muse::async::Notification, loopBoundariesChanged, (), (const, override));

    MOCK_METHOD(const Tempo&, multipliedTempo, (muse::midi::tick_t tick), (const, override));
    MOCK_METHOD(engraving::MeasureBeat, beat, (muse::midi::tick_t tick), (const, override));
    MOCK_METHOD(muse::midi::tick_t, beatToRawTick, (int measureIndex, int beatIndex), (const, override));

    MOCK_METHOD(double, tempoMultiplier, (), (const, override));
    MOCK_METHOD(void, setTempoMultiplier, (double multiplier), (override));

    MOCK_METHOD(void, addSoundFlags, (const std::vector<mu::engraving::StaffText*>& staffTextList), (override));
    MOCK_METHOD(void, removeSoundFlags, (const engraving::InstrumentTrackIdSet& trackIdSet), (override));
    MOCK_METHOD(bool, hasSoundFlags, (const engraving::InstrumentTrackIdSet& trackIdSet), (override));
};
}
