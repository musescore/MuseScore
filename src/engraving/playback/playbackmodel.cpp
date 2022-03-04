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

const InstrumentTrackId PlaybackModel::METRONOME_TRACK_ID = { 999, METRONOME_INSTRUMENT_ID };

void PlaybackModel::load(Ms::Score* score, async::Channel<int, int, int, int> notationChangesRangeChannel)
{
    if (!score || score->measures()->empty() || !score->lastMeasure()) {
        return;
    }

    m_score = score;
    m_score->tempoChanged().onNotify(this, [this](){
        reload();
    });

    notationChangesRangeChannel.resetOnReceive(this);

    notationChangesRangeChannel.onReceive(this, [this](const int tickFrom, const int tickTo,
                                                       const int staffIdxFrom, const int staffIdxTo) {
        int trackFrom = Ms::staff2track(staffIdxFrom, 0);
        int trackTo = Ms::staff2track(staffIdxTo, Ms::VOICES);

        clearExpiredTracks();
        clearExpiredContexts();
        clearExpiredEvents(tickFrom, tickTo, trackFrom, trackTo);

        ChangedTrackIdSet trackChanges;
        update(tickFrom, tickTo, trackFrom, trackTo, &trackChanges);
        notifyAboutChanges(std::move(trackChanges));
    });

    update(0, m_score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());
    m_dataChanged.notify();
}

void PlaybackModel::reload()
{
    clearExpiredTracks();
    clearExpiredContexts();

    for (auto& pair : m_playbackDataMap) {
        pair.second.originEvents.clear();
    }

    update(0, m_score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());

    for (auto& pair : m_playbackDataMap) {
        pair.second.mainStream.send(pair.second.originEvents);
    }

    m_dataChanged.notify();
}

Notification PlaybackModel::dataChanged() const
{
    return m_dataChanged;
}

bool PlaybackModel::isPlayRepeatsEnabled() const
{
    return m_expandRepeats;
}

void PlaybackModel::setPlayRepeats(const bool isEnabled)
{
    m_expandRepeats = isEnabled;
}

const InstrumentTrackId& PlaybackModel::metronomeTrackId() const
{
    return METRONOME_TRACK_ID;
}

const PlaybackData& PlaybackModel::resolveTrackPlaybackData(const InstrumentTrackId& trackId)
{
    auto search = m_playbackDataMap.find(trackId);

    if (search != m_playbackDataMap.cend()) {
        return search->second;
    }

    const Ms::Part* part = m_score->partById(trackId.partId.toUint64());

    if (!part) {
        static PlaybackData empty;
        return empty;
    }

    update(0, m_score->lastMeasure()->tick().ticks(), part->startTrack(), part->endTrack());

    return m_playbackDataMap[trackId];
}

const PlaybackData& PlaybackModel::resolveTrackPlaybackData(const ID& partId, const std::string& instrumentId)
{
    return resolveTrackPlaybackData(idKey(partId, instrumentId));
}

void PlaybackModel::triggerEventsForItem(const Ms::EngravingItem* item)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    if (!item->isPlayable()) {
        return;
    }

    InstrumentTrackId trackId = idKey(item);

    auto trackPlaybackData = m_playbackDataMap.find(trackId);
    if (trackPlaybackData == m_playbackDataMap.cend()) {
        return;
    }

    int utick = repeatList().tick2utick(item->tick().ticks());
    timestamp_t timestamp = timestampFromTicks(item->score(), utick);

    auto eventsFromTick = trackPlaybackData->second.originEvents.find(timestamp);
    if (eventsFromTick == trackPlaybackData->second.originEvents.cend()) {
        return;
    }

    PlaybackEventsMap result;

    if (item->isChord()) {
        for (const Ms::Note* note : Ms::toChord(item)->notes()) {
            findEventsForNote(note, eventsFromTick->second, result[timestamp]);
        }
    } else {
        findEventsForNote(Ms::toNote(item), eventsFromTick->second, result[timestamp]);
    }

    for (PlaybackEvent& event : result[timestamp]) {
        if (!std::holds_alternative<mpe::NoteEvent>(event)) {
            continue;
        }

        mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
        noteEvent.setFixedDuration(Ms::MScore::defaultPlayDuration);
    }

    trackPlaybackData->second.offStream.send(std::move(result));
}

void PlaybackModel::update(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                           ChangedTrackIdSet* trackChanges)
{
    updateSetupData();
    updateEvents(tickFrom, tickTo, trackFrom, trackTo, trackChanges);
}

