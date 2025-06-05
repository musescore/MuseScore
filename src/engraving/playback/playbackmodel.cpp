/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/fret.h"
#include "dom/instrument.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/part.h"
#include "dom/staff.h"
#include "dom/repeatlist.h"
#include "dom/segment.h"
#include "dom/tempo.h"
#include "dom/tie.h"
#include "dom/tremolotwochord.h"

#include "log.h"

#include <limits>

using namespace mu;
using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse::async;

static const String METRONOME_INSTRUMENT_ID(u"metronome");
static const String CHORD_SYMBOLS_INSTRUMENT_ID(u"chord_symbols");

const InstrumentTrackId PlaybackModel::METRONOME_TRACK_ID = { 999, METRONOME_INSTRUMENT_ID };

static const Harmony* findChordSymbol(const EngravingItem* item)
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
    TRACEFUNC;

    if (!score || score->measures()->empty() || !score->lastMeasure()) {
        return;
    }

    m_score = score;

    auto changesChannel = score->changesChannel();
    changesChannel.resetOnReceive(this);

    changesChannel.onReceive(this, [this](const ScoreChangesRange& range) {
        if (!range.isValid()) {
            return;
        }

        TickBoundaries tickRange = tickBoundaries(range);
        TrackBoundaries trackRange = trackBoundaries(range);

        clearExpiredTracks();
        clearExpiredContexts(trackRange.trackFrom, trackRange.trackTo);
        clearExpiredEvents(tickRange.tickFrom, tickRange.tickTo, trackRange.trackFrom, trackRange.trackTo);

        InstrumentTrackIdSet oldTracks = existingTrackIdSet();

        ChangedTrackIdSet trackChanges;
        update(tickRange.tickFrom, tickRange.tickTo, trackRange.trackFrom, trackRange.trackTo, &trackChanges);

        notifyAboutChanges(oldTracks, trackChanges);
    });

    update(0, m_score->lastMeasure()->endTick().ticks(), 0, m_score->ntracks());

    for (const auto& pair : m_playbackDataMap) {
        m_trackAdded.send(pair.first);
    }

    m_dataChanged.notify();
}

