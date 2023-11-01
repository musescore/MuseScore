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

static const GuitarBend* findBend(const Note* note)
{
    return note ? note->bendFor() : nullptr;
}

static bool skipNote(const Note* note, const mu::mpe::ArticulationMap& articualtionMap)
{
    if (!isNotePlayable(note, articualtionMap)) {
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
    const GuitarBend* bend = findBend(startNote);
    if (!bend) {
        ChordArticulationsRenderer::renderNote(startNote->chord(), startNote, startNoteCtx, result);
        return;
    }

    const Note* currNote = startNote;
    mpe::PlaybackEventList bendEvents;

    std::vector<const GuitarBend*> bends;

    while (currNote && bend) {
        RenderingContext currNoteCtx = buildRenderingContext(score, currNote, startNoteCtx);
        bends.push_back(bend);

        switch (bend->type()) {
        case GuitarBendType::BEND:
        case GuitarBendType::PRE_BEND:
            ChordArticulationsRenderer::renderNote(currNote->chord(), currNote, currNoteCtx, bendEvents);

            currNote = bend->endNote();
            bend = findBend(currNote);
            break;
        case GuitarBendType::SLIGHT_BEND:
            renderSlightBend(currNote, currNoteCtx, bendEvents);

            currNote = nullptr;
            bend = nullptr;
            break;
        case GuitarBendType::GRACE_NOTE_BEND: {
            const Note* principalNote = bend->endNote();
            renderGraceNote(currNote, principalNote, currNoteCtx, bendEvents);

            //! NOTE: skip the grace and the principal note, GraceChordsRenderer renders them both
            bend = findBend(principalNote);
            if (bend) {
                bends.push_back(bend);
            }

            currNote = bend ? bend->endNote() : nullptr;
            bend = findBend(currNote);
        } break;
        }
    }

    if (currNote) {
        RenderingContext currNoteCtx = buildRenderingContext(score, currNote, startNoteCtx);
        ChordArticulationsRenderer::renderNote(currNote->chord(), currNote, currNoteCtx, bendEvents);
    }

    if (!bendEvents.empty()) {
        BendTimeFactorMap bendTimeFactorMap = buildBendTimeFactorMap(score, bends);
        mpe::NoteEvent event = buildBendEvent(startNote, startNoteCtx, bendEvents, bendTimeFactorMap);
        result.emplace_back(std::move(event));
    }
}

void BendsRenderer::renderGraceNote(const Note* graceNote, const Note* principalNote, const RenderingContext& ctx,
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

void BendsRenderer::renderSlightBend(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    ChordArticulationsRenderer::renderNote(note->chord(), note, ctx, result);

    NominalNoteCtx slightNoteCtx(note, ctx);
    mpe::timestamp_t timeOffset = slightNoteCtx.duration / 2;

    slightNoteCtx.timestamp += timeOffset;
    slightNoteCtx.duration -= timeOffset;
    slightNoteCtx.pitchLevel += mpe::PITCH_LEVEL_STEP / 2;

    result.emplace_back(buildNoteEvent(std::move(slightNoteCtx)));
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

BendsRenderer::BendTimeFactorMap BendsRenderer::buildBendTimeFactorMap(const Score* score, const std::vector<const GuitarBend*>& bends)
{
    BendTimeFactorMap result;

    for (const GuitarBend* bend : bends) {
        float startFactor = std::clamp(bend->startTimeFactor(), 0.f, 1.f);
        float endFactor = std::clamp(bend->endTimeFactor(), 0.f, 1.f);

        const Note* endNote = bend->endNote();
        mpe::timestamp_t endNoteTime = timestampFromTicks(score, endNote->tick().ticks());

        IF_ASSERT_FAILED(RealIsEqualOrLess(startFactor, endFactor)) {
            result.insert_or_assign(endNoteTime, BendTimeFactors { 0.f, 1.f });
            continue;
        }

        result.insert_or_assign(endNoteTime, BendTimeFactors { startFactor, endFactor });
    }

    return result;
}

mu::mpe::NoteEvent BendsRenderer::buildBendEvent(const Note* startNote, const RenderingContext& startNoteCtx,
                                                 const mpe::PlaybackEventList& bendNoteEvents, const BendTimeFactorMap& timeFactorMap)
{
    NominalNoteCtx noteCtx(startNote, startNoteCtx);

    const mpe::NoteEvent& startNoteEvent = std::get<mpe::NoteEvent>(bendNoteEvents.front());
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
