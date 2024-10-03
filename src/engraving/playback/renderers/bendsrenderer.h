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
#pragma once

#include "mpe/events.h"

namespace mu::engraving {
class Note;
class GuitarBend;
class Score;
struct RenderingContext;
struct GraceChordCtx;
class BendsRenderer
{
public:
    static bool isMultibendPart(const Note* note);

    static void render(const Note* note, const RenderingContext& ctx, muse::mpe::PlaybackEventList& result);

private:
    struct BendTimeFactors {
        float startFactor = 0.f;
        float endFactor = 1.f;
    };

    using PitchOffsets = std::vector<std::pair<muse::mpe::timestamp_t, muse::mpe::pitch_level_t> >;
    using BendTimeFactorMap = std::map<muse::mpe::timestamp_t, BendTimeFactors>;

    static void renderMultibend(const Note* startNote, const RenderingContext& startNoteCtx, muse::mpe::PlaybackEventList& result);
    static void renderGraceAndPrincipalNotes(const Note* graceNote, const Note* principalNote, const RenderingContext& ctx,
                                             muse::mpe::PlaybackEventList& result);
    static void renderGraceNote(const Note* note, const GraceChordCtx& ctx, muse::mpe::PlaybackEventList& result);

    static void appendBendTimeFactors(const Score* score, const GuitarBend* bend, BendTimeFactorMap& timeFactorMap);

    static RenderingContext buildRenderingContext(const Note* note, const RenderingContext& initialCtx);
    static muse::mpe::NoteEvent buildSlightNoteEvent(const Note* note, const RenderingContext& ctx);
    static muse::mpe::NoteEvent buildBendEvent(const Note* startNote, const RenderingContext& startNoteCtx,
                                               const muse::mpe::PlaybackEventList& bendNoteEvents, const BendTimeFactorMap& timeFactorMap);
    static muse::mpe::PitchCurve buildPitchCurve(muse::mpe::timestamp_t bendStartTime, muse::mpe::duration_t totalBendDuration,
                                                 const PitchOffsets& pitchOffsets, const BendTimeFactorMap& timeFactorMap);
};
}
