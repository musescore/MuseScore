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

void NoteArticulationsParser::buildNoteArticulationMap(const Ms::Note* note, const PlaybackContext& ctx,
                                                       mpe::ArticulationMap& result) const
{
    if (!note) {
        LOGE() << "Unable to render playback events of invalid note";
        return;
    }

    mpe::ArticulationMetaMap perNoteArticulationsMetaMap;
    parse(note, ctx, perNoteArticulationsMetaMap);

    if (result.isEmpty() && perNoteArticulationsMetaMap.empty()) {
        mpe::ArticulationMeta standardMeta(mpe::ArticulationType::Standard,
                                           ctx.profile->pattern(mpe::ArticulationType::Standard),
                                           ctx.nominalTimestamp,
                                           ctx.nominalDuration);

        perNoteArticulationsMetaMap.insert_or_assign(standardMeta.type, std::move(standardMeta));
    }

    for (auto& pair : perNoteArticulationsMetaMap) {
        result.emplace(pair.first, mpe::ArticulationAppliedData(std::move(pair.second), 0, mpe::HUNDRED_PERCENTS));
    }
}

void NoteArticulationsParser::doParse(const Ms::EngravingItem* item, const PlaybackContext& ctx,
                                      mpe::ArticulationMetaMap& result) const
{
    IF_ASSERT_FAILED(item->type() == Ms::ElementType::NOTE && ctx.isValid()) {
        return;
    }

    const Ms::Note* note = Ms::toNote(item);

    if (!note || !note->play()) {
        return;
    }

    parseGhostNote(note, ctx, result);
    parseNoteHead(note, ctx, result);
    parseSpanners(note, ctx, result);
}

ArticulationType NoteArticulationsParser::articulationTypeByNotehead(const NoteHeadGroup noteheadGroup) const
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

void NoteArticulationsParser::parseGhostNote(const Ms::Note* note, const PlaybackContext& ctx,
                                             mpe::ArticulationMetaMap& result) const
{
    if (!note->ghost()) {
        return;
    }

    result.insert_or_assign(mpe::ArticulationType::GhostNote, mpe::ArticulationMeta(mpe::ArticulationType::GhostNote,
                                                                                    ctx.profile->pattern(mpe::ArticulationType::GhostNote),
                                                                                    ctx.nominalTimestamp,
                                                                                    ctx.nominalDuration));
}

void NoteArticulationsParser::parseNoteHead(const Ms::Note* note, const PlaybackContext& ctx,
                                            mpe::ArticulationMetaMap& result) const
{
    mpe::ArticulationType typeByNoteHead = articulationTypeByNotehead(note->headGroup());

    if (typeByNoteHead == mpe::ArticulationType::Undefined) {
        return;
    }

    result.insert_or_assign(typeByNoteHead, mpe::ArticulationMeta(typeByNoteHead,
                                                                  ctx.profile->pattern(typeByNoteHead),
                                                                  ctx.nominalTimestamp,
                                                                  ctx.nominalDuration));
}

void NoteArticulationsParser::parseSpanners(const Ms::Note* note, const PlaybackContext& ctx,
                                            mpe::ArticulationMetaMap& result) const
{
    for (const Ms::Spanner* spanner : note->spannerFor()) {
        int spannerFrom = spanner->tick().ticks();
        int spannerTo = spanner->tick().ticks() + spanner->ticks().ticks();
        int spannerDurationTicks = spannerTo - spannerFrom;

        PlaybackContext spannerContext = ctx;
        spannerContext.nominalTimestamp = timestampFromTicks(note->score(), spannerFrom);
        spannerContext.nominalDuration = durationFromTicks(ctx.beatsPerSecond, spannerDurationTicks);
        spannerContext.nominalPositionTick = spannerFrom;
        spannerContext.nominalDurationTicks = spannerDurationTicks;

        SpannersMetaParser::instance()->parse(spanner, std::move(ctx), result);
    }
}
