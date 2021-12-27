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

#include "libmscore/spanner.h"
#include "libmscore/hairpin.h"
#include "libmscore/trill.h"
#include "libmscore/glissando.h"
#include "libmscore/note.h"

#include "playback/utils/pitchutils.h"

using namespace mu::engraving;

void SpannersMetaParser::doParse(const Ms::EngravingItem* item, const PlaybackContext& ctx,
                                 mpe::ArticulationMetaMap& result)
{
    IF_ASSERT_FAILED(item->isSpanner() && ctx.isValid()) {
        return;
    }

    const Ms::Spanner* spanner = Ms::toSpanner(item);

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    mpe::pitch_level_t overallPitchRange = 0;

    switch (spanner->type()) {
    case Ms::ElementType::SLUR:
        type = mpe::ArticulationType::Legato;
        break;
    case Ms::ElementType::PEDAL:
        type = mpe::ArticulationType::Pedal;
        break;
    case Ms::ElementType::LET_RING:
        type = mpe::ArticulationType::LaissezVibrer;
        break;
    case Ms::ElementType::HAIRPIN: {
        const Ms::Hairpin* hairpin = Ms::toHairpin(spanner);
        if (hairpin->isCrescendo()) {
            type = mpe::ArticulationType::Crescendo;
        } else if (hairpin->isDecrescendo()) {
            type = mpe::ArticulationType::Decrescendo;
        }
        break;
    }
    case Ms::ElementType::TRILL: {
        const Ms::Trill* trill = Ms::toTrill(spanner);
        if (trill->trillType() == Ms::Trill::Type::TRILL_LINE) {
            type = mpe::ArticulationType::Trill;
        } else if (trill->trillType() == Ms::Trill::Type::UPPRALL_LINE) {
            type = mpe::ArticulationType::UpPrall;
        } else if (trill->trillType() == Ms::Trill::Type::DOWNPRALL_LINE) {
            type = mpe::ArticulationType::PrallDown;
        } else if (trill->trillType() == Ms::Trill::Type::PRALLPRALL_LINE) {
            type = mpe::ArticulationType::LinePrall;
        }
        break;
    }
    case Ms::ElementType::GLISSANDO: {
        const Ms::Glissando* glissando = Ms::toGlissando(spanner);
        if (!glissando->playGlissando()) {
            break;
        }

        Ms::Note* startNote = Ms::toNote(glissando->startElement());
        Ms::Note* endNote = Ms::toNote(glissando->endElement());

        if (!startNote || !endNote) {
            break;
        }

        if (glissando->glissandoType() == Ms::GlissandoType::STRAIGHT
            || glissando->glissandoStyle() == Ms::GlissandoStyle::PORTAMENTO) {
            type = mpe::ArticulationType::ContinuousGlissando;
        } else {
            type = mpe::ArticulationType::DiscreteGlissando;
        }

        int startNoteTpc = startNote->tpc();
        int endNoteTpc = endNote->tpc();

        mpe::PitchClass startNotePitchClass = pitchClassFromTpc(startNoteTpc);
        mpe::PitchClass endNotePitchClass = pitchClassFromTpc(endNoteTpc);

        mpe::octave_t startNoteOctave = actualOctave(startNote->octave(), startNotePitchClass, Ms::tpc2alter(startNoteTpc));
        mpe::octave_t endNoteOctave = actualOctave(endNote->octave(), endNotePitchClass, Ms::tpc2alter(endNoteTpc));

        overallPitchRange = mpe::pitchLevelDiff(startNotePitchClass, startNoteOctave, endNotePitchClass, endNoteOctave);

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
    articulationMeta.pattern = ctx.profile->pattern(type);
    articulationMeta.timestamp = ctx.nominalTimestamp;
    articulationMeta.overallPitchChangesRange = std::abs(overallPitchRange);
    articulationMeta.overallDuration = ctx.nominalDuration;

    result.insert_or_assign(type, std::move(articulationMeta));
}
