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

#include "spannersmetaparser.h"

#include "libmscore/glissando.h"
#include "libmscore/note.h"
#include "libmscore/spanner.h"
#include "libmscore/trill.h"
#include "libmscore/pedal.h"
#include "libmscore/tempo.h"

#include "playback/utils/pitchutils.h"
#include "playback/filters/spannerfilter.h"

using namespace mu::engraving;

bool SpannersMetaParser::isAbleToParse(const EngravingItem* spannerItem)
{
    static const std::unordered_set<ElementType> SUPPORTED_TYPES = {
        ElementType::SLUR,
        ElementType::PEDAL,
        ElementType::LET_RING,
        ElementType::PALM_MUTE,
        ElementType::TRILL,
        ElementType::GLISSANDO,
    };

    return SUPPORTED_TYPES.find(spannerItem->type()) != SUPPORTED_TYPES.cend();
}

void SpannersMetaParser::doParse(const EngravingItem* item, const RenderingContext& spannerCtx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->isSpanner()) {
        return;
    }

    const Spanner* spanner = toSpanner(item);

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    mpe::pitch_level_t overallPitchRange = 0;
    mpe::dynamic_level_t overallDynamicRange = 0;
    int overallDurationTicks = SpannerFilter::spannerActualDurationTicks(spanner, spannerCtx.nominalDurationTicks);

    switch (spanner->type()) {
    case ElementType::SLUR: {
        type = mpe::ArticulationType::Legato;
        break;
    }
    case ElementType::PEDAL: {
        type = mpe::ArticulationType::Pedal;
        break;
    }
    case ElementType::LET_RING:
        type = mpe::ArticulationType::LaissezVibrer;
        break;
    case ElementType::PALM_MUTE: {
        type = mpe::ArticulationType::Mute;
        break;
    }
    case ElementType::TRILL: {
        const Trill* trill = toTrill(spanner);

        if (!trill->playArticulation()) {
            return;
        }

        if (trill->trillType() == TrillType::TRILL_LINE) {
            type = mpe::ArticulationType::Trill;
        } else if (trill->trillType() == TrillType::UPPRALL_LINE) {
            type = mpe::ArticulationType::UpPrall;
        } else if (trill->trillType() == TrillType::DOWNPRALL_LINE) {
            type = mpe::ArticulationType::PrallDown;
        } else if (trill->trillType() == TrillType::PRALLPRALL_LINE) {
            type = mpe::ArticulationType::LinePrall;
        }
        break;
    }
    case ElementType::GLISSANDO: {
        const Glissando* glissando = toGlissando(spanner);
        if (!glissando->playGlissando()) {
            break;
        }

        Note* startNote = toNote(glissando->startElement());
        Note* endNote = toNote(glissando->endElement());

        if (!startNote || !endNote) {
            break;
        }

        if (glissando->glissandoStyle() == GlissandoStyle::PORTAMENTO) {
            type = mpe::ArticulationType::ContinuousGlissando;
        } else {
            type = mpe::ArticulationType::DiscreteGlissando;
        }

        int startNoteTpc = startNote->playingTpc();
        int endNoteTpc = endNote->playingTpc();

        mpe::PitchClass startNotePitchClass = pitchClassFromTpc(startNoteTpc);
        mpe::PitchClass endNotePitchClass = pitchClassFromTpc(endNoteTpc);

        mpe::octave_t startNoteOctave = actualOctave(startNote->playingOctave(), startNotePitchClass, tpc2alter(startNoteTpc));
        mpe::octave_t endNoteOctave = actualOctave(endNote->playingOctave(), endNotePitchClass, tpc2alter(endNoteTpc));

        overallPitchRange = mpe::pitchLevelDiff(endNotePitchClass, endNoteOctave, startNotePitchClass, startNoteOctave);

        break;
    }
    default:
        break;
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = spannerCtx.profile->pattern(type);
    articulationMeta.timestamp = spannerCtx.nominalTimestamp;
    articulationMeta.overallPitchChangesRange = overallPitchRange;
    articulationMeta.overallDynamicChangesRange = overallDynamicRange;
    articulationMeta.overallDuration = spannerDuration(spanner->score(),
                                                       spannerCtx.nominalPositionStartTick,
                                                       overallDurationTicks);

    appendArticulationData(std::move(articulationMeta), result);
}

mu::mpe::duration_t SpannersMetaParser::spannerDuration(const Score* score, const int positionTick, const int durationTicks)
{
    if (!score) {
        return 0;
    }

    BeatsPerSecond startBps = score->tempomap()->tempo(positionTick);
    BeatsPerSecond endBps = score->tempomap()->tempo(positionTick + durationTicks);

    if (startBps == endBps) {
        return durationFromTicks(startBps.val, durationTicks);
    }

    mpe::duration_t result = (durationFromTicks(startBps.val, durationTicks)
                              + durationFromTicks(endBps.val, durationTicks)) / 2;

    return result;
}
