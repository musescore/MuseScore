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
    parseArticulationSymbols(chord, ctx, result);
    parseAnnotations(chord, ctx, result);
    parseTremolo(chord, ctx, result);
    parseArpeggio(chord, ctx, result);
    parseGraceNotes(chord, ctx, result);
    parseChordLine(chord, ctx, result);
}

void ChordArticulationsParser::parseSpanners(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const Score* score = chord->score();

    const std::multimap<int, Spanner*>& spannerMap = score->spanner();
    auto startIt = spannerMap.lower_bound(ctx.nominalPositionStartTick);
    auto endIt = spannerMap.upper_bound(ctx.nominalPositionEndTick);
    for (auto it = startIt; it != endIt; ++it) {
        Spanner* spanner = it->second;

        if (spanner->staffIdx() != chord->staffIdx()) {
            continue;
        }

        int spannerFrom = spanner->startUniqueTicks();
        int spannerTo = spanner->endUniqueTicks();
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
    const Tremolo* tremolo = chord->tremolo();

    if (!tremolo) {
        return;
    }

    TremoloMetaParser::parse(tremolo, ctx, result);
}

void ChordArticulationsParser::parseArpeggio(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    const Arpeggio* arpeggio = chord->arpeggio();

    if (!arpeggio) {
        return;
    }

    if (chord->notes().empty() || chord->notes().size() == 1) {
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

    if (!chordLine) {
        return;
    }

    ChordLineMetaParser::parse(chordLine, ctx, result);
}
