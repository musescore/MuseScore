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
#include "notationplaybackstub.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::midi;
using namespace muse::async;

NotationPlaybackStub::NotationPlaybackStub()
{
}

void NotationPlaybackStub::init()
{
}

void NotationPlaybackStub::reload()
{
}

const engraving::InstrumentTrackId& NotationPlaybackStub::metronomeTrackId() const
{
    static const engraving::InstrumentTrackId dummy;
    return dummy;
}

engraving::InstrumentTrackId NotationPlaybackStub::chordSymbolsTrackId(const ID&) const
{
    static const engraving::InstrumentTrackId dummy;
    return dummy;
}

bool NotationPlaybackStub::isChordSymbolsTrack(const engraving::InstrumentTrackId&) const
{
    return false;
}

const muse::mpe::PlaybackData& NotationPlaybackStub::trackPlaybackData(const engraving::InstrumentTrackId&) const
{
    static const muse::mpe::PlaybackData dummy;
    return dummy;
}

void NotationPlaybackStub::triggerEventsForItems(const std::vector<const EngravingItem*>&)
{
}

void NotationPlaybackStub::triggerMetronome(int)
{
}

InstrumentTrackIdSet NotationPlaybackStub::existingTrackIdSet() const
{
    return engraving::InstrumentTrackIdSet();
}

muse::async::Channel<InstrumentTrackId> NotationPlaybackStub::trackAdded() const
{
    return muse::async::Channel<InstrumentTrackId>();
}

muse::async::Channel<InstrumentTrackId> NotationPlaybackStub::trackRemoved() const
{
    return muse::async::Channel<InstrumentTrackId>();
}

muse::audio::secs_t NotationPlaybackStub::totalPlayTime() const
{
    return muse::audio::secs_t();
}

muse::async::Channel<muse::audio::secs_t> NotationPlaybackStub::totalPlayTimeChanged() const
{
    return muse::async::Channel<muse::audio::secs_t>();
}

muse::audio::secs_t NotationPlaybackStub::playedTickToSec(tick_t) const
{
    return muse::audio::secs_t();
}

tick_t NotationPlaybackStub::secToPlayedTick(muse::audio::secs_t) const
{
    return 0;
}

tick_t NotationPlaybackStub::secToTick(muse::audio::secs_t) const
{
    return 0;
}

RetVal<muse::midi::tick_t> NotationPlaybackStub::playPositionTickByRawTick(muse::midi::tick_t) const
{
    return RetVal<muse::midi::tick_t>::make_ok(0);
}

RetVal<muse::midi::tick_t> NotationPlaybackStub::playPositionTickByElement(const EngravingItem*) const
{
    return RetVal<muse::midi::tick_t>::make_ok(0);
}

void NotationPlaybackStub::addLoopBoundary(LoopBoundaryType, tick_t)
{
}

void NotationPlaybackStub::setLoopBoundariesEnabled(bool)
{
}

const LoopBoundaries& NotationPlaybackStub::loopBoundaries() const
{
    static const LoopBoundaries dummy;
    return dummy;
}

Notification NotationPlaybackStub::loopBoundariesChanged() const
{
    return Notification();
}

const Tempo& NotationPlaybackStub::tempo(tick_t) const
{
    static const Tempo dummy;
    return dummy;
}

MeasureBeat NotationPlaybackStub::beat(tick_t) const
{
    return MeasureBeat();
}

tick_t NotationPlaybackStub::beatToRawTick(int, int) const
{
    return 0;
}

double NotationPlaybackStub::tempoMultiplier() const
{
    return 0;
}

void NotationPlaybackStub::setTempoMultiplier(double)
{
}

void NotationPlaybackStub::addSoundFlags(const std::vector<StaffText*>&)
{
}

void NotationPlaybackStub::removeSoundFlags(const InstrumentTrackIdSet&)
{
}

bool NotationPlaybackStub::hasSoundFlags(const engraving::InstrumentTrackIdSet&)
{
    return false;
}
