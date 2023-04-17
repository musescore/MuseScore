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

class GraceChordsRenderer : public RenderBase<GraceChordsRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& context,
                         mpe::PlaybackEventList& result);
private:
    static bool isPlacedBeforePrincipalNote(const mpe::ArticulationType type);

    static void renderPrependedGraceNotes(const Chord* chord, const RenderingContext& context, const mpe::ArticulationType type,
                                          mpe::PlaybackEventList& result);
    static void renderAppendedGraceNotes(const Chord* chord, const RenderingContext& context, const mpe::ArticulationType type,
                                         mpe::PlaybackEventList& result);

    static mpe::duration_t graceNotesTotalDuration(const std::vector<Chord*>& graceChords, const RenderingContext& context);
    static float graceNotesDurationRatio(const mpe::duration_t totalDuration, const mpe::duration_t maxAvailableDuration);

    static void buildGraceNoteEvents(const std::vector<Chord*>& graceChords, const RenderingContext& context,
                                     const mpe::ArticulationType type, const mpe::timestamp_t timestampFrom,
                                     const mpe::duration_t availableDuration, mpe::PlaybackEventList& result);

    static void buildPrincipalNoteEvents(const Chord* chord, const RenderingContext& ctx, const mpe::ArticulationType type,
                                         const mpe::timestamp_t timestamp, const mpe::duration_t duration, mpe::PlaybackEventList& result);

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
