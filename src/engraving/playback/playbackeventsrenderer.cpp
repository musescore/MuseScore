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

#include "playbackeventsrenderer.h"

#include "log.h"

#include "dom/chord.h"
#include "dom/harmony.h"
#include "dom/note.h"
#include "dom/rest.h"
#include "dom/sig.h"
#include "dom/tempo.h"
#include "dom/staff.h"

#include "utils/arrangementutils.h"

#include "metaparsers/chordarticulationsparser.h"
#include "metaparsers/notearticulationsparser.h"

#include "renderers/chordarticulationsrenderer.h"

#include "filters/chordfilter.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static ArticulationMap makeStandardArticulationMap(const ArticulationsProfilePtr profile, timestamp_t timestamp, duration_t duration)
{
    IF_ASSERT_FAILED(profile) {
        return {};
    }

    ArticulationMeta meta(ArticulationType::Standard,
                          profile->pattern(ArticulationType::Standard),
                          timestamp,
                          duration,
                          0,
                          0);

    ArticulationMap articulations;
    articulations.emplace(ArticulationType::Standard, mpe::ArticulationAppliedData(std::move(meta), 0, mpe::HUNDRED_PERCENT));
    articulations.preCalculateAverageData();

    return articulations;
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const int tickPositionOffset,
                                    const ArticulationsProfilePtr profile, const PlaybackContextPtr playbackCtx,
                                    PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(item->isChordRest()) {
        return;
    }

    if (item->type() == ElementType::CHORD) {
        renderNoteEvents(toChord(item), tickPositionOffset, profile, playbackCtx, result);
    } else if (item->type() == ElementType::REST) {
        renderRestEvents(toRest(item), tickPositionOffset, result);
    }
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const mpe::timestamp_t actualTimestamp,
                                    const mpe::duration_t actualDuration, const mpe::dynamic_level_t actualDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(item->isChordRest() || item->isNote()) {
        return;
    }

    ElementType type = item->type();

    if (type == ElementType::CHORD) {
        const Chord* chord = toChord(item);
        mpe::PlaybackEventList& events = result[actualTimestamp];

        for (const Note* note : chord->notes()) {
            renderFixedNoteEvent(note, actualTimestamp, actualDuration,
                                 actualDynamicLevel, persistentArticulationApplied, profile, events);
        }
    } else if (type == ElementType::NOTE) {
        renderFixedNoteEvent(toNote(item), actualTimestamp, actualDuration,
                             actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
    } else if (type == ElementType::REST) {
        renderRestEvents(toRest(item), 0, result);
    }
}

void PlaybackEventsRenderer::renderChordSymbol(const Harmony* chordSymbol,
                                               const int ticksPositionOffset,
                                               const mpe::ArticulationsProfilePtr profile,
                                               mpe::PlaybackEventsMap& result) const
{
    if (!chordSymbol->isRealizable()) {
        return;
    }

    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    const Score* score = chordSymbol->score();
    int positionTick = chordSymbol->tick().ticks();

    timestamp_t eventTimestamp = timestampFromTicks(score, positionTick + ticksPositionOffset);
    PlaybackEventList& events = result[eventTimestamp];

    int durationTicks = realized.getActualDuration(positionTick + ticksPositionOffset).ticks();
    duration_t duration = timestampFromTicks(score, positionTick + ticksPositionOffset + durationTicks) - eventTimestamp;

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    staff_layer_idx_t staffIdx = static_cast<staff_layer_idx_t>(chordSymbol->staffIdx());
    Staff* staff = chordSymbol->staff();
    IF_ASSERT_FAILED(staff) {
        return;
    }
    Key key = staff->key(chordSymbol->tick());

    ArticulationMap articulations = makeStandardArticulationMap(profile, eventTimestamp, duration);

    double bps = score->tempomap()->tempo(positionTick).val;

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int pitch = it->first;
        int tpc = pitch2tpc(pitch, key, Prefer::NEAREST);
        int octave = playingOctave(pitch, tpc);
        pitch_level_t pitchLevel = notePitchLevel(tpc, octave);

        events.emplace_back(mpe::NoteEvent(eventTimestamp,
                                           duration,
                                           voiceIdx,
                                           staffIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           bps));
    }
}

