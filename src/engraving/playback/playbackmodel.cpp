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

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/instrument.h"
#include "libmscore/fret.h"

#include "utils/pitchutils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu::async;

static const std::string METRONOME_INSTRUMENT_ID("metronome");
static const std::string CHORD_SYMBOLS_INSTRUMENT_ID("chord_symbols");

const InstrumentTrackId PlaybackModel::METRONOME_TRACK_ID = { 999, METRONOME_INSTRUMENT_ID };
const InstrumentTrackId PlaybackModel::CHORD_SYMBOLS_TRACK_ID = { 1000, CHORD_SYMBOLS_INSTRUMENT_ID };

static const Harmony* findChordSymbols(const EngravingItem* item)
{
    if (item->isHarmony()) {
        return toHarmony(item);
    } else if (item->isFretDiagram()) {
        return toFretDiagram(item)->harmony();
    }

    return nullptr;
}

void PlaybackModel::load(Score* score)
{
    if (!score || score->measures()->empty() || !score->lastMeasure()) {
        return;
    }

    m_score = score;

    auto changesChannel = score->changesChannel();
    changesChannel.resetOnReceive(this);

    changesChannel.onReceive(this, [this](const ScoreChangesRange& range) {
        TickBoundaries tickRange = tickBoundaries(range);
        TrackBoundaries trackRange = trackBoundaries(range);

        clearExpiredTracks();
        clearExpiredContexts(trackRange.trackFrom, trackRange.trackTo);
        clearExpiredEvents(tickRange.tickFrom, tickRange.tickTo, trackRange.trackFrom, trackRange.trackTo);

        InstrumentTrackIdSet existingTracks = existingTrackIdSet();
        ChangedTrackIdSet trackChanges;
        update(tickRange.tickFrom, tickRange.tickTo, trackRange.trackFrom, trackRange.trackTo, &trackChanges);
        notifyAboutChanges(std::move(trackChanges), std::move(existingTracks));
    });

    update(0, m_score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());

    for (const auto& pair : m_playbackDataMap) {
        m_trackAdded.send(pair.first);
    }

    m_dataChanged.notify();
}

