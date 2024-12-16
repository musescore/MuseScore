/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "noterenderer.h"
#include "gracechordcontext.h"

#include "playback/metaparsers/internal/gracenotesmetaparser.h"
#include "playback/utils/expressionutils.h"

#include "dom/note.h"
#include "dom/guitarbend.h"

using namespace mu::engraving;
using namespace muse;

bool BendsRenderer::isMultibendPart(const Note* note)
{
    if (note->bendFor() || note->bendBack()) {
        return true;
    }

    if (note->tieFor()) {
        const Note* lastTiedNote = note->lastTiedNote(false);
        if (lastTiedNote && lastTiedNote->bendFor()) {
            return true;
        }
    }

    if (note->tieBack()) {
        const Note* firstTiedNote = note->firstTiedNote(false);
        if (firstTiedNote && firstTiedNote->bendBack()) {
            return true;
        }
    }

    return false;
}

void BendsRenderer::render(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    //! NOTE: ignore the grace note and render only the principal note
    if (note->isPreBendStart()) {
        return;
    }

    if (const GuitarBend* bendBack = note->bendBack()) {
        if (bendBack->type() != GuitarBendType::PRE_BEND) {
            return;
        }
    }

    if (!isNotePlayable(note, ctx.commonArticulations)) {
        return;
    }

    renderMultibend(note, ctx, result);
}

void BendsRenderer::renderMultibend(const Note* startNote, const RenderingContext& startNoteCtx,
                                    mpe::PlaybackEventList& result)
{
    const Note* currNote = startNote;
    const GuitarBend* currBend = currNote->bendFor();

    mpe::PlaybackEventList bendEvents;
    BendTimeFactorMap bendTimeFactorMap;

    while (currNote) {
        RenderingContext currNoteCtx = buildRenderingContext(currNote, startNoteCtx);
        appendBendTimeFactors(currNoteCtx.score, currBend, bendTimeFactorMap);

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
            appendBendTimeFactors(currNoteCtx.score, currBend, bendTimeFactorMap);
        } else {
            NoteRenderer::render(currNote, currNoteCtx, bendEvents);
        }

        if (currBend) {
            if (currBend->type() == GuitarBendType::SLIGHT_BEND) {
                bendEvents.emplace_back(buildSlightNoteEvent(currNote, currNoteCtx));
                break;
            }
        }

        if (currNote->tieFor()) {
            currBend = currNote->lastTiedNote(false)->bendFor();
            appendBendTimeFactors(currNoteCtx.score, currBend, bendTimeFactorMap);
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
        if (!muse::contains(GRACE_NOTE_ARTICULATION_TYPES, pair.first)) {
            continue;
        }

        GraceChordCtx graceCtx = GraceChordCtx::buildCtx(principalNote->chord(), pair.first, ctx);

        if (isGraceNotePlacedBeforePrincipalNote(pair.first)) {
            renderGraceNote(graceNote, graceCtx, result);
            NoteRenderer::render(principalNote, graceCtx.principalChordCtx, result);
        } else {
            NoteRenderer::render(principalNote, graceCtx.principalChordCtx, result);
            renderGraceNote(graceNote, graceCtx, result);
        }

        return;
    }
}

void BendsRenderer::renderGraceNote(const Note* note, const GraceChordCtx& ctx, muse::mpe::PlaybackEventList& result)
{
    for (const auto& pair : ctx.graceChordCtxList) {
        for (const Note* graceNote : pair.first->notes()) {
            if (note == graceNote) {
                NoteRenderer::render(note, pair.second, result);
                return;
            }
        }
    }
}

void BendsRenderer::appendBendTimeFactors(const Score* score, const GuitarBend* bend, BendTimeFactorMap& timeFactorMap)
{
    if (!bend) {
        return;
    }

    const float startFactor = std::clamp(bend->startTimeFactor(), 0.f, 1.f);
    const float endFactor = std::clamp(bend->endTimeFactor(), 0.f, 1.f);

    const Note* endNote = bend->endNote();
    const mpe::timestamp_t endNoteTime = timestampFromTicks(score, endNote->tick().ticks());

    IF_ASSERT_FAILED(RealIsEqualOrLess(startFactor, endFactor)) {
        timeFactorMap.insert_or_assign(endNoteTime, BendTimeFactors { 0.f, 1.f });
        return;
    }

    timeFactorMap.insert_or_assign(endNoteTime, BendTimeFactors { startFactor, endFactor });
}