void PlaybackModel::reload()
{
    TRACEFUNC;

    int trackFrom = 0;
    size_t trackTo = m_score->ntracks();

    const Measure* lastMeasure = m_score->lastMeasure();

    int tickFrom = 0;
    int tickTo = lastMeasure ? lastMeasure->endTick().ticks() : 0;

    clearExpiredTracks();
    clearExpiredContexts(trackFrom, trackTo);

    for (auto& pair : m_playbackDataMap) {
        pair.second.originEvents.clear();
    }

    update(tickFrom, tickTo, trackFrom, trackTo);

    for (auto& pair : m_playbackDataMap) {
        pair.second.mainStream.send(pair.second.originEvents, pair.second.dynamics, pair.second.params);
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

bool PlaybackModel::isPlayChordSymbolsEnabled() const
{
    return m_playChordSymbols;
}

void PlaybackModel::setPlayChordSymbols(const bool isEnabled)
{
    m_playChordSymbols = isEnabled;
}

bool PlaybackModel::isMetronomeEnabled() const
{
    return m_metronomeEnabled;
}

void PlaybackModel::setIsMetronomeEnabled(const bool isEnabled)
{
    if (m_metronomeEnabled == isEnabled) {
        return;
    }

    m_metronomeEnabled = isEnabled;
    reloadMetronomeEvents();
}

const InstrumentTrackId& PlaybackModel::metronomeTrackId() const
{
    return METRONOME_TRACK_ID;
}

InstrumentTrackId PlaybackModel::chordSymbolsTrackId(const ID& partId) const
{
    return { partId, CHORD_SYMBOLS_INSTRUMENT_ID };
}

bool PlaybackModel::isChordSymbolsTrack(const InstrumentTrackId& trackId) const
{
    return trackId == chordSymbolsTrackId(trackId.partId);
}

bool PlaybackModel::hasSoundFlags(const InstrumentTrackId& trackId) const
{
    auto search = m_playbackCtxMap.find(trackId);

    if (search == m_playbackCtxMap.cend()) {
        return false;
    }

    return search->second->hasSoundFlags();
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

const PlaybackData& PlaybackModel::resolveTrackPlaybackData(const ID& partId, const String& instrumentId)
{
    return resolveTrackPlaybackData(idKey(partId, instrumentId));
}

void PlaybackModel::triggerEventsForItems(const std::vector<const EngravingItem*>& items)
{
    std::vector<const EngravingItem*> playableItems = filterPlayableItems(items);
    if (playableItems.empty()) {
        return;
    }

    InstrumentTrackId trackId = idKey(playableItems);
    if (!trackId.isValid()) {
        return;
    }

    auto trackPlaybackDataIt = m_playbackDataMap.find(trackId);
    if (trackPlaybackDataIt == m_playbackDataMap.cend()) {
        return;
    }

    PlaybackData& trackPlaybackData = trackPlaybackDataIt->second;
    ArticulationsProfilePtr profile = profilesRepository()->defaultProfile(trackPlaybackData.setupData.category);
    if (!profile) {
        LOGE() << "unsupported instrument family: " << trackId.partId.toUint64();
        return;
    }

    PlaybackEventsMap result;

    const RepeatList& repeats = repeatList();

    constexpr timestamp_t actualTimestamp = 0;
    constexpr dynamic_level_t actualDynamicLevel = dynamicLevelFromType(muse::mpe::DynamicType::Natural);
    duration_t actualDuration = MScore::defaultPlayDuration * 1000;

    const PlaybackContextPtr ctx = playbackCtx(trackId);

    int minTick = std::numeric_limits<int>::max();

    for (const EngravingItem* item : playableItems) {
        if (item->isHarmony()) {
            m_renderer.renderChordSymbol(toHarmony(item), actualTimestamp, actualDuration, profile, result);
            continue;
        }

        int utick = repeats.tick2utick(item->tick().ticks());
        minTick = std::min(utick, minTick);

        m_renderer.render(item, actualTimestamp, actualDuration, actualDynamicLevel, ctx->persistentArticulationType(utick), profile,
                          result);
    }

    PlaybackParamList params = ctx->playbackParams(playableItems.front()->track(), minTick);
    trackPlaybackData.offStream.send(std::move(result), std::move(params));
}

void PlaybackModel::triggerMetronome(int tick)
{
    auto trackPlaybackData = m_playbackDataMap.find(METRONOME_TRACK_ID);
    if (trackPlaybackData == m_playbackDataMap.cend()) {
        return;
    }

    const ArticulationsProfilePtr profile = defaultActiculationProfile(METRONOME_TRACK_ID);

    PlaybackEventsMap result;
    m_renderer.renderMetronome(m_score, tick, 0, profile, result);
    trackPlaybackData->second.offStream.send(std::move(result), {});
}

void PlaybackModel::triggerCountIn(int tick, muse::mpe::duration_t& totalCountInDuration)
{
    auto trackPlaybackData = m_playbackDataMap.find(METRONOME_TRACK_ID);
    if (trackPlaybackData == m_playbackDataMap.cend()) {
        return;
    }

    const ArticulationsProfilePtr profile = defaultActiculationProfile(METRONOME_TRACK_ID);

    PlaybackEventsMap result;
    m_renderer.renderCountIn(m_score, tick, 0, profile, result, totalCountInDuration);
    trackPlaybackData->second.offStream.send(std::move(result), {});
}

InstrumentTrackIdSet PlaybackModel::existingTrackIdSet() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_playbackDataMap) {
        result.insert(pair.first);
    }

    return result;
}

muse::async::Channel<InstrumentTrackId> PlaybackModel::trackAdded() const
{
    return m_trackAdded;
}

muse::async::Channel<InstrumentTrackId> PlaybackModel::trackRemoved() const
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
            InstrumentTrackId trackId = idKey(part->id(), pair.second->id());

            if (!trackId.isValid() || containsTrack(trackId)) {
                continue;
            }

            m_setupResolver.resolveSetupData(pair.second, m_playbackDataMap[trackId].setupData);
        }

        if (part->hasChordSymbol()) {
            InstrumentTrackId trackId = chordSymbolsTrackId(part->id());
            m_setupResolver.resolveChordSymbolsSetupData(part->instrument(), m_playbackDataMap[trackId].setupData);
        }
    }

    m_setupResolver.resolveMetronomeSetupData(m_playbackDataMap[METRONOME_TRACK_ID].setupData);
}

