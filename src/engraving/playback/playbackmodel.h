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
class RepeatList;
}

namespace mu::engraving {
class PlaybackModel : public async::Asyncable
{
    INJECT(engraving, mpe::IArticulationProfilesRepository, profilesRepository)

public:
    void load(Ms::Score* score);
    void reload();

    async::Notification dataChanged() const;

    bool isPlayRepeatsEnabled() const;
    void setPlayRepeats(const bool isEnabled);

    const InstrumentTrackId& metronomeTrackId() const;

    const mpe::PlaybackData& resolveTrackPlaybackData(const InstrumentTrackId& trackId);
    const mpe::PlaybackData& resolveTrackPlaybackData(const ID& partId, const std::string& instrumentId);
    void triggerEventsForItem(const Ms::EngravingItem* item);

private:
    static const InstrumentTrackId METRONOME_TRACK_ID;

    using ChangedTrackIdSet = InstrumentTrackIdSet;

    struct TickBoundaries
    {
        int tickFrom = -1;
        int tickTo = -1;
    };

    struct TrackBoundaries
    {
        int trackFrom = -1;
        int trackTo = -1;
    };

    InstrumentTrackId idKey(const Ms::EngravingItem* item) const;
    InstrumentTrackId idKey(const ID& partId, const std::string& instrimentId) const;

    void update(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo, ChangedTrackIdSet* trackChanges = nullptr);
    void updateSetupData();
    void updateContext(const int trackFrom, const int trackTo);
    void updateEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                      ChangedTrackIdSet* trackChanges = nullptr);

    bool hasToReloadTracks(const std::unordered_set<Ms::ElementType>& changedTypes) const;
    bool hasToReloadScore(const std::unordered_set<Ms::ElementType>& changedTypes) const;

    void clearExpiredTracks();
    void clearExpiredContexts(const int trackFrom, const int trackTo);
    void clearExpiredEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo);
    void collectChangesTracks(const InstrumentTrackId& trackId, ChangedTrackIdSet* result);
    void notifyAboutChanges(ChangedTrackIdSet&& trackChanges);

    void removeEvents(const InstrumentTrackId& trackId, const mpe::timestamp_t timestampFrom, const mpe::timestamp_t timestampTo);

    TrackBoundaries trackBoundaries(const Ms::ScoreChangesRange& changesRange) const;
    TickBoundaries tickBoundaries(const Ms::ScoreChangesRange& changesRange) const;

    const Ms::RepeatList& repeatList() const;

    Ms::Score* m_score = nullptr;
    bool m_expandRepeats = true;

    PlaybackEventsRenderer m_renderer;
    PlaybackSetupDataResolver m_setupResolver;

    std::unordered_map<InstrumentTrackId, PlaybackContext> m_playbackCtxMap;
    std::unordered_map<InstrumentTrackId, mpe::PlaybackData> m_playbackDataMap;

    async::Notification m_dataChanged;
};
}

#endif // PLAYBACKMODEL_H
