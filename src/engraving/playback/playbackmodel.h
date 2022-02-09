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
#include "id.h"
#include "modularity/ioc.h"
#include "mpe/events.h"
#include "mpe/iarticulationprofilesrepository.h"

#include "types/types.h"
#include "playbackeventsrenderer.h"
#include "playbacksetupdataresolver.h"
#include "playbackcontext.h"

namespace Ms {
class Score;
class Note;
class EngravingItem;
class Segment;
class Instrument;
}

namespace mu::engraving {
class PlaybackModel : public async::Asyncable
{
    INJECT(engraving, mpe::IArticulationProfilesRepository, profilesRepository)

public:
    void load(Ms::Score* score, async::Channel<int, int, int, int> notationChangesRangeChannel);

    const mpe::PlaybackData& trackPlaybackData(const ID& partId, const std::string& instrumentId) const;
    const mpe::PlaybackData& metronomePlaybackData() const;
    void triggerEventsForItem(const Ms::EngravingItem* item);

private:
    struct TrackIdKey {
        ID partId = 0;
        std::string instrumentId;

        bool operator ==(const TrackIdKey& other) const
        {
            return partId == other.partId && instrumentId == other.instrumentId;
        }

        bool operator <(const TrackIdKey& other) const noexcept
        {
            return partId < other.partId
                   && instrumentId < other.instrumentId;
        }
    };

    static const TrackIdKey METRONOME_TRACK_ID;

    struct IdKeyHash {
        std::size_t operator()(const TrackIdKey& s) const noexcept
        {
            std::size_t h1 = std::hash<int> {}(s.partId.toUint64());
            std::size_t h2 = std::hash<std::string> {}(s.instrumentId);
            return h1 ^ (h2 << 1);
        }
    };

    using TrackChangesMap = std::unordered_map<TrackIdKey, std::vector<mpe::timestamp_t>, IdKeyHash>;

    TrackIdKey idKey(const Ms::EngravingItem* item) const;
    TrackIdKey idKey(const ID& partId, const std::string& instrimentId) const;

    void update(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo, TrackChangesMap* trackChanges = nullptr);
    void updateSetupData();
    void updateEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                      TrackChangesMap* trackChanges = nullptr);
    void clearExpiredTracks();
    void clearExpiredContexts();
    void collectChangesTimestamps(const TrackIdKey& trackId, const int positionTick, const int tickPositionOffset, TrackChangesMap* result);
    void notifyAboutChanges(TrackChangesMap&& trackChanges);

    void findEventsForNote(const Ms::Note* note, const mpe::PlaybackEventList& sourceEvents, mpe::PlaybackEventList& result) const;

    Ms::Score* m_score = nullptr;

    PlaybackEventsRenderer m_renderer;
    PlaybackSetupDataResolver m_setupResolver;

    std::unordered_map<TrackIdKey, PlaybackContext, IdKeyHash> m_playbackCtxMap;
    std::unordered_map<TrackIdKey, mpe::PlaybackData, IdKeyHash> m_playbackDataMap;
};
}

#endif // PLAYBACKMODEL_H