void PlaybackModel::updateContext(const track_idx_t trackFrom, const track_idx_t trackTo)
{
    for (const Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            updateContext(trackId);
        }

        if (part->hasChordSymbol()) {
            updateContext(chordSymbolsTrackId(part->id()));
        }
    }
}

void PlaybackModel::updateContext(const InstrumentTrackId& trackId)
{
    PlaybackContextPtr ctx = playbackCtx(trackId);
    ctx->update(trackId.partId, m_score, m_expandRepeats);

    PlaybackData& trackData = m_playbackDataMap[trackId];
    trackData.dynamics = ctx->dynamicLevelLayers(m_score);
    trackData.params = ctx->playbackParamLayers(m_score);
}

void PlaybackModel::processSegment(const int tickPositionOffset, const Segment* segment, const std::set<staff_idx_t>& staffIdxSet,
                                   bool isFirstChordRestSegmentOfMeasure, ChangedTrackIdSet* trackChanges)
{
    for (const EngravingItem* item : segment->annotations()) {
        if (!item || !item->part()) {
            continue;
        }

        const Harmony* chordSymbol = findChordSymbol(item);
        if (!chordSymbol) {
            continue;
        }

        staff_idx_t staffIdx = item->staffIdx();
        if (staffIdxSet.find(staffIdx) == staffIdxSet.cend()) {
            continue;
        }

        InstrumentTrackId trackId = chordSymbolsTrackId(item->part()->id());

        ArticulationsProfilePtr profile = defaultActiculationProfile(trackId);
        if (!profile) {
            LOGE() << "unsupported instrument family: " << item->part()->id();
            continue;
        }

        if (chordSymbol->play()) {
            m_renderer.renderChordSymbol(chordSymbol, tickPositionOffset, profile,
                                         m_playbackDataMap[trackId].originEvents);
        }

        collectChangesTracks(trackId, trackChanges);
    }

    if (segment->isTimeTickType()) {
        return; // optimization: search only for annotations
    }

    for (const EngravingItem* item : segment->elist()) {
        if (!item || !item->isChordRest() || !item->part()) {
            continue;
        }

        staff_idx_t staffIdx = item->staffIdx();
        if (staffIdxSet.find(staffIdx) == staffIdxSet.cend()) {
            continue;
        }

        InstrumentTrackId trackId = idKey(item);

        if (!trackId.isValid()) {
            continue;
        }

        if (isFirstChordRestSegmentOfMeasure) {
            if (item->isMeasureRepeat()) {
                const MeasureRepeat* measureRepeat = toMeasureRepeat(item);
                const Measure* currentMeasure = measureRepeat->measure();

                processMeasureRepeat(tickPositionOffset, measureRepeat, currentMeasure, staffIdx, trackChanges);

                continue;
            } else if (item->voice() == 0) {
                const Measure* currentMeasure = segment->measure();

                if (currentMeasure->measureRepeatCount(staffIdx) > 0) {
                    const MeasureRepeat* measureRepeat = currentMeasure->measureRepeatElement(staffIdx);

                    processMeasureRepeat(tickPositionOffset, measureRepeat, currentMeasure, staffIdx, trackChanges);
                    continue;
                }
            }
        }

        ArticulationsProfilePtr profile = defaultActiculationProfile(trackId);
        if (!profile) {
            LOGE() << "unsupported instrument family: " << item->part()->id();
            continue;
        }

        const PlaybackContextPtr ctx = playbackCtx(trackId);
        m_renderer.render(item, tickPositionOffset, std::move(profile), ctx, m_playbackDataMap[trackId].originEvents);

        collectChangesTracks(trackId, trackChanges);
    }
}

