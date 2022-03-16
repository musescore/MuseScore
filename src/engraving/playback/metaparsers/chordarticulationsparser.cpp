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

#include "chordarticulationsparser.h"

#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/spanner.h"
#include "libmscore/tremolo.h"
#include "libmscore/arpeggio.h"
#include "libmscore/chordline.h"

#include "playback/utils/arrangementutils.h"
#include "internal/spannersmetaparser.h"
#include "internal/symbolsmetaparser.h"
#include "internal/annotationsmetaparser.h"
#include "internal/tremolometaparser.h"
#include "internal/arpeggiometaparser.h"
#include "internal/gracenotesmetaparser.h"
#include "internal/chordlinemetaparser.h"

using namespace mu::engraving;
using namespace mu::mpe;

void ChordArticulationsParser::buildChordArticulationMap(const Ms::Chord* chord, const RenderingContext& ctx,
                                                         mpe::ArticulationMap& result)
{
    if (!chord) {
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

void ChordArticulationsParser::doParse(const Ms::EngravingItem* item, const RenderingContext& ctx,
                                       mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == Ms::ElementType::CHORD && ctx.isValid()) {
        return;
    }

    const Ms::Chord* chord = Ms::toChord(item);

    parseSpanners(chord, ctx, result);
    parseArticulationSymbols(chord, ctx, result);
    parseAnnotations(chord, ctx, result);
    parseTremolo(chord, ctx, result);
    parseArpeggio(chord, ctx, result);
    parseGraceNotes(chord, ctx, result);
    parseChordLine(chord, ctx, result);
}

void ChordArticulationsParser::parseSpanners(const Ms::Chord* chord, const RenderingContext& ctx,
                                             mpe::ArticulationMap& result)
{
    const Ms::Score* score = chord->score();

    for (const auto& pair : score->spanner()) {
        const Ms::Spanner* spanner = pair.second;

        if (spanner->staffIdx() != chord->staffIdx()) {
            continue;
        }

        const Ms::Segment* startSegment = spanner->startSegment();
        const Ms::Segment* endSegment = spanner->endSegment();

        if (!startSegment || !endSegment) {
            continue;
        }

        int spannerFrom = score->repeatList().tick2utick(startSegment->tick().ticks());
        int spannerTo = spannerFrom + std::abs(endSegment->tick().ticks() + endSegment->ticks().ticks());

        if (spannerFrom > ctx.nominalPositionEndTick || spannerTo <= ctx.nominalPositionStartTick) {
            continue;
        }

        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks == 0) {
            continue;
        }

        RenderingContext spannerContext = ctx;
        spannerContext.nominalTimestamp = timestampFromTicks(score, spannerFrom);
        spannerContext.nominalDuration = durationFromTicks(ctx.beatsPerSecond.val, spannerDurationTicks);
        spannerContext.nominalPositionStartTick = spannerFrom;
        spannerContext.nominalDurationTicks = spannerDurationTicks;

        SpannersMetaParser::parse(spanner, std::move(spannerContext), result);
    }
}

void ChordArticulationsParser::parseArticulationSymbols(const Ms::Chord* chord, const RenderingContext& ctx,
                                                        mpe::ArticulationMap& result)
{
    for (const Ms::Articulation* articulation : chord->articulations()) {
        SymbolsMetaParser::parse(articulation, ctx, result);
    }
}

void ChordArticulationsParser::parseAnnotations(const Ms::Chord* chord, const RenderingContext& ctx,
                                                mpe::ArticulationMap& result)
{
    for (const Ms::EngravingItem* annotation : chord->segment()->annotations()) {
        if (annotation->staffIdx() != chord->staffIdx()) {
            continue;
        }

        AnnotationsMetaParser::parse(annotation, ctx, result);
    }
}

void ChordArticulationsParser::parseTremolo(const Ms::Chord* chord, const RenderingContext& ctx,
                                            mpe::ArticulationMap& result)
{
    const Ms::Tremolo* tremolo = chord->tremolo();

    if (!tremolo) {
        return;
    }

    TremoloMetaParser::parse(tremolo, ctx, result);
}

void ChordArticulationsParser::parseArpeggio(const Ms::Chord* chord, const RenderingContext& ctx,
                                             mpe::ArticulationMap& result)
{
    const Ms::Arpeggio* arpeggio = chord->arpeggio();

    if (!arpeggio) {
        return;
    }

    if (chord->notes().empty() || chord->notes().size() == 1) {
        return;
    }

    ArpeggioMetaParser::parse(arpeggio, ctx, result);
}

void ChordArticulationsParser::parseGraceNotes(const Ms::Chord* chord, const RenderingContext& ctx,
                                               mpe::ArticulationMap& result)
{
    for (const Ms::Chord* graceChord : chord->graceNotes()) {
        GraceNotesMetaParser::parse(graceChord, ctx, result);
    }
}

void ChordArticulationsParser::parseChordLine(const Ms::Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const Ms::ChordLine* chordLine = chord->chordLine();

    if (!chordLine) {
        return;
    }

    ChordLineMetaParser::parse(chordLine, ctx, result);
}
