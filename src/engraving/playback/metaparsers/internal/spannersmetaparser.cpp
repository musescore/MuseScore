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

#include "spannersmetaparser.h"

#include "dom/glissando.h"
#include "dom/note.h"
#include "dom/spanner.h"
#include "dom/trill.h"
#include "dom/pedal.h"
#include "dom/tempo.h"

#include "playback/utils/pitchutils.h"
#include "playback/filters/spannerfilter.h"

using namespace mu::engraving;
using namespace muse;

bool SpannersMetaParser::isAbleToParse(const EngravingItem* spannerItem)
{
    static const std::unordered_set<ElementType> SUPPORTED_TYPES {
        ElementType::SLUR,
        ElementType::HAMMER_ON_PULL_OFF,
        ElementType::PEDAL,
        ElementType::LET_RING,
        ElementType::PALM_MUTE,
        ElementType::TRILL,
        ElementType::GLISSANDO,
        ElementType::GUITAR_BEND,
        ElementType::VIBRATO,
    };

    return muse::contains(SUPPORTED_TYPES, spannerItem->type());
}

void SpannersMetaParser::doParse(const EngravingItem* item, const RenderingContext& spannerCtx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->isSpanner()) {
        return;
    }

    const Spanner* spanner = toSpanner(item);
    if (!spanner->playSpanner()) {
        return;
    }

    const int overallDurationTicks = SpannerFilter::spannerActualDurationTicks(spanner, spannerCtx.nominalDurationTicks);

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;
    mpe::pitch_level_t overallPitchRange = 0;
    mpe::dynamic_level_t overallDynamicRange = 0;

    switch (spanner->type()) {
    case ElementType::SLUR:
    case ElementType::HAMMER_ON_PULL_OFF: {
        type = mpe::ArticulationType::Legato;
        break;
    }
    case ElementType::PEDAL: {
        type = mpe::ArticulationType::Pedal;
        break;
    }
    case ElementType::LET_RING:
        type = mpe::ArticulationType::LetRing;
        break;
    case ElementType::PALM_MUTE: {
        type = mpe::ArticulationType::PalmMute;
        break;
    }
    case ElementType::GUITAR_BEND: {
        type = mpe::ArticulationType::Multibend;
        break;
    }
    case ElementType::VIBRATO: {
        type = mpe::ArticulationType::Vibrato;
        break;
    }
    case ElementType::TRILL: {
        const Trill* trill = toTrill(spanner);

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
        const Note* startNote = toNote(glissando->startElement());
        const Note* endNote = toNote(glissando->endElement());

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

    const mpe::ArticulationPattern& pattern = spannerCtx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = pattern;
    articulationMeta.timestamp = spannerCtx.nominalTimestamp;
    articulationMeta.overallPitchChangesRange = overallPitchRange;
    articulationMeta.overallDynamicChangesRange = overallDynamicRange;
    articulationMeta.overallDuration = durationFromStartAndTicks(spannerCtx.score,
                                                                 spannerCtx.nominalPositionStartTick,
                                                                 overallDurationTicks,
                                                                 spannerCtx.positionTickOffset);

    appendArticulationData(std::move(articulationMeta), result);
}
