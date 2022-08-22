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
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/sig.h"
#include "libmscore/harmony.h"
#include "libmscore/staff.h"
#include "libmscore/swing.h"

#include "utils/arrangementutils.h"
#include "metaparsers/chordarticulationsparser.h"
#include "metaparsers/notearticulationsparser.h"
#include "renderers/ornamentsrenderer.h"
#include "renderers/tremolorenderer.h"
#include "renderers/arpeggiorenderer.h"
#include "renderers/gracenotesrenderer.h"
#include "renderers/glissandosrenderer.h"
#include "filters/chordfilter.h"

using namespace mu::engraving;
using namespace mu::mpe;

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
    TRACEFUNC;

    IF_ASSERT_FAILED(item->isChordRest()) {
        return;
    }

    if (item->type() == ElementType::CHORD) {
        renderNoteEvents(toChord(
                             item), tickPositionOffset, nominalDynamicLevel, persistentArticulationApplied, profile, result);
    } else if (item->type() == ElementType::REST) {
        renderRestEvents(toRest(item), tickPositionOffset, result);
    }
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const mpe::timestamp_t actualTimestamp,
                                    const mpe::duration_t actualDuration, const mpe::dynamic_level_t actualDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    TRACEFUNC;

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
                                               const int ticksPositionOffset, mpe::PlaybackEventsMap& result) const
{
    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    const Score* score = chordSymbol->score();
    int positionTick = chordSymbol->tick().ticks() + ticksPositionOffset;

    timestamp_t eventTimestamp = timestampFromTicks(score, positionTick);
    PlaybackEventList& events = result[eventTimestamp];

    int durationTicks = realized.getActualDuration(positionTick).ticks();
    BeatsPerSecond bps = score->tempomap()->tempo(positionTick);
    duration_t duration = durationFromTicks(bps.val, durationTicks);

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    static ArticulationMap emptyArticulations;

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int octave = playingOctave(it->first, it->second);
        pitch_level_t pitchLevel = notePitchLevel(it->second, octave);

        events.emplace_back(mpe::NoteEvent(eventTimestamp,
                                           duration,
                                           voiceIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           emptyArticulations));
    }
}

void PlaybackEventsRenderer::renderChordSymbol(const Harmony* chordSymbol, const mpe::timestamp_t actualTimestamp,
                                               const mpe::duration_t actualDuration, mpe::PlaybackEventsMap& result) const
{
    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    PlaybackEventList& events = result[actualTimestamp];

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    static ArticulationMap emptyArticulations;

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int octave = playingOctave(it->first, it->second);
        pitch_level_t pitchLevel = notePitchLevel(it->second, octave);

        events.emplace_back(mpe::NoteEvent(actualTimestamp,
                                           actualDuration,
                                           voiceIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           emptyArticulations));
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick,
                                             const int ticksPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(measureStartTick).timesig();
    int ticksPerBeat = timeSignatureFraction.ticks() / timeSignatureFraction.numerator();

    BeatsPerSecond bps = score->tempomap()->tempo(measureStartTick);

    int step = timeSignatureFraction.isBeatedCompound(bps.val)
               ? timeSignatureFraction.beatTicks() : timeSignatureFraction.dUnitTicks();

    for (int tick = measureStartTick; tick < measureEndTick; tick += step) {
        static ArticulationMap emptyArticulations;

        timestamp_t eventTimestamp = timestampFromTicks(score, tick + ticksPositionOffset);

        BeatType beatType = score->tick2beatType(Fraction::fromTicks(tick));
        pitch_level_t eventPitchLevel = pitchLevel(PitchClass::A, 4);
        if (beatType == BeatType::DOWNBEAT && tick == 0) {
            eventPitchLevel = pitchLevel(PitchClass::B, 4);
        }

        result[eventTimestamp].emplace_back(mpe::NoteEvent(eventTimestamp,
                                                           durationFromTicks(bps.val, ticksPerBeat, ticksPerBeat),
                                                           0,
                                                           eventPitchLevel,
                                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                                           emptyArticulations));
    }
}