void PlaybackModel::updateSetupData()
{
    for (const Ms::Part* part : m_score->parts()) {
        for (const auto& pair : *part->instruments()) {
            InstrumentTrackId trackId = idKey(part->id(), pair.second->id().toStdString());

            if (!trackId.isValid()) {
                continue;
            }

            m_setupResolver.resolveSetupData(pair.second, m_playbackDataMap[std::move(trackId)].setupData);
        }
    }

    m_setupResolver.resolveMetronomeSetupData(m_playbackDataMap[METRONOME_TRACK_ID].setupData);
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                                 ChangedTrackIdSet* trackChanges)
{
    for (const Ms::RepeatSegment* repeatSegment : repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        int repeatStartTick = repeatSegment->tick;
        int repeatEndTick = repeatStartTick + repeatSegment->len();

        if (repeatStartTick > tickTo || repeatEndTick <= tickFrom) {
            continue;
        }

        for (const Ms::Measure* measure : repeatSegment->measureList()) {
            int measureStartTick = measure->tick().ticks();
            int measureEndTick = measure->endTick().ticks();

            if (measureStartTick > tickTo || measureEndTick <= tickFrom) {
                continue;
            }

            for (Ms::Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType()) {
                    continue;
                }

                int segmentStartTick = segment->tick().ticks();
                int segmentEndTick = segmentStartTick + segment->ticks().ticks();

                if (segmentStartTick > tickTo || segmentEndTick <= tickFrom) {
                    continue;
                }

                for (int i = trackFrom; i < trackTo; ++i) {
                    Ms::EngravingItem* item = segment->element(i);

                    if (!item || !item->isChordRest() || !item->part()) {
                        continue;
                    }

                    InstrumentTrackId trackId = idKey(item);

                    if (!trackId.isValid()) {
                        continue;
                    }

                    PlaybackContext& ctx = m_playbackCtxMap[trackId];
                    ctx.update(segment, segmentStartTick);

                    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(m_playbackDataMap[trackId].setupData.category);
                    if (!profile) {
                        LOGE() << "unsupported instrument family: " << item->part()->id();
                        continue;
                    }

                    m_renderer.render(item, tickPositionOffset, ctx.nominalDynamicLevel(segmentStartTick),
                                      ctx.persistentArticulationType(segmentStartTick), std::move(profile),
                                      m_playbackDataMap[trackId].originEvents);

                    collectChangesTracks(trackId, trackChanges);
                }

                m_renderer.renderMetronome(m_score, segmentStartTick, segment->ticks().ticks(),
                                           tickPositionOffset, m_playbackDataMap[METRONOME_TRACK_ID].originEvents);
                collectChangesTracks(METRONOME_TRACK_ID, trackChanges);
            }
        }
    }
}

void PlaybackModel::clearExpiredTracks()
{
    auto it = m_playbackDataMap.cbegin();

    while (it != m_playbackDataMap.cend())
    {
        if (it->first == METRONOME_TRACK_ID) {
            ++it;
            continue;
        }

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
        if (it->first == METRONOME_TRACK_ID) {
            ++it;
            continue;
        }

        const Ms::Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || part->instruments()->contains(it->first.instrumentId)) {
            it = m_playbackCtxMap.erase(it);
            continue;
        }

        ++it;
    }
}

void PlaybackModel::clearExpiredEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo)
{
    timestamp_t timestampFrom = timestampFromTicks(m_score, tickFrom);
    timestamp_t timestampTo = timestampFromTicks(m_score, tickTo);

    for (const Ms::Part* part : m_score->parts()) {
        if (part->startTrack() > trackTo || part->endTrack() <= trackFrom) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            removeEvents(trackId, timestampFrom, timestampTo);
        }
    }

    removeEvents(METRONOME_TRACK_ID, timestampFrom, timestampTo);
}

void PlaybackModel::collectChangesTracks(const InstrumentTrackId& trackId, ChangedTrackIdSet* result)
{
    if (!result) {
        return;
    }

    result->insert(trackId);
}

void PlaybackModel::notifyAboutChanges(ChangedTrackIdSet&& trackChanges)
{
    for (const InstrumentTrackId& trackId : trackChanges) {
        auto search = m_playbackDataMap.find(trackId);

        if (search == m_playbackDataMap.cend()) {
            continue;
        }

        search->second.mainStream.send(search->second.originEvents);
    }

    if (!trackChanges.empty()) {
        m_dataChanged.notify();
    }
}

void PlaybackModel::removeEvents(const InstrumentTrackId& trackId, const mpe::timestamp_t timestampFrom, const mpe::timestamp_t timestampTo)
{
    PlaybackData& trackPlaybackData = m_playbackDataMap[trackId];

    auto lowerBound = trackPlaybackData.originEvents.lower_bound(timestampFrom);
    auto upperBound = trackPlaybackData.originEvents.upper_bound(timestampTo);

    for (auto it = lowerBound; it != upperBound;) {
        it = trackPlaybackData.originEvents.erase(it);
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

const Ms::RepeatList& PlaybackModel::repeatList() const
{
    return m_score->repeatList();
}

InstrumentTrackId PlaybackModel::idKey(const Ms::EngravingItem* item) const
{
    return { item->part()->id(),
             item->part()->instrumentId(item->tick()).toStdString() };
}

InstrumentTrackId PlaybackModel::idKey(const ID& partId, const std::string& instrimentId) const
{
    return { partId, instrimentId };
}
