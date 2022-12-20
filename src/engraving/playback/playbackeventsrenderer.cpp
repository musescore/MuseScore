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

#include "playbackeventsrenderer.h"

#include "log.h"

#include "libmscore/chord.h"
#include "libmscore/harmony.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/sig.h"
#include "libmscore/tempo.h"

#include "utils/arrangementutils.h"
#include "metaparsers/chordarticulationsparser.h"

#include "renderers/gracechordsrenderer.h"
#include "renderers/chordarticulationsrenderer.h"
#include "filters/chordfilter.h"

using namespace mu::engraving;
using namespace mu::mpe;

static ArticulationMap makeArticulations(ArticulationType persistentArticulationApplied, ArticulationsProfilePtr profile,
                                         timestamp_t timestamp, duration_t duration)
{
    ArticulationMeta meta(persistentArticulationApplied,
                          profile->pattern(persistentArticulationApplied),
                          timestamp,
                          duration,
                          0,
                          0);

    ArticulationMap articulations;
    articulations.emplace(persistentArticulationApplied, mu::mpe::ArticulationAppliedData(std::move(meta), 0, mu::mpe::HUNDRED_PERCENT));
    articulations.preCalculateAverageData();

    return articulations;
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied,
                                    const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    render(item, 0, nominalDynamicLevel, persistentArticulationApplied, profile, result);
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const int tickPositionOffset,
                                    const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(item->isChordRest()) {
        return;
    }

    if (item->type() == ElementType::CHORD) {
        renderNoteEvents(toChord(item), tickPositionOffset, nominalDynamicLevel, persistentArticulationApplied, profile, result);
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

    if (item->type() == ElementType::CHORD) {
        const Chord* chord = toChord(item);

        for (const Note* note : chord->notes()) {
            renderFixedNoteEvent(note, actualTimestamp, actualDuration,
                                 actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
        }
    } else if (item->type() == ElementType::NOTE) {
        renderFixedNoteEvent(toNote(item), actualTimestamp, actualDuration,
                             actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
    } else if (item->type() == ElementType::REST) {
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

    int durationTicks = realized.getActualDuration(positionTick).ticks();
    BeatsPerSecond bps = score->tempomap()->tempo(positionTick);
    duration_t duration = durationFromTicks(bps.val, durationTicks);

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());

    ArticulationMap articulations = makeArticulations(mpe::ArticulationType::Standard, profile, eventTimestamp, duration);

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int octave = playingOctave(it->first, it->second);
        pitch_level_t pitchLevel = notePitchLevel(it->second, octave);

        events.emplace_back(mpe::NoteEvent(eventTimestamp,
                                           duration,
                                           voiceIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           bps.val));
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

    ArticulationMap articulations = makeArticulations(mpe::ArticulationType::Standard, profile, actualTimestamp, actualDuration);

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int octave = playingOctave(it->first, it->second);
        pitch_level_t pitchLevel = notePitchLevel(it->second, octave);

        events.emplace_back(mpe::NoteEvent(actualTimestamp,
                                           actualDuration,
                                           voiceIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           2.0));
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick,
                                             const int ticksPositionOffset, mpe::PlaybackEventsMap& result) const
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

        renderMetronome(score, tick, eventTimestamp, result);
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int tick, const mpe::timestamp_t actualTimestamp,
                                             mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    BeatsPerSecond bps = score->tempomap()->tempo(tick);
    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(tick).timesig();
    int ticksPerBeat = timeSignatureFraction.ticks() / timeSignatureFraction.numerator();
    static ArticulationMap emptyArticulations;

    BeatType beatType = score->tick2beatType(Fraction::fromTicks(tick));
    pitch_level_t eventPitchLevel = pitchLevel(PitchClass::A, 4);
    if (beatType == BeatType::DOWNBEAT && tick == 0) {
        eventPitchLevel = pitchLevel(PitchClass::B, 4);
    }

    result[actualTimestamp].emplace_back(mpe::NoteEvent(actualTimestamp,
                                                        durationFromTicks(bps.val, ticksPerBeat, ticksPerBeat),
                                                        0,
                                                        eventPitchLevel,
                                                        dynamicLevelFromType(mpe::DynamicType::Natural),
                                                        emptyArticulations,
                                                        bps.val));
}

void PlaybackEventsRenderer::renderNoteEvents(const Chord* chord, const int tickPositionOffset,
                                              const mpe::dynamic_level_t nominalDynamicLevel,
                                              const ArticulationType persistentArticulationApplied,
                                              const mpe::ArticulationsProfilePtr profile, PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(chord) {
        return;
    }

    int chordPosTick = chord->tick().ticks();
    int chordDurationTicks = chord->actualTicks().ticks();

    const Score* score = chord->score();

    BeatsPerSecond bps = score->tempomap()->tempo(chordPosTick);
    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(chordPosTick).timesig();

    static ArticulationMap articulations;

    RenderingContext ctx(timestampFromTicks(chord->score(), chordPosTick + tickPositionOffset),
                         durationFromTicks(bps.val, chordDurationTicks),
                         nominalDynamicLevel,
                         chord->tick().ticks(),
                         tickPositionOffset,
                         chordDurationTicks,
                         bps,
                         timeSignatureFraction,
                         persistentArticulationApplied,
                         articulations,
                         profile);

    if (!ChordFilter::isItemPlayable(chord, ctx)) {
        return;
    }

    ChordArticulationsParser::buildChordArticulationMap(chord, ctx, ctx.commonArticulations);

    renderArticulations(chord, ctx, result[ctx.nominalTimestamp]);
}

void PlaybackEventsRenderer::renderFixedNoteEvent(const Note* note, const mpe::timestamp_t actualTimestamp,
                                                  const mpe::duration_t actualDuration,
                                                  const mpe::dynamic_level_t actualDynamicLevel,
                                                  const mpe::ArticulationType persistentArticulationApplied,
                                                  const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventList& result) const
{
    ArticulationMap articulations = makeArticulations(persistentArticulationApplied, profile, actualDuration, actualTimestamp);

    result.emplace_back(buildFixedNoteEvent(note, actualTimestamp, actualDuration, actualDynamicLevel, articulations));
}

void PlaybackEventsRenderer::renderRestEvents(const Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(rest) {
        return;
    }

    int positionTick = rest->tick().ticks();
    int durationTicks = rest->ticks().ticks();
    double beatsPerSecond = rest->score()->tempomap()->tempo(positionTick).val;

    timestamp_t nominalTimestamp = timestampFromTicks(rest->score(), positionTick + tickPositionOffset);
    duration_t nominalDuration = durationFromTicks(beatsPerSecond, durationTicks);

    result[nominalTimestamp].emplace_back(mpe::RestEvent(nominalTimestamp, nominalDuration, static_cast<voice_layer_idx_t>(rest->voice())));
}

void PlaybackEventsRenderer::renderArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    for (const auto& type : ctx.commonArticulations) {
        if (GraceChordsRenderer::isAbleToRender(type.first)) {
            GraceChordsRenderer::render(chord, type.first, ctx, result);
            return;
        }
    }

    ChordArticulationsRenderer::render(chord, ArticulationType::Last, ctx, result);
}