void PlaybackEventsRenderer::renderNoteEvents(const Chord* chord, const int tickPositionOffset,
                                              const mpe::dynamic_level_t nominalDynamicLevel,
                                              const ArticulationType persistentArticulationApplied,
                                              const mpe::ArticulationsProfilePtr profile, PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(chord) {
        return;
    }

    int chordPosTick = chord->tick().ticks() + tickPositionOffset;
    int chordDurationTicks = chord->durationTypeTicks().ticks();

    const Score* score = chord->score();

    BeatsPerSecond bps = score->tempomap()->tempo(chordPosTick);
    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(chordPosTick).timesig();

    static ArticulationMap articulations;

    RenderingContext ctx(timestampFromTicks(chord->score(), chordPosTick),
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
    ArticulationMeta meta(persistentArticulationApplied,
                          profile->pattern(persistentArticulationApplied),
                          actualTimestamp,
                          actualDuration,
                          0,
                          0);

    ArticulationMap articulations;
    articulations.emplace(persistentArticulationApplied, mpe::ArticulationAppliedData(std::move(meta), 0, mpe::HUNDRED_PERCENT));
    articulations.preCalculateAverageData();

    result.emplace_back(buildFixedNoteEvent(note, actualTimestamp, actualDuration, actualDynamicLevel, articulations));
}

void PlaybackEventsRenderer::renderRestEvents(const Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(rest) {
        return;
    }

    int positionTick = rest->tick().ticks() + tickPositionOffset;
    int durationTicks = rest->ticks().ticks();
    double beatsPerSecond = rest->score()->tempomap()->tempo(positionTick).val;

    timestamp_t nominalTimestamp = timestampFromTicks(rest->score(), positionTick);
    duration_t nominalDuration = durationFromTicks(beatsPerSecond, durationTicks);

    result[nominalTimestamp].emplace_back(mpe::RestEvent(nominalTimestamp, nominalDuration, static_cast<voice_layer_idx_t>(rest->voice())));
}

void PlaybackEventsRenderer::renderArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    if (renderChordArticulations(chord, ctx, result)) {
        return;
    }

    renderNoteArticulations(chord, ctx, result);
}

bool PlaybackEventsRenderer::renderChordArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    for (const auto& pair : ctx.commonArticulations) {
        const ArticulationType type = pair.first;

        if (OrnamentsRenderer::isAbleToRender(type)) {
            OrnamentsRenderer::render(chord, type, ctx, result);
            return true;
        }

        if (TremoloRenderer::isAbleToRender(type)) {
            TremoloRenderer::render(chord, type, ctx, result);
            return true;
        }

        if (ArpeggioRenderer::isAbleToRender(type)) {
            ArpeggioRenderer::render(chord, type, ctx, result);
            return true;
        }

        if (GraceNotesRenderer::isAbleToRender(type)) {
            GraceNotesRenderer::render(chord, type, ctx, result);
            return true;
        }
    }

    return false;
}

void PlaybackEventsRenderer::renderNoteArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    Swing::ChordDurationAdjustment swingDurationAdjustment;

    if (!chord->tuplet()) {
        SwingParameters swing = chord->staff()->swing(chord->tick());

        if (swing.isOn()) {
            swingDurationAdjustment = Swing::applySwing(chord, swing);
        }
    }

    for (const Note* note : chord->notes()) {
        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, ctx);

        NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.chordCtx.commonArticulations);

        if (!swingDurationAdjustment.isNull()) {
            //! NOTE: Swing must be applied to the "raw" note duration, but not to the additional duration (e.g, from a tied note)
            duration_t additionalDuration = noteCtx.duration - ctx.nominalDuration;
            noteCtx.timestamp = noteCtx.timestamp + ctx.nominalDuration * swingDurationAdjustment.remainingDurationMultiplier;
            noteCtx.duration = ctx.nominalDuration * swingDurationAdjustment.durationMultiplier + additionalDuration;
        }

        if (note->tieFor()) {
            const Note* lastTiedNote = note->lastTiedNote();
            noteCtx.duration += lastTiedNoteDurationOffset(lastTiedNote, ctx);
            result.emplace_back(buildNoteEvent(std::move(noteCtx)));
            continue;
        }

        if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::DiscreteGlissando)) {
            GlissandosRenderer::render(note, ArticulationType::DiscreteGlissando, noteCtx.chordCtx, result);
            continue;
        }

        if (noteCtx.chordCtx.commonArticulations.contains(ArticulationType::ContinuousGlissando)) {
            GlissandosRenderer::render(note, ArticulationType::ContinuousGlissando, noteCtx.chordCtx, result);
            continue;
        }

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}

duration_t PlaybackEventsRenderer::lastTiedNoteDurationOffset(const Note* lastTiedNote, const RenderingContext& ctx) const
{
    NominalNoteCtx lastTiedNoteCtx(lastTiedNote, ctx);
    lastTiedNoteCtx.duration = durationFromTicks(ctx.beatsPerSecond.val, lastTiedNote->chord()->ticks().ticks());

    ChordArticulationsParser::buildChordArticulationMap(lastTiedNote->chord(), ctx, lastTiedNoteCtx.chordCtx.commonArticulations);

    mpe::NoteEvent lastTiedNoteEvent = buildNoteEvent(std::move(lastTiedNoteCtx));

    return lastTiedNoteEvent.arrangementCtx().actualDuration - lastTiedNoteEvent.arrangementCtx().nominalDuration;
}
