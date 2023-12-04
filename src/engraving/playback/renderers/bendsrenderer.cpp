/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "bendsrenderer.h"

#include "chordarticulationsrenderer.h"
#include "gracechordsrenderer.h"

#include "playback/metaparsers/internal/gracenotesmetaparser.h"

#include "dom/note.h"
#include "dom/guitarbend.h"
#include "dom/tempo.h"

using namespace mu::engraving;

static bool skipNote(const Note* note, const mu::mpe::ArticulationMap& articulationMap)
{
    if (!isNotePlayable(note, articulationMap)) {
        return true;
    }

    //! NOTE: ignore the grace note and render only the principal note
    if (note->isPreBendStart()) {
        return true;
    }

    const GuitarBend* bend = note->bendBack();
    if (bend) {
        return bend->type() != GuitarBendType::PRE_BEND;
    }

    return false;
}

const mu::mpe::ArticulationTypeSet& BendsRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Multibend,
    };

    return types;
}

void BendsRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType preferredType,
                             const RenderingContext& ctx,
                             mpe::PlaybackEventList& result)
{
    UNUSED(preferredType);

    IF_ASSERT_FAILED(item->type() == ElementType::CHORD) {
        return;
    }

    const Chord* chord = toChord(item);
    const Score* score = chord->score();

    IF_ASSERT_FAILED(score) {
        return;
    }

    for (const Note* note : chord->notes()) {
        if (skipNote(note, ctx.commonArticulations)) {
            continue;
        }

        renderMultibend(score, note, ctx, result);
    }

    for (const Chord* graceChord : chord->graceNotes()) {
        for (const Note* note : graceChord->notes()) {
            if (skipNote(note, ctx.commonArticulations)) {
                continue;
            }

            renderMultibend(score, note, ctx, result);
        }
    }
}

void BendsRenderer::renderMultibend(const Score* score, const Note* startNote, const RenderingContext& startNoteCtx,
                                    mpe::PlaybackEventList& result)
{
    const Note* currNote = startNote;
    const GuitarBend* currBend = currNote->bendFor();

    mpe::PlaybackEventList bendEvents;
    BendTimeFactorMap bendTimeFactorMap;

    while (currNote) {
        RenderingContext currNoteCtx = buildRenderingContext(score, currNote, startNoteCtx);
        appendBendTimeFactors(score, currBend, bendTimeFactorMap);

        if (currNote->isGrace()) {
            IF_ASSERT_FAILED(currBend) {
                break;
            }

            const Note* principalNote = currBend->endNote();
            IF_ASSERT_FAILED(principalNote) {
                break;
            }

            renderGraceAndPrincipalNotes(currNote, principalNote, currNoteCtx, bendEvents);

            // Skip the principal note, it's already been rendered
            currNote = principalNote;
            currBend = principalNote->bendFor();
            appendBendTimeFactors(score, currBend, bendTimeFactorMap);
        } else {
            ChordArticulationsRenderer::renderNote(currNote->chord(), currNote, currNoteCtx, bendEvents);
        }

        if (currBend) {
            if (currBend->type() == GuitarBendType::SLIGHT_BEND) {
                bendEvents.emplace_back(buildSlightNoteEvent(currNote, currNoteCtx));
                break;
            }
        }

        if (currNote->tieFor()) {
            currBend = currNote->lastTiedNote()->bendFor();
            appendBendTimeFactors(score, currBend, bendTimeFactorMap);
        }

        currNote = currBend ? currBend->endNote() : nullptr;
        currBend = currNote ? currNote->bendFor() : nullptr;
    }

    if (!bendEvents.empty()) {
        mpe::NoteEvent event = buildBendEvent(startNote, startNoteCtx, bendEvents, bendTimeFactorMap);
        result.emplace_back(std::move(event));
    }
}

void BendsRenderer::renderGraceAndPrincipalNotes(const Note* graceNote, const Note* principalNote, const RenderingContext& ctx,
                                                 mpe::PlaybackEventList& result)
{
    IF_ASSERT_FAILED(graceNote && graceNote->isGrace() && principalNote) {
        return;
    }

    for (const auto& pair : ctx.commonArticulations) {
        if (GraceChordsRenderer::isAbleToRender(pair.first)) {
            GraceChordsRenderer::renderGraceNote(graceNote, principalNote, pair.first, ctx, result);
            return;
        }
    }
}

void BendsRenderer::appendBendTimeFactors(const Score* score, const GuitarBend* bend, BendTimeFactorMap& timeFactorMap)
{
    if (!bend) {
        return;
    }

    float startFactor = std::clamp(bend->startTimeFactor(), 0.f, 1.f);
    float endFactor = std::clamp(bend->endTimeFactor(), 0.f, 1.f);

    const Note* endNote = bend->endNote();
    mpe::timestamp_t endNoteTime = timestampFromTicks(score, endNote->tick().ticks());

    IF_ASSERT_FAILED(RealIsEqualOrLess(startFactor, endFactor)) {
        timeFactorMap.insert_or_assign(endNoteTime, BendTimeFactors { 0.f, 1.f });
    }

    timeFactorMap.insert_or_assign(endNoteTime, BendTimeFactors { startFactor, endFactor });
}

