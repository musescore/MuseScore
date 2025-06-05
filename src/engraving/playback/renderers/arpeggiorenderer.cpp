/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "dom/chord.h"
#include "dom/arpeggio.h"

#include "noterenderer.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static bool isDirectionUp(const mpe::ArticulationType type)
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

static usecs_t timestampOffsetStep(const RenderingContext& ctx, int stepCount)
{
    constexpr int MAX_TIMESTAMP_OFFSET_STEP = 60000;
    const int offsetStep = ctx.nominalDuration / stepCount;

    return std::min(offsetStep, MAX_TIMESTAMP_OFFSET_STEP);
}

static std::map<int /*pitch*/, const Note*> arpeggioNotes(const Chord* chord)
{
    std::map<int, const Note*> result;
    for (const Note* note : chord->notes()) {
        result.emplace(note->pitch(), note);
    }

    return result;
}

const ArticulationTypeSet& ArpeggioRenderer::supportedTypes()
{
    static const mpe::ArticulationTypeSet types = {
        mpe::ArticulationType::Arpeggio, mpe::ArticulationType::ArpeggioDown,
        mpe::ArticulationType::ArpeggioStraightDown, mpe::ArticulationType::ArpeggioStraightUp,
        mpe::ArticulationType::ArpeggioUp
    };

    return types;
}

void ArpeggioRenderer::doRender(const EngravingItem* item, const mpe::ArticulationType preferredType,
                                const RenderingContext& ctx, mpe::PlaybackEventList& result)
{
    const Chord* chord = toChord(item);
    IF_ASSERT_FAILED(chord) {
        return;
    }

    const Arpeggio* arpeggio = chord->arpeggio();
    IF_ASSERT_FAILED(arpeggio) {
        return;
    }

    const int stepsCount = static_cast<int>(chord->notes().size());
    const usecs_t offsetStep = timestampOffsetStep(ctx, stepsCount);
    const double stretch = arpeggio->Stretch();

    auto renderEvent = [&](const Note* note, const int stepNumber) {
        const timestamp_t offset = offsetStep * stepNumber * stretch;

        RenderingContext noteCtx(ctx);
        noteCtx.nominalTimestamp += offset;
        noteCtx.nominalDuration -= offset;

        NoteRenderer::render(note, noteCtx, result);
    };

    const std::map<int, const Note*> notes = arpeggioNotes(chord);

    if (isDirectionUp(preferredType)) {
        for (auto it = notes.begin(); it != notes.end(); ++it) {
            renderEvent(it->second, std::distance(notes.begin(), it));
        }
    } else {
        for (auto it = notes.rbegin(); it != notes.rend(); ++it) {
            renderEvent(it->second, std::distance(notes.rbegin(), it));
        }
    }
}