void PlaybackModel::reload()
{
    int trackFrom = 0;
    size_t trackTo = m_score->ntracks();

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

const InstrumentTrackId& PlaybackModel::chordSymbolsTrackId() const
{
    return CHORD_SYMBOLS_TRACK_ID;
}

const PlaybackData& PlaybackModel::resolveTrackPlaybackData(const InstrumentTrackId& trackId)
{
    auto search = m_playbackDataMap.find(trackId);

    if (search != m_playbackDataMap.cend()) {
        return search->second;
    }

    const Part* part = m_score ? m_score->partById(trackId.partId.toUint64()) : nullptr;

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

void PlaybackModel::triggerEventsForItem(const EngravingItem* item)
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

    if (item->isHarmony()) {
        const Harmony* chordSymbol = toHarmony(item);

        if (chordSymbol->isRealizable()) {
            PlaybackEventsMap result;
            m_renderer.renderChordSymbols(chordSymbol, 0 /*ticksPositionOffset*/, result);
            trackPlaybackData->second.offStream.send(std::move(result));
        }

        return;
    }

    int utick = repeatList().tick2utick(item->tick().ticks());
    timestamp_t actualTimestamp = timestampFromTicks(item->score(), utick);
    duration_t actualDuration = MScore::defaultPlayDuration;
    dynamic_level_t actualDynamicLevel = dynamicLevelFromType(mpe::DynamicType::Natural);

    const PlaybackContext& ctx = m_playbackCtxMap[trackId];

    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(m_playbackDataMap[trackId].setupData.category);
    if (!profile) {
        LOGE() << "unsupported instrument family: " << trackId.partId.toUint64();
        return;
    }

    PlaybackEventsMap result;
    m_renderer.render(item, actualTimestamp, actualDuration, actualDynamicLevel, ctx.persistentArticulationType(utick), profile, result);

    trackPlaybackData->second.offStream.send(std::move(result));
}

async::Channel<InstrumentTrackId> PlaybackModel::trackAdded() const
{
    return m_trackAdded;
}

async::Channel<InstrumentTrackId> PlaybackModel::trackRemoved() const
{
    return m_trackRemoved;
}

void PlaybackModel::update(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                           ChangedTrackIdSet* trackChanges)
{
    updateSetupData();
    updateContext(trackFrom, trackTo);
    updateEvents(tickFrom, tickTo, trackFrom, trackTo, trackChanges);
}

void PlaybackModel::updateSetupData()
{
    for (const Part* part : m_score->parts()) {
        for (const auto& pair : part->instruments()) {
            InstrumentTrackId trackId = idKey(part->id(), pair.second->id().toStdString());

            if (!trackId.isValid() || containsTrack(trackId)) {
                continue;
            }

            m_setupResolver.resolveSetupData(pair.second, m_playbackDataMap[trackId].setupData);
        }
    }

    m_setupResolver.resolveMetronomeSetupData(m_playbackDataMap[METRONOME_TRACK_ID].setupData);
    m_setupResolver.resolveChordSymbolsSetupData(m_playbackDataMap[CHORD_SYMBOLS_TRACK_ID].setupData);
}

void PlaybackModel::updateContext(const track_idx_t trackFrom, const track_idx_t trackTo)
{
    for (const Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.update(trackId.partId, m_score);

            PlaybackData& trackData = m_playbackDataMap[trackId];
            trackData.dynamicLevelMap = ctx.dynamicLevelMap(m_score);
        }
    }
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                                 ChangedTrackIdSet* trackChanges)
{
    std::set<ID> changedPartIdSet = m_score->partIdsFromRange(trackFrom, trackTo);

    for (const RepeatSegment* repeatSegment : repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        int repeatStartTick = repeatSegment->tick;
        int repeatEndTick = repeatStartTick + repeatSegment->len();

        if (repeatStartTick > tickTo || repeatEndTick <= tickFrom) {
            continue;
        }

        for (const Measure* measure : repeatSegment->measureList()) {
            int measureStartTick = measure->tick().ticks();
            int measureEndTick = measure->endTick().ticks();

            if (measureStartTick > tickTo || measureEndTick <= tickFrom) {
                continue;
            }

            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType()) {
                    continue;
                }

                int segmentStartTick = segment->tick().ticks();
                int segmentEndTick = segmentStartTick + segment->ticks().ticks();

                if (segmentStartTick > tickTo || segmentEndTick <= tickFrom) {
                    continue;
                }

                for (const EngravingItem* item : segment->annotations()) {
                    if (!item || !item->part()) {
                        continue;
                    }

                    const Harmony* chordSymbol = findChordSymbols(item);
                    if (!chordSymbol || !chordSymbol->isRealizable()) {
                        continue;
                    }

                    ID partId = item->part()->id();
                    if (changedPartIdSet.find(partId) == changedPartIdSet.cend()) {
                        return;
                    }

                    m_renderer.renderChordSymbols(chordSymbol, tickPositionOffset, m_playbackDataMap[CHORD_SYMBOLS_TRACK_ID].originEvents);
                    collectChangesTracks(CHORD_SYMBOLS_TRACK_ID, trackChanges);
                }

                for (const EngravingItem* item : segment->elist()) {
                    if (!item || !item->isChordRest() || !item->part()) {
                        continue;
                    }

                    ID partId = item->part()->id();

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

                    m_renderer.render(item, tickPositionOffset, ctx.appliableDynamicLevel(segmentStartTick + tickPositionOffset),
                                      ctx.persistentArticulationType(segmentStartTick + tickPositionOffset), std::move(profile),
                                      m_playbackDataMap[trackId].originEvents);

                    collectChangesTracks(trackId, trackChanges);
                }
            }

            m_renderer.renderMetronome(m_score, measureStartTick, measureEndTick, tickPositionOffset,
                                       m_playbackDataMap[METRONOME_TRACK_ID].originEvents);
            collectChangesTracks(METRONOME_TRACK_ID, trackChanges);
        }
    }
}

bool PlaybackModel::hasToReloadTracks(const std::unordered_set<ElementType>& changedTypes) const
{
    static const std::unordered_set<ElementType> REQUIRED_TYPES = {
        ElementType::PLAYTECH_ANNOTATION, ElementType::DYNAMIC, ElementType::HAIRPIN,
        ElementType::HAIRPIN_SEGMENT
    };

    for (const ElementType type : REQUIRED_TYPES) {
        if (changedTypes.find(type) == changedTypes.cend()) {
            continue;
        }

        return true;
    }

    return false;
}

bool PlaybackModel::hasToReloadScore(const std::unordered_set<ElementType>& changedTypes) const
{
    static const std::unordered_set<ElementType> REQUIRED_TYPES = {
        ElementType::GRADUAL_TEMPO_CHANGE, ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT,
        ElementType::TEMPO_TEXT,
        ElementType::LAYOUT_BREAK, ElementType::FERMATA,
        ElementType::MEASURE_REPEAT, ElementType::VOLTA,
        ElementType::VOLTA_SEGMENT, ElementType::SYSTEM_TEXT,
        ElementType::JUMP, ElementType::MARKER
    };

    for (const ElementType type : REQUIRED_TYPES) {
        if (changedTypes.find(type) == changedTypes.cend()) {
            continue;
        }

        return true;
    }

    return false;
}

bool PlaybackModel::containsTrack(const InstrumentTrackId& trackId) const
{
    return m_playbackDataMap.find(trackId) != m_playbackDataMap.cend();
}