RenderingContext BendsRenderer::buildRenderingContext(const Note* note, const RenderingContext& initialCtx)
{
    const Chord* chord = note->chord();

    if (chord->isGrace()) {
        // Use the principal chord to build the context
        const EngravingItem* parent = chord->parentItem();
        if (parent && parent->isChord()) {
            chord = toChord(parent);
        }
    }

    RenderingContext ctx = engraving::buildRenderingCtx(chord, initialCtx.positionTickOffset,
                                                        initialCtx.profile, initialCtx.playbackCtx);

    if (note->isGrace()) {
        GraceNotesMetaParser::parse(note->chord(), ctx, ctx.commonArticulations);
    }

    return ctx;
}

mpe::NoteEvent BendsRenderer::buildSlightNoteEvent(const Note* note, const RenderingContext& ctx)
{
    NominalNoteCtx slightNoteCtx(note, ctx);
    mpe::timestamp_t timeOffset = slightNoteCtx.duration / 2;

    slightNoteCtx.timestamp += timeOffset;
    slightNoteCtx.duration -= timeOffset;
    slightNoteCtx.pitchLevel += mpe::PITCH_LEVEL_STEP / 2;

    return buildNoteEvent(std::move(slightNoteCtx));
}

mpe::NoteEvent BendsRenderer::buildBendEvent(const Note* startNote, const RenderingContext& startNoteCtx,
                                             const mpe::PlaybackEventList& bendNoteEvents, const BendTimeFactorMap& timeFactorMap)
{
    NominalNoteCtx noteCtx(startNote, startNoteCtx);

    const mpe::NoteEvent& startNoteEvent = std::get<mpe::NoteEvent>(bendNoteEvents.front());
    noteCtx.articulations = startNoteEvent.expressionCtx().articulations;
    noteCtx.timestamp = startNoteEvent.arrangementCtx().actualTimestamp;

    PitchOffsets pitchOffsets;

    auto multibendIt = noteCtx.articulations.find(mpe::ArticulationType::Multibend);
    if (multibendIt != noteCtx.articulations.end()) {
        const mpe::ArticulationMeta& meta = multibendIt->second.meta;
        pitchOffsets.emplace_back(meta.timestamp, 0);
    }

    for (size_t i = 1; i < bendNoteEvents.size(); ++i) {
        const mpe::PlaybackEvent& event = bendNoteEvents.at(i);
        if (!std::holds_alternative<mpe::NoteEvent>(event)) {
            continue;
        }

        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
        const mpe::ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();
        noteCtx.duration = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration - noteCtx.timestamp;

        const mpe::pitch_level_t offset = noteEvent.pitchCtx().nominalPitchLevel - noteCtx.pitchLevel;
        pitchOffsets.emplace_back(arrangementCtx.actualTimestamp, offset);
    }

    if (multibendIt != noteCtx.articulations.end()) {
        mpe::ArticulationMeta& meta = multibendIt->second.meta;
        meta.timestamp = noteCtx.timestamp;
        meta.overallDuration = noteCtx.duration;
    }

    mpe::PitchCurve curve = buildPitchCurve(noteCtx.timestamp, noteCtx.duration, pitchOffsets, timeFactorMap);
    mpe::NoteEvent result = buildNoteEvent(std::move(noteCtx), curve);

    return result;
}

mpe::PitchCurve BendsRenderer::buildPitchCurve(mpe::timestamp_t noteTimestamp, mpe::duration_t noteDuration,
                                               const PitchOffsets& pitchOffsets, const BendTimeFactorMap& timeFactorMap)
{
    auto findFactorsAtTime = [&timeFactorMap](mpe::timestamp_t time) -> const BendTimeFactors& {
        auto it = muse::findLessOrEqual(timeFactorMap, time);
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
        const float ratio = static_cast<float>(pair.first - noteTimestamp) / noteDuration;

        const mpe::percentage_t nominalOffsetPercent = static_cast<mpe::percentage_t>(ratio * 100.f) * mpe::ONE_PERCENT;
        const mpe::percentage_t nominalPercentDiff = nominalOffsetPercent - prevNominalOffsetPrecent;

        const mpe::percentage_t actualOffsetStartPercent = prevNominalOffsetPrecent + nominalPercentDiff * factors.startFactor;
        const mpe::percentage_t actualOffsetEndPercent = prevNominalOffsetPrecent + nominalPercentDiff * factors.endFactor;

        prevNominalOffsetPrecent = nominalOffsetPercent;

        const auto& prevOffset = result.rbegin();
        result.insert_or_assign(actualOffsetStartPercent, prevOffset->second);
        result.insert_or_assign(actualOffsetEndPercent, pair.second);
    }

    return result;
}
