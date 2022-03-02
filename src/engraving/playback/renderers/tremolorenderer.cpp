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

#include "libmscore/chord.h"
#include "libmscore/tremolo.h"

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

void TremoloRenderer::doRender(const Ms::EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                               mpe::PlaybackEventList& result)
{
    const Ms::Chord* chord = Ms::toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const Ms::Tremolo* tremolo = chord->tremolo();
    IF_ASSERT_FAILED(tremolo) {
        return;
    }

    const ArticulationAppliedData& articulationData = context.commonArticulations.at(preferredType);

    duration_t stepDuration = durationFromTicks(context.beatsPerSecond.val, stepDurationTicksByType(preferredType));

    if (stepDuration <= 0) {
        LOGE() << "Unable to render unsupported tremolo type";
        return;
    }

    int stepsCount = articulationData.meta.overallDuration / stepDuration;

    if (tremolo->twoNotes()) {
        const Ms::Chord* firstTremoloChord = tremolo->chord1();
        const Ms::Chord* secondTremoloChord = tremolo->chord2();

        IF_ASSERT_FAILED(firstTremoloChord && secondTremoloChord) {
            return;
        }

        for (int i = 0; i < stepsCount; ++i) {
            const Ms::Chord* currentChord = firstTremoloChord;

            if (i % 2 == 0) {
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

int TremoloRenderer::stepDurationTicksByType(const mpe::ArticulationType& type)
{
    static constexpr int QUAVER_NOTE_DURATION_TICKS = Constants::division / 2;
    static constexpr int SEMI_QUAVER_NOTE_DURATION_TICKS = QUAVER_NOTE_DURATION_TICKS / 2;
    static constexpr int DEMI_SEMI_QUAVER_NOTE_DURATION_TICKS = SEMI_QUAVER_NOTE_DURATION_TICKS / 2;
    static constexpr int HEMI_SEMI_DEMI_QUAVER_NOTE_DURATION_TICKS = DEMI_SEMI_QUAVER_NOTE_DURATION_TICKS / 2;

    switch (type) {
    case ArticulationType::Tremolo8th: return QUAVER_NOTE_DURATION_TICKS;
    case ArticulationType::Tremolo16th: return SEMI_QUAVER_NOTE_DURATION_TICKS;
    case ArticulationType::Tremolo32nd: return DEMI_SEMI_QUAVER_NOTE_DURATION_TICKS;
    case ArticulationType::Tremolo64th: return HEMI_SEMI_DEMI_QUAVER_NOTE_DURATION_TICKS;
    default: return 0;
    }
}

void TremoloRenderer::buildAndAppendEvents(const Ms::Chord* chord, const ArticulationType type, const mpe::duration_t stepDuration,
                                           const mpe::timestamp_t timestampOffset, const RenderingContext& context,
                                           mpe::PlaybackEventList& result)
{
    for (size_t noteIdx = 0; noteIdx < chord->notes().size(); ++noteIdx) {
        const Ms::Note* note = chord->notes().at(noteIdx);

        if (!isNotePlayable(note)) {
            continue;
        }

        NominalNoteCtx noteCtx(note, context);
        noteCtx.duration = stepDuration;
        noteCtx.timestamp += timestampOffset;

        updateArticulationBoundaries(type, noteCtx.timestamp, noteCtx.duration, noteCtx.chordCtx.commonArticulations);
        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    }
}
