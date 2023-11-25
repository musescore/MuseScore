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
#include "dom/tremolo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu::async;

static const std::string METRONOME_INSTRUMENT_ID("metronome");
static const std::string CHORD_SYMBOLS_INSTRUMENT_ID("chord_symbols");

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

bool PlaybackModel::isPlayChordSymbolsEnabled() const
{
    return m_playChordSymbols;
}

void PlaybackModel::setPlayChordSymbols(const bool isEnabled)
{
    m_playChordSymbols = isEnabled;
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
    constexpr dynamic_level_t actualDynamicLevel = dynamicLevelFromType(mpe::DynamicType::Natural);
    duration_t actualDuration = MScore::defaultPlayDuration * 1000;

    for (const EngravingItem* item : playableItems) {
        if (item->isHarmony()) {
            m_renderer.renderChordSymbol(toHarmony(item), actualTimestamp, actualDuration, profile, result);
            continue;
        }

        int utick = repeats.tick2utick(item->tick().ticks());
        const PlaybackContext& ctx = m_playbackCtxMap[trackId];

        m_renderer.render(item, actualTimestamp, actualDuration, actualDynamicLevel, ctx.persistentArticulationType(utick), profile,
                          result);
    }

    trackPlaybackData.offStream.send(std::move(result));
}

void PlaybackModel::triggerMetronome(int tick)
{
    auto trackPlaybackData = m_playbackDataMap.find(metronomeTrackId());
    if (trackPlaybackData == m_playbackDataMap.cend()) {
        return;
    }

    PlaybackEventsMap result;
    m_renderer.renderMetronome(m_score, tick, 0, result);
    trackPlaybackData->second.offStream.send(std::move(result));
}

InstrumentTrackIdSet PlaybackModel::existingTrackIdSet() const
{
    InstrumentTrackIdSet result;

    for (const auto& pair : m_playbackDataMap) {
        result.insert(pair.first);
    }

    return result;
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
    PlaybackContext& ctx = m_playbackCtxMap[trackId];
    ctx.update(trackId.partId, m_score);

    PlaybackData& trackData = m_playbackDataMap[trackId];
    trackData.dynamicLevelMap = ctx.dynamicLevelMap(m_score);
}

void PlaybackModel::processSegment(const int tickPositionOffset, const Segment* segment, const std::set<staff_idx_t>& staffIdxSet,
                                   bool isFirstSegmentOfMeasure, ChangedTrackIdSet* trackChanges)
{
    int segmentStartTick = segment->tick().ticks();

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

        if (isFirstSegmentOfMeasure) {
            if (item->isMeasureRepeat()) {
                const MeasureRepeat* measureRepeat = toMeasureRepeat(item);
                const Measure* currentMeasure = measureRepeat->measure();

                processMeasureRepeat(tickPositionOffset, measureRepeat, currentMeasure, staffIdx, trackChanges);

                continue;
            } else {
                const Measure* currentMeasure = segment->measure();

                if (currentMeasure->measureRepeatCount(staffIdx) > 0) {
                    const MeasureRepeat* measureRepeat = currentMeasure->measureRepeatElement(staffIdx);

                    processMeasureRepeat(tickPositionOffset, measureRepeat, currentMeasure, staffIdx, trackChanges);
                    continue;
                }
            }
        }

        const PlaybackContext& ctx = m_playbackCtxMap[trackId];

        ArticulationsProfilePtr profile = defaultActiculationProfile(trackId);
        if (!profile) {
            LOGE() << "unsupported instrument family: " << item->part()->id();
            continue;
        }

        m_renderer.render(item, tickPositionOffset, ctx.appliableDynamicLevel(segmentStartTick + tickPositionOffset),
                          ctx.persistentArticulationType(segmentStartTick + tickPositionOffset), std::move(profile),
                          m_playbackDataMap[trackId].originEvents);

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

    bool isFirstSegmentOfRepeatedMeasure = true;

    for (const Segment* seg = referringMeasure->first(); seg; seg = seg->next()) {
        if (!seg->isChordRestType()) {
            continue;
        }

        processSegment(tickPositionOffset + repeatPositionTickOffset, seg, { staffIdx }, isFirstSegmentOfRepeatedMeasure, trackChanges);
        isFirstSegmentOfRepeatedMeasure = false;
    }
}

