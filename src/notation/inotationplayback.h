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
#ifndef MU_NOTATION_INOTATIONPLAYBACK_H
#define MU_NOTATION_INOTATIONPLAYBACK_H

#include "types/retval.h"
#include "midi/miditypes.h"
#include "audio/audiotypes.h"
#include "async/channel.h"
#include "mpe/events.h"
#include "engraving/types/types.h"

#include "internal/inotationundostack.h"
#include "notationtypes.h"

namespace mu::notation {
class INotationPlayback
{
public:
    virtual ~INotationPlayback() = default;

    virtual void init(INotationUndoStackPtr undoStack) = 0;

    virtual const engraving::InstrumentTrackId& metronomeTrackId() const = 0;
    virtual engraving::InstrumentTrackId chordSymbolsTrackId(const ID& partId) const = 0;
    virtual bool isChordSymbolsTrack(const engraving::InstrumentTrackId& trackId) const = 0;

    virtual const mpe::PlaybackData& trackPlaybackData(const engraving::InstrumentTrackId& trackId) const = 0;
    virtual void triggerEventsForItems(const std::vector<const EngravingItem*>& items, mpe::duration_t duration = -1) = 0;
    virtual void triggerMetronome(int tick) = 0;

    virtual engraving::InstrumentTrackIdSet existingTrackIdSet() const = 0;
    virtual async::Channel<engraving::InstrumentTrackId> trackAdded() const = 0;
    virtual async::Channel<engraving::InstrumentTrackId> trackRemoved() const = 0;

    virtual audio::msecs_t totalPlayTime() const = 0;
    virtual async::Channel<audio::msecs_t> totalPlayTimeChanged() const = 0;

    virtual float playedTickToSec(midi::tick_t tick) const = 0;
    virtual midi::tick_t secToPlayedTick(float sec) const = 0;
    virtual midi::tick_t secToTick(float sec) const = 0;

    virtual RetVal<midi::tick_t> playPositionTickByRawTick(midi::tick_t tick) const = 0;
    virtual RetVal<midi::tick_t> playPositionTickByElement(const EngravingItem* element) const = 0;

    enum BoundaryTick : midi :: tick_t {
        FirstScoreTick = 0,
        SelectedNoteTick,
        LastScoreTick
    };

    virtual void addLoopBoundary(LoopBoundaryType boundaryType, midi::tick_t tick) = 0;
    virtual void setLoopBoundariesVisible(bool visible) = 0;
    virtual const LoopBoundaries& loopBoundaries() const = 0;
    virtual async::Notification loopBoundariesChanged() const = 0;

    virtual const Tempo& tempo(midi::tick_t tick) const = 0;
    virtual MeasureBeat beat(midi::tick_t tick) const = 0;
    virtual midi::tick_t beatToTick(int measureIndex, int beatIndex) const = 0;

    virtual double tempoMultiplier() const = 0;
    virtual void setTempoMultiplier(double multiplier) = 0;
};

using INotationPlaybackPtr = std::shared_ptr<INotationPlayback>;
}

#endif // MU_NOTATION_INOTATIONPLAYBACK_H
