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

#include "notearticulationsparser.h"

#include "libmscore/note.h"
#include "libmscore/spanner.h"

#include "playback/utils/arrangementutils.h"
#include "internal/spannersmetaparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

void NoteArticulationsParser::buildNoteArticulationMap(const Ms::Note* note, const RenderingContext& ctx,
                                                       mpe::ArticulationMap& result)
{
    if (!note) {
        LOGE() << "Unable to render playback events of invalid note";
        return;
    }

    parse(note, ctx, result);

    if (result.empty()) {
        appendArticulationData({ mpe::ArticulationType::Standard,
                                 ctx.profile->pattern(mpe::ArticulationType::Standard),
                                 ctx.nominalTimestamp,
                                 ctx.nominalDuration,
                                 0,
                                 0 }, result);
    }

    result.preCalculateAverageData();
}

void NoteArticulationsParser::doParse(const Ms::EngravingItem* item, const RenderingContext& ctx,
                                      mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == Ms::ElementType::NOTE && ctx.isValid()) {
        return;
    }

    const Ms::Note* note = Ms::toNote(item);

    if (!note || !note->play()) {
        return;
    }

    parsePersistentMeta(ctx, result);
    parseGhostNote(note, ctx, result);
    parseNoteHead(note, ctx, result);
    parseSpanners(note, ctx, result);
}

ArticulationType NoteArticulationsParser::articulationTypeByNotehead(const NoteHeadGroup noteheadGroup)
{
    switch (noteheadGroup) {
    case NoteHeadGroup::HEAD_CROSS:
        return mpe::ArticulationType::CrossNote;

    case NoteHeadGroup::HEAD_CIRCLED_LARGE:
    case NoteHeadGroup::HEAD_CIRCLED:
        return mpe::ArticulationType::CircleNote;

    case NoteHeadGroup::HEAD_TRIANGLE_DOWN:
    case NoteHeadGroup::HEAD_TRIANGLE_UP:
        return mpe::ArticulationType::TriangleNote;

    case NoteHeadGroup::HEAD_DIAMOND:
    case NoteHeadGroup::HEAD_DIAMOND_OLD:
        return mpe::ArticulationType::DiamondNote;

    default:
        return mpe::ArticulationType::Undefined;
    }
}

void NoteArticulationsParser::parsePersistentMeta(const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    if (ctx.persistentArticulation == ArticulationType::Undefined
        || ctx.persistentArticulation == ArticulationType::Standard) {
        return;
    }

    appendArticulationData({ ctx.persistentArticulation,
                             ctx.profile->pattern(ctx.persistentArticulation),
                             ctx.nominalTimestamp,
                             ctx.nominalDuration,
                             0,
                             0 }, result);
}

void NoteArticulationsParser::parseGhostNote(const Ms::Note* note, const RenderingContext& ctx,
                                             mpe::ArticulationMap& result)
{
    if (!note->ghost()) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(mpe::ArticulationType::GhostNote,
                                                 ctx.profile->pattern(mpe::ArticulationType::GhostNote),
                                                 ctx.nominalTimestamp,
                                                 ctx.nominalDuration), result);
}

void NoteArticulationsParser::parseNoteHead(const Ms::Note* note, const RenderingContext& ctx,
                                            mpe::ArticulationMap& result)
{
    mpe::ArticulationType typeByNoteHead = articulationTypeByNotehead(note->headGroup());

    if (typeByNoteHead == mpe::ArticulationType::Undefined) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(typeByNoteHead,
                                                 ctx.profile->pattern(typeByNoteHead),
                                                 ctx.nominalTimestamp,
                                                 ctx.nominalDuration), result);
}

void NoteArticulationsParser::parseSpanners(const Ms::Note* note, const RenderingContext& ctx,
                                            mpe::ArticulationMap& result)
{
    for (const Ms::Spanner* spanner : note->spannerFor()) {
        int spannerFrom = spanner->tick().ticks();
        int spannerTo = spanner->tick().ticks() + std::abs(spanner->ticks().ticks());
        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks == 0) {
            continue;
        }

        RenderingContext spannerContext = ctx;
        spannerContext.nominalTimestamp = timestampFromTicks(note->score(), spannerFrom);
        spannerContext.nominalDuration = durationFromTicks(ctx.beatsPerSecond.val, spannerDurationTicks);
        spannerContext.nominalPositionStartTick = spannerFrom;
        spannerContext.nominalDurationTicks = spannerDurationTicks;

        SpannersMetaParser::parse(spanner, spannerContext, result);
    }
}