void PlaybackModel::processMeasureRepeat(const int tickPositionOffset, const MeasureRepeat* measureRepeat, const Measure* currentMeasure,
                                         const staff_idx_t staffIdx, ChangedTrackIdSet* trackChanges)
{
    if (!measureRepeat || !currentMeasure) {
        return;
    }

    const Measure* referringMeasure = measureRepeat->referringMeasure(currentMeasure);
    if (!referringMeasure) {
        return;
    }

    IF_ASSERT_FAILED(referringMeasure != currentMeasure) {
        return;
    }

    int currentMeasureTick = currentMeasure->tick().ticks();
    int referringMeasureTick = referringMeasure->tick().ticks();
    int repeatPositionTickOffset = currentMeasureTick - referringMeasureTick;
    int tickFrom = tickPositionOffset + repeatPositionTickOffset;

    std::set<staff_idx_t> staffToProcessIdxSet { staffIdx };
    int chordRestSegmentNum = -1;

    for (const Segment* seg = referringMeasure->first(); seg; seg = seg->next()) {
        if (!seg->isChordRestType() && !seg->isTimeTickType()) {
            continue;
        }

        if (seg->isChordRestType()) {
            chordRestSegmentNum++;
        }

        processSegment(tickFrom, seg, staffToProcessIdxSet, chordRestSegmentNum == 0, trackChanges);
    }
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                                 ChangedTrackIdSet* trackChanges)
{
    TRACEFUNC;

    std::set<staff_idx_t> staffToProcessIdxSet = m_score->staffIdxSetFromRange(trackFrom, trackTo, [](const Staff& staff) {
        return staff.isPrimaryStaff(); // skip linked staves
    });

    const ArticulationsProfilePtr metronomeProfile = defaultActiculationProfile(METRONOME_TRACK_ID);
    PlaybackEventsMap& metronomeEvents = m_playbackDataMap[METRONOME_TRACK_ID].originEvents;

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

            int chordRestSegmentNum = -1;

            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType() && !segment->isTimeTickType()) {
                    continue;
                }

                int segmentStartTick = segment->tick().ticks();
                int segmentEndTick = segmentStartTick + segment->ticks().ticks();

                if (segmentStartTick > tickTo || segmentEndTick <= tickFrom) {
                    continue;
                }

                if (segment->isChordRestType()) {
                    chordRestSegmentNum++;
                }

                processSegment(tickPositionOffset, segment, staffToProcessIdxSet, chordRestSegmentNum == 0, trackChanges);
            }

            if (m_metronomeEnabled) {
                m_renderer.renderMetronome(m_score, measureStartTick, measureEndTick, tickPositionOffset,
                                           metronomeProfile, metronomeEvents);
                collectChangesTracks(METRONOME_TRACK_ID, trackChanges);
            }
        }
    }
}

void PlaybackModel::reloadMetronomeEvents()
{
    TRACEFUNC;

    PlaybackData& metronomeData = m_playbackDataMap[METRONOME_TRACK_ID];
    metronomeData.originEvents.clear();

    if (!m_metronomeEnabled) {
        metronomeData.mainStream.send(metronomeData.originEvents, metronomeData.dynamics, metronomeData.params);
        return;
    }

    const ArticulationsProfilePtr metronomeProfile = defaultActiculationProfile(METRONOME_TRACK_ID);

    for (const RepeatSegment* repeatSegment : repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            int measureStartTick = measure->tick().ticks();
            int measureEndTick = measure->endTick().ticks();

            m_renderer.renderMetronome(m_score, measureStartTick, measureEndTick, tickPositionOffset,
                                       metronomeProfile, metronomeData.originEvents);
        }
    }

    metronomeData.mainStream.send(metronomeData.originEvents, metronomeData.dynamics, metronomeData.params);
}

bool PlaybackModel::hasToReloadTracks(const ScoreChangesRange& changesRange) const
{
    static const std::unordered_set<ElementType> REQUIRED_TYPES = {
        ElementType::PLAYTECH_ANNOTATION,
        ElementType::CAPO,
        ElementType::DYNAMIC,
        ElementType::HAIRPIN,
        ElementType::HAIRPIN_SEGMENT,
        ElementType::HARMONY,
        ElementType::STAFF_TEXT,
        ElementType::SOUND_FLAG,
        ElementType::MEASURE_REPEAT,
        ElementType::GUITAR_BEND,
        ElementType::GUITAR_BEND_SEGMENT,
        ElementType::BREATH,
    };

    for (const ElementType type : REQUIRED_TYPES) {
        if (changesRange.changedTypes.find(type) == changesRange.changedTypes.cend()) {
            continue;
        }

        return true;
    }

    if (changesRange.isValidBoundary()) {
        const Measure* measureTo = m_score->tick2measure(Fraction::fromTicks(changesRange.tickTo));
        if (!measureTo) {
            return false;
        }

        if (measureTo->containsMeasureRepeat(changesRange.staffIdxFrom, changesRange.staffIdxTo)) {
            return true;
        }

        const Measure* nextMeasure = measureTo->nextMeasure();

        for (int i = 0; i < MeasureRepeat::MAX_NUM_MEASURES && nextMeasure; ++i) {
            if (nextMeasure->containsMeasureRepeat(changesRange.staffIdxFrom, changesRange.staffIdxTo)) {
                return true;
            }

            nextMeasure = nextMeasure->nextMeasure();
        }
    }

    return false;
}