void PlaybackEventsRenderer::renderChordSymbol(const Harmony* chordSymbol, const mpe::timestamp_t actualTimestamp,
                                               const mpe::duration_t actualDuration, const ArticulationsProfilePtr profile,
                                               mpe::PlaybackEventsMap& result) const
{
    if (!chordSymbol->isRealizable()) {
        return;
    }

    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    PlaybackEventList& events = result[actualTimestamp];

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    staff_layer_idx_t staffIdx = static_cast<staff_layer_idx_t>(chordSymbol->staffIdx());

    Key key = chordSymbol->staff()->key(chordSymbol->tick());

    ArticulationMap articulations = makeStandardArticulationMap(profile, actualTimestamp, actualDuration);

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int pitch = it->first;
        int tpc = pitch2tpc(pitch, key, Prefer::NEAREST);
        int octave = playingOctave(pitch, tpc);
        pitch_level_t pitchLevel = notePitchLevel(tpc, octave);

        events.emplace_back(mpe::NoteEvent(actualTimestamp,
                                           actualDuration,
                                           voiceIdx,
                                           staffIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           Constants::DEFAULT_TEMPO.val));
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick,
                                             const int ticksPositionOffset, const muse::mpe::ArticulationsProfilePtr profile,
                                             mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(measureStartTick).timesig();
    BeatsPerSecond bps = score->tempomap()->tempo(measureStartTick);

    int step = timeSignatureFraction.isBeatedCompound(bps.val)
               ? timeSignatureFraction.beatTicks() : timeSignatureFraction.dUnitTicks();

    for (int tick = measureStartTick; tick < measureEndTick; tick += step) {
        timestamp_t eventTimestamp = timestampFromTicks(score, tick + ticksPositionOffset);

        renderMetronome(score, tick, eventTimestamp, profile, result);
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int tick, const mpe::timestamp_t actualTimestamp,
                                             const muse::mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(tick).timesig();
    int ticksPerBeat = timeSignatureFraction.ticks() / timeSignatureFraction.numerator();

    duration_t duration = timestampFromTicks(score, tick + ticksPerBeat) - actualTimestamp;

    BeatType beatType = score->tick2beatType(Fraction::fromTicks(tick));
    pitch_level_t eventPitchLevel = beatType == BeatType::DOWNBEAT
                                    ? pitchLevel(PitchClass::E, 5) // high wood block
                                    : pitchLevel(PitchClass::F, 5); // low wood block

    const ArticulationMap articulations = makeStandardArticulationMap(profile, actualTimestamp, duration);

    BeatsPerSecond bps = score->tempomap()->tempo(tick);

    result[actualTimestamp].emplace_back(mpe::NoteEvent(actualTimestamp,
                                                        duration,
                                                        0,
                                                        0,
                                                        eventPitchLevel,
                                                        dynamicLevelFromType(mpe::DynamicType::mf),
                                                        articulations,
                                                        bps.val));
}

void PlaybackEventsRenderer::renderNoteEvents(const Chord* chord, const int tickPositionOffset,
                                              const mpe::ArticulationsProfilePtr profile, const PlaybackContextPtr playbackCtx,
                                              PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(chord) {
        return;
    }

    RenderingContext ctx = engraving::buildRenderingCtx(chord, tickPositionOffset, profile, playbackCtx);

    if (!ChordFilter::isItemPlayable(chord, ctx)) {
        return;
    }

    ChordArticulationsParser::buildChordArticulationMap(chord, ctx, ctx.commonArticulations);

    ChordArticulationsRenderer::render(chord, ArticulationType::Last, ctx, result[ctx.nominalTimestamp]);
}

void PlaybackEventsRenderer::renderFixedNoteEvent(const Note* note, const mpe::timestamp_t actualTimestamp,
                                                  const mpe::duration_t actualDuration,
                                                  const mpe::dynamic_level_t actualDynamicLevel,
                                                  const mpe::ArticulationType persistentArticulationApplied,
                                                  const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventList& result) const
{
    static const ArticulationMap articulations;
    static const PlaybackContextPtr dummyCtx = std::make_shared<PlaybackContext>();

    RenderingContext ctx(actualTimestamp,
                         actualDuration,
                         actualDynamicLevel,
                         0,
                         0,
                         ticksFromTempoAndDuration(Constants::DEFAULT_TEMPO.val, actualDuration),
                         Constants::DEFAULT_TEMPO,
                         TimeSigMap::DEFAULT_TIME_SIGNATURE,
                         persistentArticulationApplied,
                         articulations,
                         note->score(),
                         profile,
                         dummyCtx);

    NoteArticulationsParser::parsePersistentMeta(ctx, ctx.commonArticulations);
    NoteArticulationsParser::parseGhostNote(note, ctx, ctx.commonArticulations);
    NoteArticulationsParser::parseNoteHead(note, ctx, ctx.commonArticulations);
    NoteArticulationsParser::parseSymbols(note, ctx, ctx.commonArticulations);

    if (ctx.commonArticulations.empty()) {
        ctx.commonArticulations = makeStandardArticulationMap(profile, actualTimestamp, actualDuration);
    } else {
        ctx.commonArticulations.preCalculateAverageData();
    }

    NominalNoteCtx noteCtx(note, ctx);
    result.emplace_back(buildNoteEvent(std::move(noteCtx)));
}

void PlaybackEventsRenderer::renderRestEvents(const Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(rest) {
        return;
    }

    int positionTick = rest->tick().ticks();
    int durationTicks = rest->ticks().ticks();

    auto nominalTnD
        = timestampAndDurationFromStartAndDurationTicks(rest->score(), positionTick, durationTicks, tickPositionOffset);

    result[nominalTnD.timestamp].emplace_back(mpe::RestEvent(nominalTnD.timestamp, nominalTnD.duration,
                                                             static_cast<voice_layer_idx_t>(rest->voice())));
}
