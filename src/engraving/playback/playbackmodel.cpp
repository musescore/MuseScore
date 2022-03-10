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

void PlaybackModel::load(Ms::Score* score)
{
    if (!score || score->measures()->empty() || !score->lastMeasure()) {
        return;
    }

    m_score = score;

    auto changesChannel = score->changesChannel();
    changesChannel.resetOnReceive(this);

    changesChannel.onReceive(this, [this](const Ms::ScoreChangesRange& range) {
        int trackFrom = Ms::staff2track(range.staffIdxFrom, 0);
        int trackTo = Ms::staff2track(range.staffIdxTo, Ms::VOICES);

        int tickRangeFrom = range.tickFrom;
        int tickRangeTo = range.tickTo;

        if (hasToReloadTracks(range.changedTypes)) {
            tickRangeFrom = 0;
            tickRangeTo = m_score->lastMeasure()->endTick().ticks();
        } else if (hasToReloadScore(range.changedTypes)) {
            tickRangeFrom = 0;
            tickRangeTo = m_score->lastMeasure()->endTick().ticks();

            trackFrom = 0;
            trackTo = m_score->ntracks();
        }

        clearExpiredTracks();
        clearExpiredContexts(trackFrom, trackTo);
        clearExpiredEvents(tickRangeFrom, tickRangeTo, trackFrom, trackTo);

        ChangedTrackIdSet trackChanges;
        update(tickRangeFrom, tickRangeTo, trackFrom, trackTo, &trackChanges);
        notifyAboutChanges(std::move(trackChanges));
    });

    update(0, m_score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());
    m_dataChanged.notify();
}

void PlaybackModel::reload()
{
    int trackFrom = 0;
    int trackTo = m_score->ntracks();

    int tickFrom = 0;
    int tickTo = m_score->lastMeasure()->endTick().ticks();

    clearExpiredTracks();
    clearExpiredContexts(trackFrom, trackTo);

    for (auto& pair : m_playbackDataMap) {
        pair.second.originEvents.clear();
    }

    update(tickFrom, tickTo, trackFrom, trackTo);

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
    updateContext(trackFrom, trackTo);
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

void PlaybackModel::updateContext(const int trackFrom, const int trackTo)
{
    for (const Ms::Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.update(trackId.partId, m_score);
        }
    }
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const int trackFrom, const int trackTo,
                                 ChangedTrackIdSet* trackChanges)
{
    std::set<Ms::ID> changedPartIdSet = m_score->partIdsFromRange(trackFrom, trackTo);

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

                if (segmentStartTick > tickTo || segmentEndTick < tickFrom) {
                    continue;
                }

                for (const Ms::EngravingItem* item : segment->elist()) {
                    if (!item || !item->isChordRest() || !item->part()) {
                        continue;
                    }

                    Ms::ID partId = item->part()->id();

                    if (changedPartIdSet.find(partId) == changedPartIdSet.cend()) {
                        continue;
                    }

                    InstrumentTrackId trackId = idKey(item);

                    if (!trackId.isValid()) {
                        continue;
                    }

                    const PlaybackContext& ctx = m_playbackCtxMap[trackId];

                    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(m_playbackDataMap[trackId].setupData.category);
                    if (!profile) {
                        LOGE() << "unsupported instrument family: " << partId;
                        continue;
                    }

                    m_renderer.render(item, tickPositionOffset, ctx.nominalDynamicLevel(segmentStartTick + tickPositionOffset),
                                      ctx.persistentArticulationType(segmentStartTick + tickPositionOffset), std::move(profile),
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

bool PlaybackModel::hasToReloadTracks(const std::unordered_set<Ms::ElementType>& changedTypes) const
{
    static const std::unordered_set<Ms::ElementType> REQUIRED_TYPES = {
        Ms::ElementType::PLAYTECH_ANNOTATION, Ms::ElementType::DYNAMIC, Ms::ElementType::HAIRPIN
    };

    for (const Ms::ElementType type : REQUIRED_TYPES) {
        if (changedTypes.find(type) == changedTypes.cend()) {
            continue;
        }

        return true;
    }

    return false;
}

bool PlaybackModel::hasToReloadScore(const std::unordered_set<Ms::ElementType>& changedTypes) const
{
    static const std::unordered_set<Ms::ElementType> REQUIRED_TYPES = {
        Ms::ElementType::TEMPO_RANGED_CHANGE, Ms::ElementType::TEMPO_RANGED_CHANGE_SEGMENT, Ms::ElementType::TEMPO_TEXT
    };

    for (const Ms::ElementType type : REQUIRED_TYPES) {
        if (changedTypes.find(type) == changedTypes.cend()) {
            continue;
        }

        return true;
    }

    return false;
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

void PlaybackModel::clearExpiredContexts(const int trackFrom, const int trackTo)
{
    for (const Ms::Part* part : m_score->parts()) {
        if (part->startTrack() > trackTo || part->endTrack() <= trackFrom) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.clear();
        }
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
