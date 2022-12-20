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

#ifndef MU_ENGRAVING_PLAYBACKMODEL_H
#define MU_ENGRAVING_PLAYBACKMODEL_H

#include <unordered_map>
#include <map>
#include <functional>

#include "async/asyncable.h"
#include "async/channel.h"
#include "async/notification.h"
#include "types/id.h"
#include "modularity/ioc.h"
#include "mpe/events.h"
#include "mpe/iarticulationprofilesrepository.h"

#include "types/types.h"
#include "playbackeventsrenderer.h"
#include "playbacksetupdataresolver.h"
#include "playbackcontext.h"

namespace mu::engraving {
class Score;
class Note;
class EngravingItem;
class Segment;
class Instrument;
class RepeatList;

class PlaybackModel : public async::Asyncable
{
    INJECT(engraving, mpe::IArticulationProfilesRepository, profilesRepository)

public:
    void load(Score* score);
    void reload();

    async::Notification dataChanged() const;

    bool isPlayRepeatsEnabled() const;
    void setPlayRepeats(const bool isEnabled);

    bool isPlayChordSymbolsEnabled() const;
    void setPlayChordSymbols(const bool isEnabled);

    const InstrumentTrackId& metronomeTrackId() const;
    InstrumentTrackId chordSymbolsTrackId(const ID& partId) const;
    bool isChordSymbolsTrack(const InstrumentTrackId& trackId) const;

    const mpe::PlaybackData& resolveTrackPlaybackData(const InstrumentTrackId& trackId);
    const mpe::PlaybackData& resolveTrackPlaybackData(const ID& partId, const std::string& instrumentId);
    void triggerEventsForItems(const std::vector<const EngravingItem*>& items);

    void triggerMetronome(int tick);

    InstrumentTrackIdSet existingTrackIdSet() const;
    async::Channel<InstrumentTrackId> trackAdded() const;
    async::Channel<InstrumentTrackId> trackRemoved() const;

private:
    static const InstrumentTrackId METRONOME_TRACK_ID;
    static const InstrumentTrackId CHORD_SYMBOLS_TRACK_ID;

    using ChangedTrackIdSet = InstrumentTrackIdSet;

    struct TickBoundaries
    {
        int tickFrom = -1;
        int tickTo = -1;
    };

    struct TrackBoundaries
    {
        track_idx_t trackFrom = mu::nidx;
        track_idx_t trackTo = mu::nidx;
    };

    InstrumentTrackId idKey(const EngravingItem* item) const;
    InstrumentTrackId idKey(const std::vector<const EngravingItem*>& items) const;
    InstrumentTrackId idKey(const ID& partId, const std::string& instrumentId) const;

    void update(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                ChangedTrackIdSet* trackChanges = nullptr);
    void updateSetupData();
    void updateContext(const track_idx_t trackFrom, const track_idx_t trackTo);
    void updateContext(const InstrumentTrackId& trackId);
    void updateEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                      ChangedTrackIdSet* trackChanges = nullptr);

    void processSegment(const int tickPositionOffset, const Segment* segment, const std::set<staff_idx_t>& changedStaffIdSet,
                        ChangedTrackIdSet* trackChanges);

    bool hasToReloadTracks(const ScoreChangesRange& changesRange) const;
    bool hasToReloadScore(const std::unordered_set<ElementType>& changedTypes) const;

    bool containsTrack(const InstrumentTrackId& trackId) const;
    void clearExpiredTracks();
    void clearExpiredContexts(const track_idx_t trackFrom, const track_idx_t trackTo);
    void clearExpiredEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo);
    void collectChangesTracks(const InstrumentTrackId& trackId, ChangedTrackIdSet* result);
    void notifyAboutChanges(const InstrumentTrackIdSet& oldTracks, const InstrumentTrackIdSet& changedTracks);

    void removeEventsFromRange(const track_idx_t trackFrom, const track_idx_t trackTo, const mpe::timestamp_t timestampFrom = -1,
                               const mpe::timestamp_t timestampTo = -1);
    void removeTrackEvents(const InstrumentTrackId& trackId, const mpe::timestamp_t timestampFrom = -1,
                           const mpe::timestamp_t timestampTo = -1);

    TrackBoundaries trackBoundaries(const ScoreChangesRange& changesRange) const;
    TickBoundaries tickBoundaries(const ScoreChangesRange& changesRange) const;

    const RepeatList& repeatList() const;

    std::vector<const EngravingItem*> filterPlaybleItems(const std::vector<const EngravingItem*>& items) const;

    mpe::ArticulationsProfilePtr defaultActiculationProfile(const InstrumentTrackId& trackId) const;

    Score* m_score = nullptr;
    bool m_expandRepeats = true;
    bool m_playChordSymbols = true;

    PlaybackEventsRenderer m_renderer;
    PlaybackSetupDataResolver m_setupResolver;

    std::unordered_map<InstrumentTrackId, PlaybackContext> m_playbackCtxMap;
    std::unordered_map<InstrumentTrackId, mpe::PlaybackData> m_playbackDataMap;

    async::Notification m_dataChanged;
    async::Channel<InstrumentTrackId> m_trackAdded;
    async::Channel<InstrumentTrackId> m_trackRemoved;
};
}

#endif // PLAYBACKMODEL_H