bool PlaybackModel::hasToReloadScore(const ScoreChangesRange& changesRange) const
{
    static const std::unordered_set<ElementType> REQUIRED_TYPES = {
        ElementType::SCORE,
        ElementType::GRADUAL_TEMPO_CHANGE,
        ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT,
        ElementType::TEMPO_TEXT,
        ElementType::LAYOUT_BREAK,
        ElementType::FERMATA,
        ElementType::VOLTA,
        ElementType::VOLTA_SEGMENT,
        ElementType::SYSTEM_TEXT,
        ElementType::JUMP,
        ElementType::MARKER,
        ElementType::BREATH,
    };

    for (const ElementType type : REQUIRED_TYPES) {
        if (changesRange.changedTypes.find(type) == changesRange.changedTypes.cend()) {
            continue;
        }

        return true;
    }

    static const std::unordered_set<mu::engraving::Pid> REQUIRED_PROPERTIES {
        mu::engraving::Pid::REPEAT_START,
        mu::engraving::Pid::REPEAT_END,
        mu::engraving::Pid::REPEAT_JUMP,
        mu::engraving::Pid::REPEAT_COUNT,
    };

    for (const Pid pid: changesRange.changedPropertyIdSet) {
        if (muse::contains(REQUIRED_PROPERTIES, pid)) {
            return true;
        }
    }

    return false;
}

bool PlaybackModel::containsTrack(const InstrumentTrackId& trackId) const
{
    return m_playbackDataMap.find(trackId) != m_playbackDataMap.cend();
}

void PlaybackModel::clearExpiredTracks()
{
    auto needRemoveTrack = [this](const InstrumentTrackId& trackId) {
        const Part* part = m_score->partById(trackId.partId.toUint64());

        if (!part) {
            return true;
        }

        if (trackId.instrumentId == CHORD_SYMBOLS_INSTRUMENT_ID) {
            return !part->hasChordSymbol();
        }

        return !part->instruments().contains(trackId.instrumentId);
    };

    auto it = m_playbackDataMap.cbegin();

    while (it != m_playbackDataMap.cend()) {
        if (it->first == METRONOME_TRACK_ID) {
            ++it;
            continue;
        }

        if (needRemoveTrack(it->first)) {
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
            PlaybackContextPtr ctx = playbackCtx(trackId);
            ctx->clear();
        }

        if (part->hasChordSymbol()) {
            InstrumentTrackId trackId = chordSymbolsTrackId(part->id());
            PlaybackContextPtr ctx = playbackCtx(trackId);
            ctx->clear();
        }
    }
}

void mu::engraving::PlaybackModel::removeEventsFromRange(const track_idx_t trackFrom, const track_idx_t trackTo,
                                                         const timestamp_t timestampFrom, const timestamp_t timestampTo)
{
    for (const Part* part : m_score->parts()) {
        if (part->startTrack() > trackTo || part->endTrack() <= trackFrom) {
            continue;
        }

        for (const InstrumentTrackId& trackId : part->instrumentTrackIdSet()) {
            removeTrackEvents(trackId, timestampFrom, timestampTo);
        }

        removeTrackEvents(chordSymbolsTrackId(part->id()), timestampFrom, timestampTo);
    }

    if (m_metronomeEnabled) {
        removeTrackEvents(METRONOME_TRACK_ID, timestampFrom, timestampTo);
    }
}

