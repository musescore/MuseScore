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

#include "utils/arrangementutils.h"
#include "metaparsers/chordarticulationsparser.h"
#include "metaparsers/notearticulationsparser.h"
#include "renderers/ornamentsrenderer.h"
#include "renderers/tremolorenderer.h"
#include "renderers/arpeggiorenderer.h"
#include "renderers/gracenotesrenderer.h"
#include "renderers/glissandosrenderer.h"

using namespace mu::engraving;
using namespace mu::mpe;

void PlaybackEventsRenderer::render(const Ms::EngravingItem* item, const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied,
                                    const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    render(item, 0, nominalDynamicLevel, persistentArticulationApplied, profile, result);
}

void PlaybackEventsRenderer::render(const Ms::EngravingItem* item, const int tickPositionOffset, const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    TRACEFUNC;

    IF_ASSERT_FAILED(item->isChordRest()) {
        return;
    }

    if (item->type() == Ms::ElementType::CHORD) {
        renderNoteEvents(Ms::toChord(item), tickPositionOffset, nominalDynamicLevel, persistentArticulationApplied, profile, result);
    } else if (item->type() == Ms::ElementType::REST) {
        renderRestEvents(Ms::toRest(item), tickPositionOffset, result);
    }
}

void PlaybackEventsRenderer::render(const Ms::EngravingItem* item, const mpe::timestamp_t actualTimestamp,
                                    const mpe::duration_t actualDuration, const mpe::dynamic_level_t actualDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    TRACEFUNC;

    IF_ASSERT_FAILED(item->isChordRest() || item->isNote()) {
        return;
    }

    if (item->type() == Ms::ElementType::CHORD) {
        const Ms::Chord* chord = Ms::toChord(item);

        for (const Ms::Note* note : chord->notes()) {
            renderFixedNoteEvent(note, actualTimestamp, actualDuration,
                                 actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
        }
    } else if (item->type() == Ms::ElementType::NOTE) {
        renderFixedNoteEvent(Ms::toNote(item), actualTimestamp, actualDuration,
                             actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
    } else if (item->type() == Ms::ElementType::REST) {
        renderRestEvents(Ms::toRest(item), 0, result);
    }
}

void PlaybackEventsRenderer::renderMetronome(const Ms::Score* score, const int positionTick, const int durationTicks,
                                             const int ticksPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    Ms::BeatType beatType = score->tick2beatType(Ms::Fraction::fromTicks(positionTick));

    if (beatType == Ms::BeatType::SUBBEAT) {
        return;
    }

    Ms::TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(positionTick).timesig();
    int ticksPerBeat = timeSignatureFraction.ticks() / timeSignatureFraction.numerator();

    BeatsPerSecond bps = score->tempomap()->tempo(positionTick);

    int eventsCount = std::max(1, durationTicks / ticksPerBeat);

    for (int i = 0; i < eventsCount; ++i) {
        static ArticulationMap emptyArticulations;

        timestamp_t eventTimestamp = timestampFromTicks(score, positionTick + ticksPositionOffset + i * ticksPerBeat);

        pitch_level_t eventPitchLevel = pitchLevel(PitchClass::A, 4);
        if (beatType == Ms::BeatType::DOWNBEAT && i == 0) {
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

void PlaybackEventsRenderer::renderNoteEvents(const Ms::Chord* chord, const int tickPositionOffset,
                                              const mpe::dynamic_level_t nominalDynamicLevel,
                                              const ArticulationType persistentArticulationApplied,
                                              const mpe::ArticulationsProfilePtr profile, PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(chord) {
        return;
    }

    int chordPosTick = chord->tick().ticks() + tickPositionOffset;
    int chordDurationTicks = chord->durationTypeTicks().ticks();
    BeatsPerSecond bps = chord->score()->tempomap()->tempo(chordPosTick);

    static ArticulationMap articulations;

    RenderingContext ctx(timestampFromTicks(chord->score(), chordPosTick),
                         durationFromTicks(bps.val, chordDurationTicks),
                         nominalDynamicLevel,
                         chordPosTick,
                         chordDurationTicks,
                         bps,
                         persistentArticulationApplied,
                         articulations,
                         profile);

    ChordArticulationsParser::buildChordArticulationMap(chord, ctx, ctx.commonArticulations);

    renderArticulations(chord, ctx, result[ctx.nominalTimestamp]);
}

void PlaybackEventsRenderer::renderFixedNoteEvent(const Ms::Note* note, const mpe::timestamp_t actualTimestamp,
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

void PlaybackEventsRenderer::renderRestEvents(const Ms::Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(rest) {
        return;
    }

    int positionTick = rest->tick().ticks() + tickPositionOffset;
    int durationTicks = rest->ticks().ticks();
    qreal beatsPerSecond = rest->score()->tempomap()->tempo(positionTick).val;

    timestamp_t nominalTimestamp = timestampFromTicks(rest->score(), positionTick);
    duration_t nominalDuration = durationFromTicks(beatsPerSecond, durationTicks);

    result[nominalTimestamp].emplace_back(mpe::RestEvent(nominalTimestamp, nominalDuration, rest->voice()));
}

void PlaybackEventsRenderer::renderArticulations(const Ms::Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    if (renderChordArticulations(chord, ctx, result)) {
        return;
    }

    renderNoteArticulations(chord, ctx, result);
}

bool PlaybackEventsRenderer::renderChordArticulations(const Ms::Chord* chord, const RenderingContext& ctx,
                                                      mpe::PlaybackEventList& result) const
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

void PlaybackEventsRenderer::renderNoteArticulations(const Ms::Chord* chord, const RenderingContext& ctx,
                                                     mpe::PlaybackEventList& result) const
{
    for (const Ms::Note* note : chord->notes()) {
        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, ctx);

        NoteArticulationsParser::buildNoteArticulationMap(note, ctx, noteCtx.chordCtx.commonArticulations);

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