RenderingContext BendsRenderer::buildRenderingContext(const Score* score, const Note* note, const RenderingContext& initialCtx)
{
    const Chord* chord = note->chord();

    if (chord->isGrace()) {
        // Use the principal chord to build the context
        const EngravingItem* parent = chord->parentItem();
        if (parent && parent->isChord()) {
            chord = toChord(parent);
        }
    }

    int chordPosTicks = chord->tick().ticks();
    int chordDurationTicks = chord->actualTicks().ticks();

    BeatsPerSecond bps = score->tempomap()->tempo(chordPosTicks);

    RenderingContext ctx(timestampFromTicks(score, chordPosTicks + initialCtx.positionTickOffset),
                         durationFromTicks(bps.val, chordDurationTicks),
                         initialCtx.nominalDynamicLevel,
                         chordPosTicks,
                         initialCtx.positionTickOffset,
                         chordDurationTicks,
                         bps,
                         initialCtx.timeSignatureFraction,
                         initialCtx.persistentArticulation,
                         initialCtx.commonArticulations,
                         initialCtx.profile);

    if (note->isGrace()) {
        GraceNotesMetaParser::parse(note->chord(), ctx, ctx.commonArticulations);
    }

    return ctx;
}

mu::mpe::NoteEvent BendsRenderer::buildSlightNoteEvent(const Note* note, const RenderingContext& ctx)
{
    NominalNoteCtx slightNoteCtx(note, ctx);
    mpe::timestamp_t timeOffset = slightNoteCtx.duration / 2;

    slightNoteCtx.timestamp += timeOffset;
    slightNoteCtx.duration -= timeOffset;
    slightNoteCtx.pitchLevel += mpe::PITCH_LEVEL_STEP / 2;

    return buildNoteEvent(std::move(slightNoteCtx));
}

mu::mpe::NoteEvent BendsRenderer::buildBendEvent(const Note* startNote, const RenderingContext& startNoteCtx,
                                                 const mpe::PlaybackEventList& bendNoteEvents, const BendTimeFactorMap& timeFactorMap)
{
    NominalNoteCtx noteCtx(startNote, startNoteCtx);

    const mpe::NoteEvent& startNoteEvent = std::get<mpe::NoteEvent>(bendNoteEvents.front());
    noteCtx.chordCtx.commonArticulations = startNoteEvent.expressionCtx().articulations;
    noteCtx.timestamp = startNoteEvent.arrangementCtx().actualTimestamp;

    PitchOffsets pitchOffsets;

    for (size_t i = 1; i < bendNoteEvents.size(); ++i) {
        const mpe::PlaybackEvent& event = bendNoteEvents.at(i);
        if (!std::holds_alternative<mpe::NoteEvent>(event)) {
            continue;
        }

        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
        const mpe::ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();
        noteCtx.duration = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration - noteCtx.timestamp;

        mpe::timestamp_t offsetTime = arrangementCtx.actualTimestamp;
        mpe::pitch_level_t offset = noteEvent.pitchCtx().nominalPitchLevel - noteCtx.pitchLevel;
        pitchOffsets.emplace_back(offsetTime, offset);
    }

    mpe::PitchCurve curve = buildPitchCurve(noteCtx.timestamp, noteCtx.duration, pitchOffsets, timeFactorMap);
    mpe::NoteEvent result = buildNoteEvent(std::move(noteCtx), curve);

    return result;
}

mu::mpe::PitchCurve BendsRenderer::buildPitchCurve(mpe::timestamp_t bendStartTime, mpe::duration_t totalBendDuration,
                                                   const PitchOffsets& pitchOffsets, const BendTimeFactorMap& timeFactorMap)
{
    auto findFactorsAtTime = [&timeFactorMap](mpe::timestamp_t time) -> const BendTimeFactors& {
        auto it = mu::findLessOrEqual(timeFactorMap, time);
        if (it == timeFactorMap.end()) {
            static const BendTimeFactors dummy;
            return dummy;
        }

        return it->second;
    };

    mpe::PitchCurve result;
    result.emplace(0, 0);

    mpe::percentage_t prevNominalOffsetPrecent = 0;

    for (const auto& pair : pitchOffsets) {
        const BendTimeFactors& factors = findFactorsAtTime(pair.first);
        float ratio = static_cast<float>(pair.first - bendStartTime) / totalBendDuration;

        mpe::percentage_t nominalOffsetPercent = static_cast<mpe::percentage_t>(ratio * 100.f) * mpe::ONE_PERCENT;
        mpe::percentage_t nominalPercentDiff = nominalOffsetPercent - prevNominalOffsetPrecent;

        mpe::percentage_t actualOffsetStartPercent = prevNominalOffsetPrecent + nominalPercentDiff * factors.startFactor;
        mpe::percentage_t actualOffsetEndPercent = prevNominalOffsetPrecent + nominalPercentDiff * factors.endFactor;

        prevNominalOffsetPrecent = nominalOffsetPercent;

        const auto& prevOffset = result.rbegin();
        result.insert_or_assign(actualOffsetStartPercent, prevOffset->second);
        result.insert_or_assign(actualOffsetEndPercent, pair.second);
    }

    return result;
}
