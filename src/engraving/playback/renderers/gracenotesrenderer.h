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

#include <QVector>

#include "renderbase.h"

namespace Ms {
class Chord;
}

namespace mu::engraving {
class GraceNotesRenderer : public RenderBase<GraceNotesRenderer>
{
public:
    static const mpe::ArticulationTypeSet& supportedTypes();

    static void doRender(const Ms::EngravingItem* item, const mpe::ArticulationType type, const RenderingContext& context,
                         mpe::PlaybackEventList& result);
private:
    static bool isPlacedBeforePrincipalNote(const mpe::ArticulationType type);

    static void renderPrependedGraceNotes(const Ms::Chord* chord, const RenderingContext& context, mpe::PlaybackEventList& result);
    static void renderAppendedGraceNotes(const Ms::Chord* chord, const RenderingContext& context, mpe::PlaybackEventList& result);

    static mpe::duration_t graceNotesTotalDuration(const std::vector<NominalNoteCtx>& noteCtxList);
    static float graceNotesDurationRatio(const mpe::duration_t totalDuration, const mpe::duration_t maxAvailableDuration);
    static std::vector<NominalNoteCtx> graceNotesCtxList(const QVector<Ms::Chord*>& graceChords, const RenderingContext& context);

    static void buildGraceNoteEvents(std::vector<NominalNoteCtx>&& noteCtxList, const mpe::timestamp_t timestampFrom,
                                     const float durationRatio, mpe::PlaybackEventList& result);

    static void buildPrincipalNoteEvents(const Ms::Chord* chord, const RenderingContext& context, const mpe::duration_t duration,
                                         const mpe::timestamp_t timestamp, mpe::PlaybackEventList& result);
};
}

#endif // MU_ENGRAVING_GRACENOTESRENDERER_H