void PlaybackModel::clearExpiredEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo)
{
    TRACEFUNC;

    if (!m_score) {
        return;
    }

    const Measure* lastMeasure = m_score->lastMeasure();
    if (!lastMeasure) {
        return;
    }

    if (tickFrom == 0 && lastMeasure->endTick().ticks() == tickTo) {
        removeEventsFromRange(trackFrom, trackTo);
        return;
    }

    for (const RepeatSegment* repeatSegment : repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        int repeatStartTick = repeatSegment->tick;
        int repeatEndTick = repeatStartTick + repeatSegment->len();

        if (repeatStartTick > tickTo || repeatEndTick <= tickFrom) {
            continue;
        }

        int removeEventsFromTick = std::max(tickFrom, repeatStartTick);
        timestamp_t removeEventsFrom = timestampFromTicks(m_score, removeEventsFromTick + tickPositionOffset);

        //! NOTE: the end tick of the current repeat segment == the start tick of the next repeat segment
        //! so subtract 1 to avoid removing events belonging to the next segment
        int removeEventsToTick = std::min(tickTo, repeatEndTick - 1);
        timestamp_t removeEventsTo = timestampFromTicks(m_score, removeEventsToTick + tickPositionOffset);

        removeEventsFromRange(trackFrom, trackTo, removeEventsFrom, removeEventsTo);
    }
}

void PlaybackModel::collectChangesTracks(const InstrumentTrackId& trackId, ChangedTrackIdSet* result)
{
    if (!result) {
        return;
    }

    result->insert(trackId);
}

void PlaybackModel::notifyAboutChanges(const InstrumentTrackIdSet& oldTracks, const InstrumentTrackIdSet& changedTracks)
{
    for (const InstrumentTrackId& trackId : changedTracks) {
        auto search = m_playbackDataMap.find(trackId);

        if (search == m_playbackDataMap.cend()) {
            continue;
        }

        search->second.mainStream.send(search->second.originEvents, search->second.dynamics, search->second.params);
    }

    for (auto it = m_playbackDataMap.cbegin(); it != m_playbackDataMap.cend(); ++it) {
        if (!muse::contains(oldTracks, it->first)) {
            m_trackAdded.send(it->first);
        }
    }

    if (!changedTracks.empty()) {
        m_dataChanged.notify();
    }
}

void PlaybackModel::removeTrackEvents(const InstrumentTrackId& trackId, const muse::mpe::timestamp_t timestampFrom,
                                      const muse::mpe::timestamp_t timestampTo)
{
    IF_ASSERT_FAILED(timestampFrom <= timestampTo) {
        return;
    }

    auto search = m_playbackDataMap.find(trackId);

    if (search == m_playbackDataMap.cend()) {
        return;
    }

    PlaybackData& trackPlaybackData = search->second;

    if (timestampFrom == -1 && timestampTo == -1) {
        search->second.originEvents.clear();
        return;
    }

    PlaybackEventsMap::const_iterator lowerBound;

    if (timestampFrom == 0) {
        //!Note Some events might be started RIGHT before the "official" start of the track
        //!     Need to make sure that we don't miss those events
        lowerBound = trackPlaybackData.originEvents.begin();
    } else {
        lowerBound = trackPlaybackData.originEvents.lower_bound(timestampFrom);
    }

    auto upperBound = trackPlaybackData.originEvents.upper_bound(timestampTo);

    for (auto it = lowerBound; it != upperBound && it != trackPlaybackData.originEvents.end();) {
        it = trackPlaybackData.originEvents.erase(it);
    }
}

