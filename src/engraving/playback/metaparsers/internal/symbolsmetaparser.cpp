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

#include "symbolsmetaparser.h"

#include "libmscore/articulation.h"

using namespace mu::engraving;

void SymbolsMetaParser::doParse(const mu::engraving::EngravingItem* item, const RenderingContext& ctx,
                                mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == mu::engraving::ElementType::ARTICULATION) {
        return;
    }

    const mu::engraving::Articulation* articulationSymbol = mu::engraving::toArticulation(item);

    if (!articulationSymbol->playArticulation()) {
        return;
    }

    mpe::ArticulationTypeSet types;

    switch (articulationSymbol->symId()) {
    case mu::engraving::SymId::articAccentAbove:
    case mu::engraving::SymId::articAccentBelow:
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case mu::engraving::SymId::articAccentStaccatoAbove:
    case mu::engraving::SymId::articAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::Accent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articLaissezVibrerAbove:
    case mu::engraving::SymId::articLaissezVibrerBelow:
        types.emplace(mpe::ArticulationType::LaissezVibrer);
        break;
    case mu::engraving::SymId::articMarcatoAbove:
    case mu::engraving::SymId::articMarcatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        break;
    case mu::engraving::SymId::articMarcatoStaccatoAbove:
    case mu::engraving::SymId::articMarcatoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articMarcatoTenutoAbove:
    case mu::engraving::SymId::articMarcatoTenutoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case mu::engraving::SymId::articSoftAccentAbove:
    case mu::engraving::SymId::articSoftAccentBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        break;
    case mu::engraving::SymId::articSoftAccentStaccatoAbove:
    case mu::engraving::SymId::articSoftAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articSoftAccentTenutoAbove:
    case mu::engraving::SymId::articSoftAccentTenutoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case mu::engraving::SymId::articSoftAccentTenutoStaccatoAbove:
    case mu::engraving::SymId::articSoftAccentTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articStaccatissimoAbove:
    case mu::engraving::SymId::articStaccatissimoBelow:
    case mu::engraving::SymId::articStaccatissimoStrokeAbove:
    case mu::engraving::SymId::articStaccatissimoStrokeBelow:
    case mu::engraving::SymId::articStaccatissimoWedgeAbove:
    case mu::engraving::SymId::articStaccatissimoWedgeBelow:
        types.emplace(mpe::ArticulationType::Staccatissimo);
        break;
    case mu::engraving::SymId::articStaccatoAbove:
    case mu::engraving::SymId::articStaccatoBelow:
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articTenutoAbove:
    case mu::engraving::SymId::articTenutoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case mu::engraving::SymId::articTenutoStaccatoAbove:
    case mu::engraving::SymId::articTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case mu::engraving::SymId::articTenutoAccentAbove:
    case mu::engraving::SymId::articTenutoAccentBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case mu::engraving::SymId::dynamicSforzando:
    case mu::engraving::SymId::dynamicSforzando1:
    case mu::engraving::SymId::dynamicSforzandoPianissimo:
    case mu::engraving::SymId::dynamicSforzandoPiano:
    case mu::engraving::SymId::dynamicSforzato:
    case mu::engraving::SymId::dynamicSforzatoFF:
    case mu::engraving::SymId::dynamicSforzatoPiano:
        types.emplace(mpe::ArticulationType::Subito);
        break;
    case mu::engraving::SymId::guitarFadeIn:
        types.emplace(mpe::ArticulationType::FadeIn);
        break;
    case mu::engraving::SymId::guitarVolumeSwell:
        types.emplace(mpe::ArticulationType::VolumeSwell);
        break;
    case mu::engraving::SymId::guitarFadeOut:
        types.emplace(mpe::ArticulationType::FadeOut);
        break;
    case mu::engraving::SymId::stringsHalfHarmonic:
    case mu::engraving::SymId::stringsHarmonic:
        types.emplace(mpe::ArticulationType::Harmonic);
        break;
    case mu::engraving::SymId::stringsMuteOn:
    case mu::engraving::SymId::elecMute:
    case mu::engraving::SymId::handbellsMutedMartellato:
    case mu::engraving::SymId::brassMuteHalfClosed:
    case mu::engraving::SymId::brassMuteClosed:
    case mu::engraving::SymId::brassHarmonMuteStemHalfRight:
    case mu::engraving::SymId::brassHarmonMuteStemHalfLeft:
    case mu::engraving::SymId::brassHarmonMuteClosed:
        types.emplace(mpe::ArticulationType::Mute);
        break;
    case mu::engraving::SymId::brassHarmonMuteStemOpen:
    case mu::engraving::SymId::brassMuteOpen:
    case mu::engraving::SymId::guitarHalfOpenPedal:
    case mu::engraving::SymId::guitarOpenPedal:
    case mu::engraving::SymId::vocalMouthOpen:
    case mu::engraving::SymId::vocalMouthSlightlyOpen:
    case mu::engraving::SymId::vocalMouthWideOpen:
    case mu::engraving::SymId::windOpenHole:
    case mu::engraving::SymId::elecUnmute:
    case mu::engraving::SymId::stringsMuteOff:
        types.emplace(mpe::ArticulationType::Open);
        break;

    case mu::engraving::SymId::pluckedLeftHandPizzicato:
        types.emplace(mpe::ArticulationType::Pizzicato);
        break;
    case mu::engraving::SymId::pluckedSnapPizzicatoAbove:
    case mu::engraving::SymId::pluckedSnapPizzicatoBelow:
        types.emplace(mpe::ArticulationType::SnapPizzicato);
        break;
    case mu::engraving::SymId::stringsUpBow:
        types.emplace(mpe::ArticulationType::UpBow);
        break;
    case mu::engraving::SymId::stringsDownBow:
        types.emplace(mpe::ArticulationType::DownBow);
        break;
    case mu::engraving::SymId::stringsJeteAbove:
    case mu::engraving::SymId::stringsJeteBelow:
        types.emplace(mpe::ArticulationType::Jete);
        break;
    case mu::engraving::SymId::noteheadXWhole:
    case mu::engraving::SymId::noteheadXOrnate:
    case mu::engraving::SymId::noteheadXBlack:
    case mu::engraving::SymId::noteheadXDoubleWhole:
    case mu::engraving::SymId::noteheadWholeWithX:
    case mu::engraving::SymId::noteheadVoidWithX:
        types.emplace(mpe::ArticulationType::CrossNote);
        break;
    case mu::engraving::SymId::noteheadCircleSlash:
    case mu::engraving::SymId::noteheadCircledBlack:
    case mu::engraving::SymId::noteheadCircledHalf:
    case mu::engraving::SymId::noteheadCircledBlackLarge:
    case mu::engraving::SymId::noteheadCircledDoubleWhole:
    case mu::engraving::SymId::noteheadCircledDoubleWholeLarge:
    case mu::engraving::SymId::noteheadCircledHalfLarge:
    case mu::engraving::SymId::noteheadCircledWhole:
    case mu::engraving::SymId::noteheadCircledWholeLarge:
        types.emplace(mpe::ArticulationType::CircleNote);
        break;
    case mu::engraving::SymId::noteheadDiamondBlack:
    case mu::engraving::SymId::noteheadDiamondBlackOld:
    case mu::engraving::SymId::noteheadDiamondBlackWide:
    case mu::engraving::SymId::noteheadDiamondDoubleWhole:
    case mu::engraving::SymId::noteheadDiamondHalfWide:
    case mu::engraving::SymId::noteheadDiamondDoubleWholeOld:
    case mu::engraving::SymId::noteheadDiamondWhole:
    case mu::engraving::SymId::noteheadDiamondWholeOld:
    case mu::engraving::SymId::noteheadDiamondHalf:
    case mu::engraving::SymId::noteheadDiamondHalfFilled:
    case mu::engraving::SymId::noteheadDiamondHalfOld:
        types.emplace(mpe::ArticulationType::DiamondNote);
        break;
    case mu::engraving::SymId::noteheadTriangleDownBlack:
    case mu::engraving::SymId::noteheadTriangleDownDoubleWhole:
    case mu::engraving::SymId::noteheadTriangleDownHalf:
    case mu::engraving::SymId::noteheadTriangleDownWhite:
    case mu::engraving::SymId::noteheadTriangleDownWhole:
    case mu::engraving::SymId::noteheadTriangleLeftBlack:
    case mu::engraving::SymId::noteheadTriangleLeftWhite:
    case mu::engraving::SymId::noteheadTriangleRightBlack:
    case mu::engraving::SymId::noteheadTriangleRightWhite:
    case mu::engraving::SymId::noteheadTriangleRoundDownBlack:
    case mu::engraving::SymId::noteheadTriangleRoundDownWhite:
    case mu::engraving::SymId::noteheadTriangleUpBlack:
    case mu::engraving::SymId::noteheadTriangleUpDoubleWhole:
    case mu::engraving::SymId::noteheadTriangleUpHalf:
    case mu::engraving::SymId::noteheadTriangleUpRightBlack:
    case mu::engraving::SymId::noteheadTriangleUpRightWhite:
    case mu::engraving::SymId::noteheadTriangleUpWhite:
    case mu::engraving::SymId::noteheadTriangleUpWhole:
        types.emplace(mpe::ArticulationType::TriangleNote);
        break;
    case mu::engraving::SymId::brassScoop:
        types.emplace(mpe::ArticulationType::Scoop);
        break;
    case mu::engraving::SymId::brassPlop:
        types.emplace(mpe::ArticulationType::Plop);
        break;
    case mu::engraving::SymId::brassFallLipLong:
    case mu::engraving::SymId::brassFallLipMedium:
    case mu::engraving::SymId::brassFallLipShort:
    case mu::engraving::SymId::brassFallRoughLong:
    case mu::engraving::SymId::brassFallRoughMedium:
    case mu::engraving::SymId::brassFallRoughShort:
        types.emplace(mpe::ArticulationType::QuickFall);
        break;
    case mu::engraving::SymId::brassFallSmoothLong:
    case mu::engraving::SymId::brassFallSmoothMedium:
    case mu::engraving::SymId::brassFallSmoothShort:
        types.emplace(mpe::ArticulationType::Fall);
        break;
    case mu::engraving::SymId::brassDoitLong:
    case mu::engraving::SymId::brassDoitMedium:
    case mu::engraving::SymId::brassDoitShort:
        types.emplace(mpe::ArticulationType::Doit);
        break;
    case mu::engraving::SymId::brassBend:
        types.emplace(mpe::ArticulationType::Bend);
        break;
    case mu::engraving::SymId::dynamicCrescendoHairpin:
        types.emplace(mpe::ArticulationType::Crescendo);
        break;
    case mu::engraving::SymId::dynamicDiminuendoHairpin:
        types.emplace(mpe::ArticulationType::Decrescendo);
        break;
    case mu::engraving::SymId::ornamentUpPrall:
        types.emplace(mpe::ArticulationType::UpPrall);
        break;
    case mu::engraving::SymId::ornamentPrallDown:
        types.emplace(mpe::ArticulationType::PrallDown);
        break;
    case mu::engraving::SymId::ornamentPrallUp:
        types.emplace(mpe::ArticulationType::PrallUp);
        break;
    case mu::engraving::SymId::ornamentLinePrall:
        types.emplace(mpe::ArticulationType::LinePrall);
        break;
    case mu::engraving::SymId::ornamentPrallMordent:
        types.emplace(mpe::ArticulationType::PrallMordent);
        break;
    case mu::engraving::SymId::ornamentUpMordent:
        types.emplace(mpe::ArticulationType::UpMordent);
        break;
    case mu::engraving::SymId::ornamentMordent:
        types.emplace(mpe::ArticulationType::LowerMordent);
        break;
    case mu::engraving::SymId::ornamentDownMordent:
        types.emplace(mpe::ArticulationType::DownMordent);
        break;
    case mu::engraving::SymId::ornamentTurn:
    case mu::engraving::SymId::brassJazzTurn:
        types.emplace(mpe::ArticulationType::Turn);
        break;
    case mu::engraving::SymId::ornamentTurnInverted:
    case mu::engraving::SymId::ornamentTurnSlash:
        types.emplace(mpe::ArticulationType::InvertedTurn);
        break;
    case mu::engraving::SymId::ornamentTrill:
        if (articulationSymbol->ornamentStyle() == mu::engraving::OrnamentStyle::DEFAULT) {
            types.emplace(mpe::ArticulationType::Trill);
        } else {
            types.emplace(mpe::ArticulationType::TrillBaroque);
        }
        break;
    case mu::engraving::SymId::ornamentShortTrill:
        types.emplace(mpe::ArticulationType::UpperMordent);
        break;
    case mu::engraving::SymId::ornamentTremblement:
    case mu::engraving::SymId::ornamentTremblementCouperin:
        types.emplace(mpe::ArticulationType::Tremblement);
        break;
    case mu::engraving::SymId::graceNoteAcciaccaturaStemDown:
    case mu::engraving::SymId::graceNoteAcciaccaturaStemUp:
    case mu::engraving::SymId::graceNoteSlashStemDown:
    case mu::engraving::SymId::graceNoteSlashStemUp:
        types.emplace(mpe::ArticulationType::Acciaccatura);
        break;
    case mu::engraving::SymId::graceNoteAppoggiaturaStemDown:
    case mu::engraving::SymId::graceNoteAppoggiaturaStemUp:
        types.emplace(mpe::ArticulationType::PreAppoggiatura);
        break;
    case mu::engraving::SymId::glissandoUp:
    case mu::engraving::SymId::glissandoDown:
        types.emplace(mpe::ArticulationType::DiscreteGlissando);
        break;
    case mu::engraving::SymId::wiggleArpeggiatoDown:
    case mu::engraving::SymId::wiggleArpeggiatoDownArrow:
        types.emplace(mpe::ArticulationType::ArpeggioDown);
        break;
    case mu::engraving::SymId::wiggleArpeggiatoUp:
    case mu::engraving::SymId::wiggleArpeggiatoUpArrow:
        types.emplace(mpe::ArticulationType::ArpeggioUp);
        break;
    case mu::engraving::SymId::wiggleArpeggiatoDownSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightDown);
        break;
    case mu::engraving::SymId::wiggleArpeggiatoUpSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightUp);
        break;
    case mu::engraving::SymId::wiggleVibratoLargeFaster:
    case mu::engraving::SymId::wiggleVibratoLargeFasterStill:
    case mu::engraving::SymId::wiggleVibratoLargeFastest:
    case mu::engraving::SymId::wiggleVibratoLargeSlow:
    case mu::engraving::SymId::wiggleVibratoLargeSlower:
    case mu::engraving::SymId::wiggleVibratoLargeSlowest:
    case mu::engraving::SymId::wiggleVibratoLargestFast:
    case mu::engraving::SymId::wiggleVibratoLargestFaster:
    case mu::engraving::SymId::wiggleVibratoLargestFasterStill:
    case mu::engraving::SymId::wiggleVibratoLargestFastest:
    case mu::engraving::SymId::wiggleVibratoLargestSlow:
    case mu::engraving::SymId::wiggleVibratoLargestSlowest:
    case mu::engraving::SymId::wiggleVibratoMediumFast:
    case mu::engraving::SymId::wiggleVibratoMediumFaster:
    case mu::engraving::SymId::wiggleVibratoMediumFasterStill:
    case mu::engraving::SymId::wiggleVibratoMediumFastest:
    case mu::engraving::SymId::wiggleVibratoMediumSlow:
    case mu::engraving::SymId::wiggleVibratoMediumSlowest:
    case mu::engraving::SymId::wiggleVibratoSmallFast:
    case mu::engraving::SymId::wiggleVibratoSmallFaster:
    case mu::engraving::SymId::wiggleVibratoSmallFasterStill:
    case mu::engraving::SymId::wiggleVibratoSmallFastest:
    case mu::engraving::SymId::wiggleVibratoSmallSlow:
    case mu::engraving::SymId::wiggleVibratoSmallSlower:
    case mu::engraving::SymId::wiggleVibratoSmallSlowest:
    case mu::engraving::SymId::wiggleVibratoSmallestFast:
    case mu::engraving::SymId::wiggleVibratoSmallestFaster:
    case mu::engraving::SymId::wiggleVibratoSmallestFasterStill:
    case mu::engraving::SymId::wiggleVibratoSmallestFastest:
    case mu::engraving::SymId::wiggleVibratoSmallestSlow:
    case mu::engraving::SymId::wiggleVibratoSmallestSlower:
    case mu::engraving::SymId::wiggleVibratoSmallestSlowest:
    case mu::engraving::SymId::wiggleVibratoStart:
    case mu::engraving::SymId::wiggleVibratoLargeFast:
    case mu::engraving::SymId::wiggleVibrato:
        types.emplace(mpe::ArticulationType::Vibrato);
        break;
    case mu::engraving::SymId::wiggleVibratoWide:
        types.emplace(mpe::ArticulationType::WideVibrato);
        break;
    default:
        break;
    }

    for (const auto& type : types) {
        appendArticulationData(mpe::ArticulationMeta(type, ctx.profile->pattern(type), ctx.nominalTimestamp, ctx.nominalDuration), result);
    }
}
