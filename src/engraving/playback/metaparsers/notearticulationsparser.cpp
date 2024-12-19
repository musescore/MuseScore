/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/note.h"
#include "dom/spanner.h"

#include "playback/utils/arrangementutils.h"
#include "internal/spannersmetaparser.h"
#include "internal/symbolsmetaparser.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void NoteArticulationsParser::buildNoteArticulationMap(const Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    if (!note || !ctx.isValid()) {
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

void NoteArticulationsParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::NOTE) {
        return;
    }

    const Note* note = toNote(item);

    if (!note || !note->play()) {
        return;
    }

    parsePersistentMeta(ctx, result);
    parseGhostNote(note, ctx, result);
    parseNoteHead(note, ctx, result);
    parseSpanners(note, ctx, result);
}

ArticulationType NoteArticulationsParser::articulationTypeByNoteheadGroup(const NoteHeadGroup noteheadGroup)
{
    switch (noteheadGroup) {
    case NoteHeadGroup::HEAD_CROSS:
        return mpe::ArticulationType::CrossNote;

    case NoteHeadGroup::HEAD_CIRCLED_LARGE:
    case NoteHeadGroup::HEAD_CIRCLED:
        return mpe::ArticulationType::CircleNote;

    case NoteHeadGroup::HEAD_XCIRCLE:
        return mpe::ArticulationType::CircleCrossNote;

    case NoteHeadGroup::HEAD_TRIANGLE_DOWN:
        return mpe::ArticulationType::TriangleDownNote;
    case NoteHeadGroup::HEAD_TRIANGLE_UP:
        return mpe::ArticulationType::TriangleUpNote;

    case NoteHeadGroup::HEAD_DIAMOND:
    case NoteHeadGroup::HEAD_DIAMOND_OLD:
        return mpe::ArticulationType::DiamondNote;

    case NoteHeadGroup::HEAD_PLUS:
        return mpe::ArticulationType::PlusNote;

    case NoteHeadGroup::HEAD_SLASH:
        return mpe::ArticulationType::SlashNote;

    case NoteHeadGroup::HEAD_SLASHED1:
        return mpe::ArticulationType::SlashedForwardsNote;

    case NoteHeadGroup::HEAD_SLASHED2:
        return mpe::ArticulationType::SlashedBackwardsNote;

    case NoteHeadGroup::HEAD_DO:
        return mpe::ArticulationType::TriangleUpNote;

    case NoteHeadGroup::HEAD_RE:
        return mpe::ArticulationType::MoonNote;

    case NoteHeadGroup::HEAD_FA:
        return mpe::ArticulationType::TriangleRightNote;

    case NoteHeadGroup::HEAD_LA:
        return mpe::ArticulationType::SquareNote;

    case NoteHeadGroup::HEAD_TI:
        return mpe::ArticulationType::TriangleRoundDownNote;

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

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(ctx.persistentArticulation);
    if (pattern.empty()) {
        return;
    }

    appendArticulationData({ ctx.persistentArticulation,
                             pattern,
                             ctx.nominalTimestamp,
                             ctx.nominalDuration,
                             0,
                             0 }, result);
}

void NoteArticulationsParser::parseGhostNote(const Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    if (!note->ghost() && !note->headHasParentheses()) {
        return;
    }

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(mpe::ArticulationType::GhostNote);
    if (pattern.empty()) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(mpe::ArticulationType::GhostNote,
                                                 pattern,
                                                 ctx.nominalTimestamp,
                                                 ctx.nominalDuration), result);
}

void NoteArticulationsParser::parseNoteHead(const Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    ArticulationTypeSet types;
    mpe::ArticulationType typeByNoteHeadGroup = articulationTypeByNoteheadGroup(note->headGroup());

    if (typeByNoteHeadGroup != mpe::ArticulationType::Undefined) {
        types.insert(typeByNoteHeadGroup);
    } else if (note->ldata()->cachedNoteheadSym.has_value()) {
        SymId symId = note->ldata()->cachedNoteheadSym.value(); // fastest way to get the notehead symbol
        types = SymbolsMetaParser::symbolToArticulations(symId);
    }

    for (mpe::ArticulationType type : types) {
        if (type == mpe::ArticulationType::Undefined) {
            continue;
        }

        const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
        if (pattern.empty()) {
            continue;
        }

        appendArticulationData(mpe::ArticulationMeta(type,
                                                     pattern,
                                                     ctx.nominalTimestamp,
                                                     ctx.nominalDuration), result);
    }
}

void NoteArticulationsParser::parseSpanners(const Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    for (const Spanner* spanner : note->spannerFor()) {
        int spannerFrom = spanner->tick().ticks();
        int spannerTo = spannerFrom + std::abs(spanner->ticks().ticks());
        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks == 0) {
            spannerDurationTicks = ctx.nominalDurationTicks;
        }

        auto spannerTnD = timestampAndDurationFromStartAndDurationTicks(ctx.score, spannerFrom, spannerDurationTicks, 0);

        RenderingContext spannerContext = ctx;
        spannerContext.nominalTimestamp = spannerTnD.timestamp;
        spannerContext.nominalDuration = spannerTnD.duration;
        spannerContext.nominalPositionStartTick = spannerFrom;
        spannerContext.nominalDurationTicks = spannerDurationTicks;

        SpannersMetaParser::parse(spanner, spannerContext, result);
    }
}