PlaybackModel::TrackBoundaries PlaybackModel::trackBoundaries(const ScoreChangesRange& changesRange) const
{
    TrackBoundaries result;

    result.trackFrom = staff2track(changesRange.staffIdxFrom, 0);
    result.trackTo = staff2track(changesRange.staffIdxTo, VOICES);

    if (hasToReloadScore(changesRange) || !changesRange.isValidBoundary()) {
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

    if (hasToReloadTracks(changesRange)
        || hasToReloadScore(changesRange)
        || !changesRange.isValidBoundary()) {
        const Measure* lastMeasure = m_score->lastMeasure();
        result.tickFrom = 0;
        result.tickTo = lastMeasure ? lastMeasure->endTick().ticks() : 0;

        return result;
    }

    for (const auto& pair : changesRange.changedItems) {
        const EngravingItem* item = pair.first;

        if (item->isNote()) {
            const Note* note = toNote(item);
            const Chord* chord = note->chord();
            const TremoloTwoChord* tremoloTwo = chord->tremoloTwoChord();

            if (tremoloTwo) {
                const Chord* startChord = tremoloTwo->chord1();
                const Chord* endChord = tremoloTwo->chord2();

                IF_ASSERT_FAILED(startChord && endChord) {
                    continue;
                }

                result.tickFrom = std::min(result.tickFrom, startChord->tick().ticks());
                result.tickTo = std::max(result.tickTo, endChord->tick().ticks());
            }

            applyTiedNotesTickBoundaries(note, result);
        } else if (item->isTie()) {
            applyTieTickBoundaries(toTie(item), result);
        }

        const EngravingItem* parent = item->parentItem();
        if (!parent) {
            continue;
        }

        if (parent->isChord()) {
            const Chord* chord = toChord(parent);

            for (const Note* note : chord->notes()) {
                applyTiedNotesTickBoundaries(note, result);
            }

            for (const Spanner* spanner : chord->startingSpanners()) {
                if (spanner->isTrill() && result.tickTo < spanner->tick2().ticks()) {
                    result.tickTo = spanner->tick2().ticks();
                }
            }
        } else if (parent->isNote()) {
            applyTiedNotesTickBoundaries(toNote(parent), result);
        }
    }

    return result;
}

const RepeatList& PlaybackModel::repeatList() const
{
    m_score->masterScore()->setExpandRepeats(m_expandRepeats);

    return m_score->repeatList();
}

std::vector<const EngravingItem*> PlaybackModel::filterPlayableItems(const std::vector<const EngravingItem*>& items) const
{
    std::vector<const EngravingItem*> result;

    for (const EngravingItem* item : items) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        if (!item->isPlayable()) {
            continue;
        }

        result.push_back(item);
    }

    return result;
}

InstrumentTrackId PlaybackModel::idKey(const EngravingItem* item) const
{
    if (item->isHarmony()) {
        return chordSymbolsTrackId(item->part()->id());
    }

    return makeInstrumentTrackId(item);
}

InstrumentTrackId PlaybackModel::idKey(const std::vector<const EngravingItem*>& items) const
{
    InstrumentTrackId result;

    for (const EngravingItem* item : items) {
        InstrumentTrackId itemTrackId = idKey(item);
        if (result.isValid() && result != itemTrackId) {
            LOGE() << "Triggering events for elements with different tracks";
            return InstrumentTrackId();
        }

        result = itemTrackId;
    }

    return result;
}

InstrumentTrackId PlaybackModel::idKey(const ID& partId, const String& instrumentId) const
{
    return { partId, instrumentId };
}

muse::mpe::ArticulationsProfilePtr PlaybackModel::defaultActiculationProfile(const InstrumentTrackId& trackId) const
{
    auto it = m_playbackDataMap.find(trackId);
    if (it == m_playbackDataMap.cend()) {
        return nullptr;
    }

    return profilesRepository()->defaultProfile(it->second.setupData.category);
}

PlaybackContextPtr PlaybackModel::playbackCtx(const InstrumentTrackId& trackId)
{
    auto it = m_playbackCtxMap.find(trackId);
    if (it == m_playbackCtxMap.end()) {
        PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();
        m_playbackCtxMap.emplace(trackId, ctx);
        return ctx;
    }

    return it->second;
}

void PlaybackModel::applyTiedNotesTickBoundaries(const Note* note, TickBoundaries& tickBoundaries)
{
    const Tie* tie;
    if ((tie = note->tieFor())) {
        applyTieTickBoundaries(tie, tickBoundaries);
    } else if ((tie = note->tieBack())) {
        applyTieTickBoundaries(tie, tickBoundaries);
    }
}

void PlaybackModel::applyTieTickBoundaries(const Tie* tie, TickBoundaries& tickBoundaries)
{
    const Note* startNote = tie->startNote();
    const Note* endNote = tie->endNote();
    if (!startNote || !endNote) {
        return;
    }

    const Note* firstTiedNote = startNote->firstTiedNote();
    const Note* lastTiedNote = endNote->lastTiedNote();
    IF_ASSERT_FAILED(firstTiedNote && lastTiedNote) {
        return;
    }

    tickBoundaries.tickFrom = std::min(tickBoundaries.tickFrom, firstTiedNote->tick().ticks());
    tickBoundaries.tickTo = std::max(tickBoundaries.tickTo, lastTiedNote->tick().ticks());
}
