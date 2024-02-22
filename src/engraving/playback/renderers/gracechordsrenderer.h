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

#ifndef MU_ENGRAVING_GRACENOTESRENDERER_H
#define MU_ENGRAVING_GRACENOTESRENDERER_H

#include <vector>

#include "renderbase.h"

namespace mu::engraving {
class Chord;
class Note;

class GraceChordsRenderer : public RenderBase<GraceChordsRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& ctx,
                         mpe::PlaybackEventList& result);

    static void renderGraceNote(const Note* graceNote, const Note* principalNote, const mpe::ArticulationType graceNoteType,
                                const RenderingContext& ctx, mpe::PlaybackEventList& result);

private:
    struct GraceNotesContext {
        mpe::ArticulationType type = mpe::ArticulationType::Undefined;
        double durationFactor = 0;
        mpe::timestamp_t graceNotesTimestampFrom = 0;
        mpe::timestamp_t principalNotesTimestampFrom = 0;
        mpe::duration_t totalPrincipalNotesDuration = 0;
    };

    using GraceNoteAccepted = std::function<bool (const Note*, const RenderingContext&)>;

    static bool isPlacedBeforePrincipalNote(const mpe::ArticulationType type);

    static void renderGraceNoteEvents(const std::vector<Chord*>& graceChords, GraceNoteAccepted graceNoteAccepted,
                                      const RenderingContext& ctx, const GraceNotesContext& graceCtx, mpe::PlaybackEventList& result);

    static void renderPrincipalChord(const Chord* chord, const RenderingContext& ctx, const GraceNotesContext& graceCtx,
                                     mpe::PlaybackEventList& result);

    static void renderPrincipalNote(const Note* note, const RenderingContext& ctx, const GraceNotesContext& graceCtx,
                                    mpe::PlaybackEventList& result);

    static GraceNotesContext buildGraceNotesContext(const std::vector<Chord*>& graceChords, const RenderingContext& ctx,
                                                    const mpe::ArticulationType type);

    static RenderingContext buildPrincipalNoteCtx(const Score* score, const RenderingContext& ctx, const GraceNotesContext& graceCtx);

    static mpe::duration_t graceNotesTotalDuration(const std::vector<Chord*>& graceChords, const RenderingContext& context);

    static mpe::duration_t graceNotesMaxAvailableDuration(const mpe::ArticulationType type, const RenderingContext& ctx,
                                                          const size_t graceNotesCount);
    static mpe::timestamp_t graceNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t availableDuration,
                                                     const mpe::timestamp_t& nominalTimestamp);

    static mpe::timestamp_t principalNotesStartTimestamp(const mpe::ArticulationType type, const mpe::duration_t graceNotesDuration,
                                                         const mpe::timestamp_t& nominalTimestamp);

    static mpe::timestamp_t principalNotesDuration(const mpe::duration_t graceNotesDuration, const mpe::duration_t& nominalDuration);
};
}

#endif // MU_ENGRAVING_GRACENOTESRENDERER_H
