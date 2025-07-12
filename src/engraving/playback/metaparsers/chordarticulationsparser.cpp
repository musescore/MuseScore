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

#include "chordarticulationsparser.h"

#include "dom/arpeggio.h"
#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tapping.h"

#include "playback/utils/arrangementutils.h"
#include "playback/filters/spannerfilter.h"

#include "internal/spannersmetaparser.h"
#include "internal/symbolsmetaparser.h"
#include "internal/annotationsmetaparser.h"
#include "internal/tremolometaparser.h"
#include "internal/arpeggiometaparser.h"
#include "internal/gracenotesmetaparser.h"
#include "internal/chordlinemetaparser.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void ChordArticulationsParser::buildChordArticulationMap(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    if (!chord || !ctx.isValid()) {
        LOGE() << "Unable to render playback events of invalid chord";
        return;
    }

    parse(chord, ctx, result);

    for (const auto& pair : result) {
        if (isSingleNoteArticulation(pair.first)) {
            continue;
        }

        duration_percentage_t occupiedFrom = occupiedPercentage(ctx.nominalTimestamp - pair.second.meta.timestamp,
                                                                pair.second.meta.overallDuration);
        duration_percentage_t occupiedTo = occupiedPercentage(ctx.nominalTimestamp + ctx.nominalDuration - pair.second.meta.timestamp,
                                                              pair.second.meta.overallDuration);

        result.updateOccupiedRange(pair.first, occupiedFrom, occupiedTo);
    }

    result.preCalculateAverageData();
}

void ChordArticulationsParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::CHORD) {
        return;
    }

    const Chord* chord = toChord(item);

    parseSpanners(chord, ctx, result);
    parseAnnotations(chord, ctx, result);
    parseTremolo(chord, ctx, result);
    parseArpeggio(chord, ctx, result);
    parseGraceNotes(chord, ctx, result);
    parseChordLine(chord, ctx, result);
    parseArticulationSymbols(chord, ctx, result);
    parseTapping(chord, ctx, result);
}

void ChordArticulationsParser::parseSpanners(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const SpannerMap& spannerMap = ctx.score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    auto intervals = spannerMap.findOverlapping(ctx.nominalPositionStartTick,
                                                ctx.nominalPositionEndTick,
                                                /*excludeCollisions*/ true);

    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (!SpannersMetaParser::isAbleToParse(spanner)) {
            continue;
        }

        if (SpannerFilter::isMultiStaffSpanner(spanner)) {
            if (spanner->part() != chord->part()) {
                continue;
            }
        } else {
            if (spanner->staffIdx() != chord->staffIdx()) {
                continue;
            }
        }

        if (!SpannerFilter::isItemPlayable(spanner, ctx)) {
            continue;
        }

        RenderingContext spannerContext = ctx;
        spannerContext.nominalTimestamp = timestampFromTicks(ctx.score, interval.start + ctx.positionTickOffset);
        spannerContext.nominalPositionStartTick = interval.start;
        spannerContext.nominalDurationTicks = SpannerFilter::spannerActualDurationTicks(spanner, interval.stop - interval.start);
        spannerContext.nominalPositionEndTick = spannerContext.nominalPositionStartTick + spannerContext.nominalDurationTicks;

        SpannersMetaParser::parse(spanner, std::move(spannerContext), result);
    }
}

void ChordArticulationsParser::parseArticulationSymbols(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    for (const Articulation* articulation : chord->articulations()) {
        SymbolsMetaParser::parse(articulation, ctx, result);
    }
}

void ChordArticulationsParser::parseAnnotations(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    for (const EngravingItem* annotation : chord->segment()->annotations()) {
        if (annotation->staffIdx() != chord->staffIdx()) {
            continue;
        }

        AnnotationsMetaParser::parse(annotation, ctx, result);
    }
}

void ChordArticulationsParser::parseTremolo(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    // single chord
    {
        const TremoloSingleChord* tremoloSingle = chord->tremoloSingleChord();
        if (tremoloSingle && tremoloSingle->playTremolo()) {
            TremoloSingleMetaParser::parse(tremoloSingle, ctx, result);
        }
    }

    // two chord
    {
        const TremoloTwoChord* tremoloTwo = chord->tremoloTwoChord();
        if (tremoloTwo && tremoloTwo->playTremolo()) {
            TremoloTwoMetaParser::parse(tremoloTwo, ctx, result);
        }
    }
}

void ChordArticulationsParser::parseArpeggio(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const Arpeggio* arpeggio = chord->arpeggio();
    if (!arpeggio) {
        return;
    }

    if (chord->notes().empty()) {
        return;
    }

    ArpeggioMetaParser::parse(arpeggio, ctx, result);
}

void ChordArticulationsParser::parseGraceNotes(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    for (const Chord* graceChord : chord->graceNotes()) {
        GraceNotesMetaParser::parse(graceChord, ctx, result);
    }
}

void ChordArticulationsParser::parseChordLine(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const ChordLine* chordLine = chord->chordLine();
    if (!chordLine || !chordLine->playChordLine()) {
        return;
    }

    ChordLineMetaParser::parse(chordLine, ctx, result);
}

void ChordArticulationsParser::parseTapping(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const Tapping* tapping = chord->tapping();
    if (!tapping || !tapping->playArticulation()) {
        return;
    }

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    switch (tapping->hand()) {
    case TappingHand::LEFT:
        type = mpe::ArticulationType::LeftHandTapping;
        break;
    case TappingHand::RIGHT:
        type = mpe::ArticulationType::RightHandTapping;
        break;
    case TappingHand::INVALID:
        break;
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(type, pattern, ctx.nominalTimestamp, ctx.nominalDuration), result);
}
