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

#include "tremolorenderer.h"

#include "dom/chord.h"
#include "dom/tremolo.h"

#include "playback/metaparsers/notearticulationsparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& TremoloRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Tremolo8th, mpe::ArticulationType::Tremolo16th,
        mpe::ArticulationType::Tremolo32nd, mpe::ArticulationType::Tremolo64th
    };

    return types;
}

void TremoloRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType preferredType,
                               const RenderingContext& context,
                               mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const Tremolo* tremolo = chord->tremolo();
    IF_ASSERT_FAILED(tremolo) {
        return;
    }

    const ArticulationAppliedData& articulationData = context.commonArticulations.at(preferredType);

    duration_t stepDuration = durationFromTicks(context.beatsPerSecond.val, stepDurationTicks(chord, tremolo));

    if (stepDuration <= 0) {
        LOGE() << "Unable to render unsupported tremolo type";
        return;
    }

    int stepsCount = articulationData.meta.overallDuration / stepDuration;

    if (tremolo->twoNotes()) {
        const Chord* firstTremoloChord = tremolo->chord1();
        const Chord* secondTremoloChord = tremolo->chord2();

        IF_ASSERT_FAILED(firstTremoloChord && secondTremoloChord) {
            return;
        }

        for (int i = 0; i < stepsCount; ++i) {
            const Chord* currentChord = firstTremoloChord;

            if (i % 2 != 0) {
                currentChord = secondTremoloChord;
            }

            buildAndAppendEvents(currentChord, preferredType, stepDuration, i * stepDuration, context, result);
        }

        return;
    }

    for (int i = 0; i < stepsCount; ++i) {
        buildAndAppendEvents(chord, preferredType, stepDuration, i * stepDuration, context, result);
    }
}

int TremoloRenderer::stepDurationTicks(const Chord* chord, const Tremolo* tremolo)
{
    int ticks = Constants::DIVISION / (1 << (chord->beams() + tremolo->lines()));
    if (ticks <= 0) {
        return 1;
    }
    return ticks * chord->timeStretchFactor();
}

void TremoloRenderer::buildAndAppendEvents(const Chord* chord, const ArticulationType type,
                                           const mpe::duration_t stepDuration,
                                           const mpe::timestamp_t timestampOffset, const RenderingContext& context,
                                           mpe::PlaybackEventList& result)
{
    for (size_t noteIdx = 0; noteIdx < chord->notes().size(); ++noteIdx) {
        const Note* note = chord->notes().at(noteIdx);

        if (!isNotePlayable(note, context.commonArticulations)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        noteCtx.duration = stepDuration;
        noteCtx.timestamp += timestampOffset;

        NoteArticulationsParser::buildNoteArticulationMap(note, context, noteCtx.chordCtx.commonArticulations);
        updateArticulationBoundaries(type, noteCtx.timestamp, noteCtx.duration, noteCtx.chordCtx.commonArticulations);

        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}
