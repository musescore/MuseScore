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

#include <gtest/gtest.h>

#include <vector>

#include "playback/internal/playbackcontroller.h"

using namespace mu::engraving;
using namespace mu::playback;

namespace {
InstrumentTrackId makeTrackId(int partId)
{
    return { muse::ID(partId), u"instrument" };
}

detail::InstrumentTrackSoloMuteState makeTrackState(const InstrumentTrackId& trackId,
                                                    bool mute = false,
                                                    bool solo = false,
                                                    bool hasPlaybackTrack = true,
                                                    bool isMetronome = false,
                                                    bool isChordSymbols = false)
{
    return { trackId, { mute, solo }, hasPlaybackTrack, isMetronome, isChordSymbols };
}

InstrumentTrackIdSet audibleTrackIds(
    const std::vector<detail::InstrumentTrackSoloMuteState>& trackStates,
    bool playChordSymbols = true,
    bool isRangePlaybackMode = false,
    const InstrumentTrackIdSet& allowedInstrumentTrackIds = {})
{
    return detail::audibleInstrumentTrackIds(
        detail::resolveEffectiveTrackSoloMuteStates(trackStates,
                                                    playChordSymbols,
                                                    isRangePlaybackMode,
                                                    allowedInstrumentTrackIds));
}
}

TEST(PlaybackControllerTests, UnmutedPlaybackTracksAreAudible)
{
    const InstrumentTrackId firstTrackId = makeTrackId(1);
    const InstrumentTrackId secondTrackId = makeTrackId(2);

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(firstTrackId),
        makeTrackState(secondTrackId)
    });

    EXPECT_EQ(result.size(), 2u);
    EXPECT_TRUE(result.contains(firstTrackId));
    EXPECT_TRUE(result.contains(secondTrackId));
}

TEST(PlaybackControllerTests, PersistentOrExcerptMuteMakesTrackInaudible)
{
    const InstrumentTrackId mutedTrackId = makeTrackId(1);
    const InstrumentTrackId audibleTrackId = makeTrackId(2);

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(mutedTrackId, true),
        makeTrackState(audibleTrackId)
    });

    EXPECT_FALSE(result.contains(mutedTrackId));
    EXPECT_TRUE(result.contains(audibleTrackId));
}

TEST(PlaybackControllerTests, SoloMakesNonSoloTracksInaudible)
{
    const InstrumentTrackId soloTrackId = makeTrackId(1);
    const InstrumentTrackId nonSoloTrackId = makeTrackId(2);

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(soloTrackId, false, true),
        makeTrackState(nonSoloTrackId)
    });

    EXPECT_TRUE(result.contains(soloTrackId));
    EXPECT_FALSE(result.contains(nonSoloTrackId));
}

TEST(PlaybackControllerTests, DisabledChordSymbolsTrackIsInaudible)
{
    const InstrumentTrackId notationTrackId = makeTrackId(1);
    const InstrumentTrackId chordSymbolsTrackId = makeTrackId(2);
    const std::vector<detail::InstrumentTrackSoloMuteState> trackStates = {
        makeTrackState(notationTrackId),
        makeTrackState(chordSymbolsTrackId, false, false, true, false, true)
    };

    const InstrumentTrackIdSet disabledResult = audibleTrackIds(trackStates, false);
    const InstrumentTrackIdSet enabledResult = audibleTrackIds(trackStates, true);

    EXPECT_FALSE(disabledResult.contains(chordSymbolsTrackId));
    EXPECT_TRUE(disabledResult.contains(notationTrackId));
    EXPECT_TRUE(enabledResult.contains(chordSymbolsTrackId));
}

TEST(PlaybackControllerTests, RangePlaybackOnlyAllowsSelectedTracks)
{
    const InstrumentTrackId selectedTrackId = makeTrackId(1);
    const InstrumentTrackId outsideRangeTrackId = makeTrackId(2);
    const InstrumentTrackIdSet allowedInstrumentTrackIds = { selectedTrackId };

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(selectedTrackId),
        makeTrackState(outsideRangeTrackId)
    }, true, true, allowedInstrumentTrackIds);

    EXPECT_TRUE(result.contains(selectedTrackId));
    EXPECT_FALSE(result.contains(outsideRangeTrackId));
}

TEST(PlaybackControllerTests, MetronomeIsExcludedAndDoesNotActivateSolo)
{
    const InstrumentTrackId notationTrackId = makeTrackId(1);
    const InstrumentTrackId metronomeTrackId = makeTrackId(2);

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(notationTrackId),
        makeTrackState(metronomeTrackId, false, true, true, true)
    });

    EXPECT_TRUE(result.contains(notationTrackId));
    EXPECT_FALSE(result.contains(metronomeTrackId));
}

TEST(PlaybackControllerTests, TrackWithoutPlaybackChannelIsInaudible)
{
    const InstrumentTrackId loadedTrackId = makeTrackId(1);
    const InstrumentTrackId unloadedTrackId = makeTrackId(2);

    const InstrumentTrackIdSet result = audibleTrackIds({
        makeTrackState(loadedTrackId),
        makeTrackState(unloadedTrackId, false, false, false)
    });

    EXPECT_TRUE(result.contains(loadedTrackId));
    EXPECT_FALSE(result.contains(unloadedTrackId));
}
