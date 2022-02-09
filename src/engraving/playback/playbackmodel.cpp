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

#include "playbackmodel.h"

#include <QString>

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/instrument.h"

#include "utils/pitchutils.h"

using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu::async;

static const std::string METRONOME_INSTRUMENT_ID("metronome");

const PlaybackModel::TrackIdKey PlaybackModel::METRONOME_TRACK_ID = { 999, METRONOME_INSTRUMENT_ID };

void PlaybackModel::load(Ms::Score* score, async::Channel<int, int, int, int> notationChangesRangeChannel)
{
    m_score = score;
    notationChangesRangeChannel.resetOnReceive(this);

    notationChangesRangeChannel.onReceive(this, [this](const int tickFrom, const int tickTo,
                                                       const int staffIdxFrom, const int staffIdxTo) {
        clearExpiredTracks();
        clearExpiredContexts();

        TrackChangesMap trackChanges;
        update(tickFrom, tickTo, Ms::staff2track(staffIdxFrom, 0), Ms::staff2track(staffIdxTo, Ms::VOICES), &trackChanges);
        notifyAboutChanges(std::move(trackChanges));
    });

    update(0, score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());
}

const PlaybackData& PlaybackModel::trackPlaybackData(const ID& partId, const std::string& instrumentId) const
{
    return m_playbackDataMap.at(idKey(partId, instrumentId));
}

const PlaybackData& PlaybackModel::metronomePlaybackData() const
{
    return m_playbackDataMap.at(METRONOME_TRACK_ID);
}

void PlaybackModel::triggerEventsForItem(const Ms::EngravingItem* item)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    if (!item->isPlayable()) {
        return;
    }

    TrackIdKey trackId = idKey(item);

    auto trackPlaybackData = m_playbackDataMap.find(trackId);
    if (trackPlaybackData == m_playbackDataMap.cend()) {
        return;
    }

    timestamp_t timestamp = timestampFromTicks(item->score(), item->tick().ticks());

    const PlaybackEventList& eventsFromTick = trackPlaybackData->second.originEvents.at(timestamp);

    PlaybackEventsMap result;

    if (item->isChord()) {
        for (const Ms::Note* note : Ms::toChord(item)->notes()) {
            findEventsForNote(note, eventsFromTick, result[timestamp]);
        }
    } else {
        findEventsForNote(Ms::toNote(item), eventsFromTick, result[timestamp]);
    }

    trackPlaybackData->second.offStream.send(std::move(result));
}

void PlaybackModel::update(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                           TrackChangesMap* trackChanges)
{
    updateSetupData();
    updateEvents(tickFrom, tickTo, trackFrom, trackTo, trackChanges);
}

void PlaybackModel::updateSetupData()
{
    for (const Ms::Part* part : m_score->parts()) {
        for (const auto& pair : *part->instruments()) {
            TrackIdKey trackId = idKey(part->id(), pair.second->id().toStdString());
            m_setupResolver.resolveSetupData(pair.second, m_playbackDataMap[std::move(trackId)].setupData);
        }
    }
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                                 TrackChangesMap* trackChanges)
{
    for (const Ms::RepeatSegment* repeatSegment : m_score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        int repeatStartTick = repeatSegment->tick;
        int repeatEndTick = repeatStartTick + repeatSegment->len();

        if (repeatStartTick >= tickTo || repeatEndTick <= tickFrom) {
            continue;
        }

        for (const Ms::Measure* measure : repeatSegment->measureList()) {
            int measureStartTick = measure->tick().ticks();
            int measureEndTick = measure->endTick().ticks();

            if (measureStartTick >= tickTo || measureEndTick <= tickFrom) {
                continue;
            }

            for (Ms::Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType()) {
                    continue;
                }

                int segmentPositionTick = segment->tick().ticks();

                for (int i = trackFrom; i < trackTo; ++i) {
                    Ms::EngravingItem* item = segment->element(i);

                    if (!item || !item->isChordRest() || !item->part()) {
                        continue;
                    }

                    TrackIdKey trackId = idKey(item);

                    PlaybackContext& ctx = m_playbackCtxMap[trackId];
                    ctx.update(segment, segmentPositionTick);

                    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(m_playbackDataMap[trackId].setupData.category);
                    if (!profile) {
                        LOGE() << "unsupported instrument family: " << item->part()->id();
                        continue;
                    }

                    m_renderer.render(item, tickPositionOffset, ctx.nominalDynamicLevel(segmentPositionTick),
                                      ctx.persistentArticulationType(segmentPositionTick), std::move(profile),
                                      m_playbackDataMap[trackId].originEvents);

                    collectChangesTimestamps(trackId, item->tick().ticks(), tickPositionOffset, trackChanges);
                }

                m_renderer.renderMetronome(m_score, segmentPositionTick, segment->ticks().ticks(),
                                           tickPositionOffset, m_playbackDataMap[METRONOME_TRACK_ID].originEvents);
                collectChangesTimestamps(METRONOME_TRACK_ID, segmentPositionTick, tickPositionOffset, trackChanges);
            }
        }
    }
}

void PlaybackModel::clearExpiredTracks()
{
    auto it = m_playbackDataMap.cbegin();

    while (it != m_playbackDataMap.cend())
    {
        const Ms::Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || part->instruments()->contains(it->first.instrumentId)) {
            it = m_playbackDataMap.erase(it);
            continue;
        }

        ++it;
    }
}

void PlaybackModel::clearExpiredContexts()
{
    auto it = m_playbackCtxMap.cbegin();

    while (it != m_playbackCtxMap.cend())
    {
        const Ms::Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || part->instruments()->contains(it->first.instrumentId)) {
            it = m_playbackCtxMap.erase(it);
            continue;
        }

        ++it;
    }
}

void PlaybackModel::collectChangesTimestamps(const TrackIdKey& trackId, const int positionTick, const int tickPositionOffset,
                                             TrackChangesMap* result)
{
    if (!result) {
        return;
    }

    std::vector<timestamp_t>& changesTimestamps = result->operator[](trackId);
    changesTimestamps.push_back(timestampFromTicks(m_score, positionTick + tickPositionOffset));
}

void PlaybackModel::notifyAboutChanges(TrackChangesMap&& trackChanges)
{
    for (const auto& pair : trackChanges) {
        PlaybackData& data = m_playbackDataMap.at(pair.first);
        const PlaybackEventsMap& events = data.originEvents;

        PlaybackEventsMap changesMap;

        for (const mpe::timestamp_t& timestamp : pair.second) {
            changesMap.emplace(timestamp, events.at(timestamp));
        }

        data.mainStream.send(std::move(changesMap));
    }
}

void PlaybackModel::findEventsForNote(const Ms::Note* note, const mpe::PlaybackEventList& sourceEvents,
                                      mpe::PlaybackEventList& result) const
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    pitch_level_t pitchLevel = notePitchLevel(note->tpc(), note->octave());

    for (const PlaybackEvent& event : sourceEvents) {
        if (!std::holds_alternative<mpe::NoteEvent>(event)) {
            continue;
        }

        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

        if (noteEvent.pitchCtx().nominalPitchLevel == pitchLevel) {
            result.emplace_back(event);
            return;
        }
    }
}

PlaybackModel::TrackIdKey PlaybackModel::idKey(const Ms::EngravingItem* item) const
{
    return { item->part()->id(),
             item->part()->instrumentId(item->tick()).toStdString() };
}

PlaybackModel::TrackIdKey PlaybackModel::idKey(const ID& partId, const std::string& instrimentId) const
{
    return { partId, instrimentId };
}