void PlaybackModel::updateEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo,
                                 ChangedTrackIdSet* trackChanges)
{
    TRACEFUNC;

    std::set<staff_idx_t> staffToProcessIdxSet = m_score->staffIdxSetFromRange(trackFrom, trackTo, [](const Staff& staff) {
        return staff.isPrimaryStaff(); // skip linked staves
    });

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

            bool isFirstSegmentOfMeasure = true;

            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                if (!segment->isChordRestType()) {
                    continue;
                }

                int segmentStartTick = segment->tick().ticks();
                int segmentEndTick = segmentStartTick + segment->ticks().ticks();

                if (segmentStartTick > tickTo || segmentEndTick <= tickFrom) {
                    continue;
                }

                processSegment(tickPositionOffset, segment, staffToProcessIdxSet, isFirstSegmentOfMeasure, trackChanges);
                isFirstSegmentOfMeasure = false;
            }

            m_renderer.renderMetronome(m_score, measureStartTick, measureEndTick, tickPositionOffset,
                                       m_playbackDataMap[METRONOME_TRACK_ID].originEvents);
            collectChangesTracks(METRONOME_TRACK_ID, trackChanges);
        }
    }
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
        ElementType::MEASURE_REPEAT,
        ElementType::GUITAR_BEND,
        ElementType::GUITAR_BEND_SEGMENT,
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

bool PlaybackModel::hasToReloadScore(const std::unordered_set<ElementType>& changedTypes) const
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
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.clear();
        }

        if (part->hasChordSymbol()) {
            InstrumentTrackId trackId = chordSymbolsTrackId(part->id());
            PlaybackContext& ctx = m_playbackCtxMap[trackId];
            ctx.clear();
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

    removeTrackEvents(METRONOME_TRACK_ID, timestampFrom, timestampTo);
}

void PlaybackModel::clearExpiredEvents(const int tickFrom, const int tickTo, const track_idx_t trackFrom, const track_idx_t trackTo)
{
    TRACEFUNC;

    if (!m_score || !m_score->lastMeasure()) {
        return;
    }

    if (tickFrom == 0 && m_score->lastMeasure()->endTick().ticks() == tickTo) {
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

        timestamp_t timestampFrom = timestampFromTicks(m_score, tickFrom + tickPositionOffset);
        timestamp_t timestampTo = timestampFromTicks(m_score, tickTo + tickPositionOffset);

        removeEventsFromRange(trackFrom, trackTo, timestampFrom, timestampTo);
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

        search->second.mainStream.send(search->second.originEvents);
        search->second.dynamicLevelChanges.send(search->second.dynamicLevelMap);
    }

    for (auto it = m_playbackDataMap.cbegin(); it != m_playbackDataMap.cend(); ++it) {
        if (!mu::contains(oldTracks, it->first)) {
            m_trackAdded.send(it->first);
        }
    }

    if (!changedTracks.empty()) {
        m_dataChanged.notify();
    }
}

void PlaybackModel::removeTrackEvents(const InstrumentTrackId& trackId, const mpe::timestamp_t timestampFrom,
                                      const mpe::timestamp_t timestampTo)
{
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

    if (hasToReloadTracks(changesRange)
        || hasToReloadScore(changesRange.changedTypes)
        || !changesRange.isValidBoundary()) {
        const Measure* lastMeasure = m_score->lastMeasure();
        result.tickFrom = 0;
        result.tickTo = lastMeasure ? lastMeasure->endTick().ticks() : 0;

        return result;
    }

    for (const EngravingItem* item : changesRange.changedItems) {
        if (item->isNote()) {
            const Note* note = toNote(item);
            const Chord* chord = note->chord();
            const Tremolo* tremolo = chord->tremolo();

            if (tremolo && tremolo->twoNotes()) {
                const Chord* startChord = tremolo->chord1();
                const Chord* endChord = tremolo->chord2();

                IF_ASSERT_FAILED(startChord && endChord) {
                    continue;
                }

                result.tickFrom = std::min(result.tickFrom, startChord->tick().ticks());
                result.tickTo = std::max(result.tickTo, endChord->tick().ticks());
            }
        } else if (item->isTie()) {
            const Tie* tie = toTie(item);
            const Note* startNote = tie->startNote();
            const Note* endNote = tie->endNote();

            IF_ASSERT_FAILED(startNote && endNote) {
                continue;
            }

            const Note* firstTiedNote = startNote->firstTiedNote();
            const Note* lastTiedNote = endNote->lastTiedNote();

            IF_ASSERT_FAILED(firstTiedNote && lastTiedNote) {
                continue;
            }

            result.tickFrom = std::min(result.tickFrom, firstTiedNote->tick().ticks());
            result.tickTo = std::max(result.tickTo, lastTiedNote->tick().ticks());
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

    return { item->part()->id(),
             item->part()->instrumentId(item->tick()).toStdString() };
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

InstrumentTrackId PlaybackModel::idKey(const ID& partId, const std::string& instrumentId) const
{
    return { partId, instrumentId };
}

mpe::ArticulationsProfilePtr PlaybackModel::defaultActiculationProfile(const InstrumentTrackId& trackId) const
{
    auto it = m_playbackDataMap.find(trackId);
    if (it == m_playbackDataMap.cend()) {
        return nullptr;
    }

    return profilesRepository()->defaultProfile(it->second.setupData.category);
}
