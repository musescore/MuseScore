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

#include "arpeggiorenderer.h"

#include "libmscore/chord.h"

using namespace mu::engraving;
using namespace mu::mpe;

const ArticulationTypeSet& ArpeggioRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Arpeggio, mpe::ArticulationType::ArpeggioDown,
        mpe::ArticulationType::ArpeggioStraightDown, mpe::ArticulationType::ArpeggioStraightUp,
        mpe::ArticulationType::ArpeggioUp
    };

    return types;
}

void ArpeggioRenderer::doRender(const Ms::EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                                mpe::PlaybackEventList& result)
{
    const Ms::Chord* chord = Ms::toChord(item);

    IF_ASSERT_FAILED(chord) {
        return;
    }

    int stepsCount = static_cast<int>(chord->notes().size());
    mpe::percentage_t percentageStep = mpe::HUNDRED_PERCENT / stepsCount;

    auto buildEvent = [&](const Ms::Note* note, const int stepNumber) {
        if (!isNotePlayable(note)) {
            return;
        }

        NominalNoteCtx noteCtx(note, context);
        noteCtx.chordCtx.commonArticulations.updateOccupiedRange(preferredType, stepNumber * percentageStep,
                                                                 (stepNumber + 1) * percentageStep);
        noteCtx.timestamp += timestampOffsetStep(context) * stepNumber;
        result.emplace_back(buildNoteEvent(std::move(noteCtx)));
    };

    for (int i = 0; i < stepsCount; ++i) {
        const Ms::Note* note = chord->notes().at(i);

        if (!isDirectionUp(preferredType)) {
            note = chord->notes().at(stepsCount - 1 - i);
        }

        buildEvent(note, i);
    }
}

bool ArpeggioRenderer::isDirectionUp(const mpe::ArticulationType type)
{
    switch (type) {
    case mpe::ArticulationType::ArpeggioDown:
    case mpe::ArticulationType::ArpeggioStraightDown:
        return false;
    case mpe::ArticulationType::Arpeggio:
    case mpe::ArticulationType::ArpeggioUp:
    case mpe::ArticulationType::ArpeggioStraightUp:
        return true;
    default:
        return false;
    }
}

msecs_t ArpeggioRenderer::timestampOffsetStep(const RenderingContext& ctx)
{
    constexpr int MINIMAL_TIMESTAMP_OFFSET_STEP = 60;

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, PRESTISSIMO_BPS_BOUND)) {
        return MINIMAL_TIMESTAMP_OFFSET_STEP * 1.5;
    }

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, PRESTO_BPS_BOUND)) {
        return MINIMAL_TIMESTAMP_OFFSET_STEP * 1.25;
    }

    if (RealIsEqualOrMore(ctx.beatsPerSecond.val, MODERATO_BPS_BOUND)) {
        return MINIMAL_TIMESTAMP_OFFSET_STEP;
    }

    return MINIMAL_TIMESTAMP_OFFSET_STEP;
}
