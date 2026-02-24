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

#include "playback/metaparsers/chordarticulationsparser.h"
#include "playback/utils/expressionutils.h"

#include "dom/note.h"
#include "dom/guitarbend.h"

using namespace mu::engraving;
using namespace muse;

static constexpr bool IGNORE_UNPLAYABLE = false;

static const Chord* principalChord(const Chord* chord)
{
    if (chord->isGrace()) {
        const EngravingItem* parent = chord->parentItem();
        if (parent && parent->isChord()) {
            return toChord(parent);
        }
    }

    return chord;
}

static mpe::pitch_level_t pitchOffset(const GuitarBend* bend)
{
    return mpe::PITCH_LEVEL_STEP / 2 * bend->bendAmountInQuarterTones();
}

bool BendsRenderer::isMultibendPart(const Note* note)
{
    if (note->bendFor() || note->bendBack()) {
        return true;
    }

    if (note->tieFor()) {
        const Note* lastTiedNote = note->lastTiedNote(IGNORE_UNPLAYABLE);
        if (lastTiedNote && lastTiedNote->bendFor()) {
            return true;
        }
    }

    if (note->tieBack()) {
        const Note* firstTiedNote = note->firstTiedNote(IGNORE_UNPLAYABLE);
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
    if (note->isPreBendOrDiveStart()) {
        return;
    }

    if (const GuitarBend* bendBack = note->bendBack()) {
        static const std::unordered_set<GuitarBendType> SHOULD_RENDER_NOTE_WITH_BEND_BACK {
            GuitarBendType::PRE_BEND,
            GuitarBendType::PRE_DIVE,
            GuitarBendType::SCOOP,
        };

        if (!muse::contains(SHOULD_RENDER_NOTE_WITH_BEND_BACK, bendBack->bendType())) {
            return;
        }
    }

    if (!NoteRenderer::shouldRender(note, ctx, ctx.commonArticulations)) {
        return;
    }

    renderMultibend(note, ctx, result);
}

void BendsRenderer::renderMultibend(const Note* startNote, const RenderingContext& startNoteCtx,
                                    mpe::PlaybackEventList& result)
{
    const Note* currNote = startNote;
    const GuitarBend* currBend = nullptr;

    mpe::PlaybackEventList bendEvents;
    BendTimeFactorMap bendTimeFactorMap;

    auto nextNote = [&currNote, &currBend]() {
        return currBend && currBend->endNote() != currNote ? currBend->endNote() : nullptr;
    };

    while (currNote) {
        RenderingContext currNoteCtx = buildRenderingContext(currNote, startNoteCtx);
        renderNote(currNote, currNoteCtx, bendEvents);
        if (bendEvents.empty()) {
            break;
        }

        const GuitarBend* diveBack = currNote->diveBack();
        if (diveBack && diveBack->bendType() == GuitarBendType::SCOOP) {
            currBend = diveBack;
        } else if (currNote->tieFor()) {
            currBend = currNote->lastTiedNote(IGNORE_UNPLAYABLE)->bendFor();
        } else {
            currBend = currNote->bendFor();
        }

        if (!currBend) {
            break;
        }

        if (currBend->bendType() == GuitarBendType::SLIGHT_BEND) {
            renderSlightBend(currNote, currBend, currNoteCtx, bendEvents);
            break;
        }

        if (currBend->bendType() == GuitarBendType::DIP) {
            renderDip(currNote, currBend, currNoteCtx, bendEvents);
            break;
        }

        if (currBend->bendType() == GuitarBendType::SCOOP) {
            renderScoop(currNote, currBend, currNoteCtx, bendEvents);
            currBend = currNote->bendFor();
            currNote = nextNote();
            continue;
        }

        const mpe::PlaybackEvent& newEvent = bendEvents.back();
        if (std::holds_alternative<mpe::NoteEvent>(newEvent)) {
            const mpe::ArrangementContext& arrangementCtx = std::get<mpe::NoteEvent>(newEvent).arrangementCtx();
            const mpe::timestamp_t timestampTo = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration;
            bendTimeFactorMap.insert_or_assign(timestampTo, timeFactors(currBend));
        }

        currNote = nextNote();
    }

    if (!bendEvents.empty()) {
        mpe::NoteEvent event = buildBendEvent(startNote, startNoteCtx, bendEvents, bendTimeFactorMap);
        result.emplace_back(std::move(event));
    }
}

void BendsRenderer::renderNote(const Note* note, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    for (const auto& pair : ctx.commonArticulations) {
        if (!muse::contains(GRACE_NOTE_ARTICULATION_TYPES, pair.first)) {
            continue;
        }

        // This note is either a grace note or a principal note for other grace notes
        const GraceChordCtx graceCtx = GraceChordCtx::buildCtx(principalChord(note->chord()), pair.first, ctx);

        if (note->isGrace()) {
            renderGraceNote(note, graceCtx, result);
        } else {
            NoteRenderer::render(note, graceCtx.principalChordCtx, result);
        }

        return;
    }

    NoteRenderer::render(note, ctx, result);
}

void BendsRenderer::renderGraceNote(const Note* note, const GraceChordCtx& ctx, mpe::PlaybackEventList& result)
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

void BendsRenderer::renderSlightBend(const Note* note, const GuitarBend* bend, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    NominalNoteCtx slightNoteCtx(note, ctx);
    slightNoteCtx.duration = 0; // aux notes have no duration

    const BendTimeFactors factors = timeFactors(bend);

    // Hold the origin pitch
    if (!RealIsNull(factors.startFactor)) {
        slightNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.startFactor;
        result.emplace_back(buildNoteEvent(slightNoteCtx));
    }

    // Go up
    slightNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.endFactor;
    slightNoteCtx.pitchLevel += pitchOffset(bend);
    result.emplace_back(buildNoteEvent(slightNoteCtx));
}

void BendsRenderer::renderDip(const Note* note, const GuitarBend* bend, const RenderingContext& ctx, muse::mpe::PlaybackEventList& result)
{
    NominalNoteCtx dipNoteCtx(note, ctx);
    dipNoteCtx.duration = 0; // aux notes have no duration

    const mpe::pitch_level_t originPitchLevel = dipNoteCtx.pitchLevel;
    const BendTimeFactors factors = timeFactors(bend);

    // Hold the origin pitch
    if (!RealIsNull(factors.startFactor)) {
        dipNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.startFactor;
        result.emplace_back(buildNoteEvent(dipNoteCtx));
    }

    // Go down / up
    dipNoteCtx.pitchLevel += pitchOffset(bend);
    dipNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.endFactor;
    result.emplace_back(buildNoteEvent(dipNoteCtx));

    // Go back to the origin pitch
    dipNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration;
    dipNoteCtx.pitchLevel = originPitchLevel;
    result.emplace_back(buildNoteEvent(dipNoteCtx));
}

void BendsRenderer::renderScoop(const Note* note, const GuitarBend* bend, const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    NominalNoteCtx scoopNoteCtx(note, ctx);
    scoopNoteCtx.duration = 0; // aux notes have no duration

    const mpe::pitch_level_t originPitchLevel = scoopNoteCtx.pitchLevel;
    const BendTimeFactors factors = timeFactors(bend);

    // Start with a pitch offset
    scoopNoteCtx.pitchLevel += pitchOffset(bend);
    result.emplace_back(buildNoteEvent(scoopNoteCtx));

    // Hold it
    if (!RealIsNull(factors.startFactor)) {
        scoopNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.startFactor;
        result.emplace_back(buildNoteEvent(scoopNoteCtx));
    }

    // Go back to the origin pitch
    scoopNoteCtx.timestamp = ctx.nominalTimestamp + ctx.nominalDuration * factors.endFactor;
    scoopNoteCtx.pitchLevel = originPitchLevel;
    result.emplace_back(buildNoteEvent(scoopNoteCtx));
}

BendsRenderer::BendTimeFactors BendsRenderer::timeFactors(const GuitarBend* bend)
{
    const float startFactor = std::clamp(bend->startTimeFactor(), 0.f, 1.f);
    const float endFactor = std::clamp(bend->endTimeFactor(), 0.f, 1.f);

    IF_ASSERT_FAILED(RealIsEqualOrLess(startFactor, endFactor)) {
        return BendTimeFactors { 0.f, 1.f };
    }

    return BendTimeFactors { startFactor, endFactor };
}

RenderingContext BendsRenderer::buildRenderingContext(const Note* note, const RenderingContext& initialCtx)
{
    // Use the principal chord to build the context
    const Chord* chord = principalChord(note->chord());

    RenderingContext ctx = engraving::buildRenderingCtx(chord, initialCtx.positionTickOffset,
                                                        initialCtx.profile, initialCtx.playbackCtx,
                                                        initialCtx.commonArticulations);

    ChordArticulationsParser::buildChordArticulationMap(chord, ctx, ctx.commonArticulations);

    return ctx;
}

mpe::NoteEvent BendsRenderer::buildBendEvent(const Note* startNote, const RenderingContext& startNoteCtx,
                                             const mpe::PlaybackEventList& bendNoteEvents, const BendTimeFactorMap& timeFactorMap)
{
    NominalNoteCtx noteCtx(startNote, startNoteCtx);

    const mpe::NoteEvent& startNoteEvent = std::get<mpe::NoteEvent>(bendNoteEvents.front());
    noteCtx.articulations = startNoteEvent.expressionCtx().articulations;
    noteCtx.timestamp = startNoteEvent.arrangementCtx().actualTimestamp;

    PitchOffsets pitchOffsets;
    pitchOffsets.reserve(bendNoteEvents.size());

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

        if (arrangementCtx.actualDuration != 0) {
            noteCtx.duration = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration - noteCtx.timestamp;
        }

        const mpe::pitch_level_t offset = noteEvent.pitchCtx().nominalPitchLevel - noteCtx.pitchLevel;
        pitchOffsets.emplace_back(arrangementCtx.actualTimestamp, offset);
    }

    if (multibendIt != noteCtx.articulations.end()) {
        mpe::ArticulationMeta& meta = multibendIt->second.meta;
        meta.timestamp = noteCtx.timestamp;
        meta.overallDuration = noteCtx.duration;
    }

    mpe::PitchCurve curve = buildPitchCurve(noteCtx.timestamp, noteCtx.duration, pitchOffsets, timeFactorMap);
    mpe::NoteEvent result = buildNoteEvent(noteCtx, curve);

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