void PlaybackModel::clearExpiredTracks()
{
    auto it = m_playbackDataMap.cbegin();

    while (it != m_playbackDataMap.cend()) {
        if (it->first == METRONOME_TRACK_ID || it->first == CHORD_SYMBOLS_TRACK_ID) {
            ++it;
            continue;
        }

        const Part* part = m_score->partById(it->first.partId.toUint64());

        if (!part || !part->instruments().contains(it->first.instrumentId)) {
            m_trackRemoved.send(it->first);
            it = m_playbackDataMap.erase(it);
            continue;
        }

        ++it;
    }
}

void PlaybackModel::clearExpiredContexts(const track_idx_t trackFrom, const track_idx_t trackTo)
{
    for (const Part* part : m_score->parts()) {
        if (part->startTrack() > trackTo || part->endTrack() <= trackFrom) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.clear();
        }
    }
}

void PlaybackModel::clearExpiredEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo)
{
    timestamp_t timestampFrom = timestampFromTicks(m_score, tickFrom);
    timestamp_t timestampTo = timestampFromTicks(m_score, tickTo);

    for (const Part* part : m_score->parts()) {
        if (part->startTrack() > trackTo || part->endTrack() <= trackFrom) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            removeEvents(trackId, timestampFrom, timestampTo);
        }
    }

    removeEvents(METRONOME_TRACK_ID, timestampFrom, timestampTo);
    removeEvents(CHORD_SYMBOLS_TRACK_ID, timestampFrom, timestampTo);
}

void PlaybackModel::collectChangesTracks(const InstrumentTrackId& trackId, ChangedTrackIdSet* result)
{
    if (!result) {
        return;
    }

    result->insert(trackId);
}

void PlaybackModel::notifyAboutChanges(ChangedTrackIdSet&& trackChanges, InstrumentTrackIdSet&& existingTracks)
{
    for (const InstrumentTrackId& trackId : trackChanges) {
        auto search = m_playbackDataMap.find(trackId);

        if (search == m_playbackDataMap.cend()) {
            continue;
        }

        search->second.mainStream.send(search->second.originEvents);
        search->second.dynamicLevelChanges.send(search->second.dynamicLevelMap);

        if (existingTracks.find(trackId) == existingTracks.cend()) {
            m_trackAdded.send(trackId);
        }
    }

    if (!trackChanges.empty()) {
        m_dataChanged.notify();
    }
}

void PlaybackModel::removeEvents(const InstrumentTrackId& trackId, const mpe::timestamp_t timestampFrom, const mpe::timestamp_t timestampTo)
{
    auto search = m_playbackDataMap.find(trackId);

    if (search == m_playbackDataMap.cend()) {
        return;
    }

    PlaybackData& trackPlaybackData = search->second;

    PlaybackEventsMap::const_iterator lowerBound;

    if (timestampFrom == 0) {
        //!Note Some events might be started RIGHT before the "official" start of the track
        //!     Need to make sure that we don't miss those events
        lowerBound = trackPlaybackData.originEvents.begin();
    } else {
        lowerBound = trackPlaybackData.originEvents.lower_bound(timestampFrom);
    }

    auto upperBound = trackPlaybackData.originEvents.upper_bound(timestampTo);

    for (auto it = lowerBound; it != upperBound;) {
        it = trackPlaybackData.originEvents.erase(it);
    }
}

PlaybackModel::TrackBoundaries PlaybackModel::trackBoundaries(const ScoreChangesRange& changesRange) const
{
    TrackBoundaries result;

    result.trackFrom = staff2track(changesRange.staffIdxFrom, 0);
    result.trackTo = staff2track(changesRange.staffIdxTo, VOICES);

    if (hasToReloadScore(changesRange.changedTypes) || !changesRange.isValidBoundary()) {
        result.trackFrom = 0;
        result.trackTo = m_score->ntracks();
    }

    return result;
}

PlaybackModel::TickBoundaries PlaybackModel::tickBoundaries(const ScoreChangesRange& changesRange) const
{
    TickBoundaries result;

    result.tickFrom = changesRange.tickFrom;
    result.tickTo = changesRange.tickTo;

    if (hasToReloadTracks(changesRange.changedTypes)
        || hasToReloadScore(changesRange.changedTypes)
        || !changesRange.isValidBoundary()) {
        const Measure* lastMeasure = m_score->lastMeasure();
        result.tickFrom = 0;
        result.tickTo = lastMeasure ? lastMeasure->endTick().ticks() : 0;
    }

    return result;
}

const RepeatList& PlaybackModel::repeatList() const
{
    return m_score->repeatList();
}

InstrumentTrackId PlaybackModel::idKey(const EngravingItem* item) const
{
    if (item->isHarmony()) {
        return CHORD_SYMBOLS_TRACK_ID;
    }

    return { item->part()->id(),
             item->part()->instrumentId(item->tick()).toStdString() };
}

InstrumentTrackId PlaybackModel::idKey(const ID& partId, const std::string& instrumentId) const
{
    return { partId, instrumentId };
}

InstrumentTrackIdSet PlaybackModel::existingTrackIdSet() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_playbackDataMap) {
        result.insert(pair.first);
    }

    return result;
}
