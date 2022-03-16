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

void SymbolsMetaParser::doParse(const Ms::EngravingItem* item, const RenderingContext& ctx,
                                mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == Ms::ElementType::ARTICULATION && ctx.isValid()) {
        return;
    }

    const Ms::Articulation* articulationSymbol = Ms::toArticulation(item);

    if (!articulationSymbol->playArticulation()) {
        return;
    }

    mpe::ArticulationTypeSet types;

    switch (articulationSymbol->symId()) {
    case Ms::SymId::articAccentAbove:
    case Ms::SymId::articAccentBelow:
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case Ms::SymId::articAccentStaccatoAbove:
    case Ms::SymId::articAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::Accent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articLaissezVibrerAbove:
    case Ms::SymId::articLaissezVibrerBelow:
        types.emplace(mpe::ArticulationType::LaissezVibrer);
        break;
    case Ms::SymId::articMarcatoAbove:
    case Ms::SymId::articMarcatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        break;
    case Ms::SymId::articMarcatoStaccatoAbove:
    case Ms::SymId::articMarcatoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articMarcatoTenutoAbove:
    case Ms::SymId::articMarcatoTenutoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case Ms::SymId::articSoftAccentAbove:
    case Ms::SymId::articSoftAccentBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        break;
    case Ms::SymId::articSoftAccentStaccatoAbove:
    case Ms::SymId::articSoftAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articSoftAccentTenutoAbove:
    case Ms::SymId::articSoftAccentTenutoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case Ms::SymId::articSoftAccentTenutoStaccatoAbove:
    case Ms::SymId::articSoftAccentTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articStaccatissimoAbove:
    case Ms::SymId::articStaccatissimoBelow:
    case Ms::SymId::articStaccatissimoStrokeAbove:
    case Ms::SymId::articStaccatissimoStrokeBelow:
    case Ms::SymId::articStaccatissimoWedgeAbove:
    case Ms::SymId::articStaccatissimoWedgeBelow:
        types.emplace(mpe::ArticulationType::Staccatissimo);
        break;
    case Ms::SymId::articStaccatoAbove:
    case Ms::SymId::articStaccatoBelow:
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articTenutoAbove:
    case Ms::SymId::articTenutoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case Ms::SymId::articTenutoStaccatoAbove:
    case Ms::SymId::articTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case Ms::SymId::articTenutoAccentAbove:
    case Ms::SymId::articTenutoAccentBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case Ms::SymId::dynamicSforzando:
    case Ms::SymId::dynamicSforzando1:
    case Ms::SymId::dynamicSforzandoPianissimo:
    case Ms::SymId::dynamicSforzandoPiano:
    case Ms::SymId::dynamicSforzato:
    case Ms::SymId::dynamicSforzatoFF:
    case Ms::SymId::dynamicSforzatoPiano:
        types.emplace(mpe::ArticulationType::Subito);
        break;
    case Ms::SymId::guitarFadeIn:
        types.emplace(mpe::ArticulationType::FadeIn);
        break;
    case Ms::SymId::guitarVolumeSwell:
        types.emplace(mpe::ArticulationType::VolumeSwell);
        break;
    case Ms::SymId::guitarFadeOut:
        types.emplace(mpe::ArticulationType::FadeOut);
        break;
    case Ms::SymId::stringsHalfHarmonic:
    case Ms::SymId::stringsHarmonic:
        types.emplace(mpe::ArticulationType::Harmonic);
        break;
    case Ms::SymId::stringsMuteOn:
    case Ms::SymId::elecMute:
    case Ms::SymId::handbellsMutedMartellato:
    case Ms::SymId::brassMuteHalfClosed:
    case Ms::SymId::brassMuteClosed:
    case Ms::SymId::brassHarmonMuteStemHalfRight:
    case Ms::SymId::brassHarmonMuteStemHalfLeft:
    case Ms::SymId::brassHarmonMuteClosed:
        types.emplace(mpe::ArticulationType::Mute);
        break;
    case Ms::SymId::brassHarmonMuteStemOpen:
    case Ms::SymId::brassMuteOpen:
    case Ms::SymId::guitarHalfOpenPedal:
    case Ms::SymId::guitarOpenPedal:
    case Ms::SymId::vocalMouthOpen:
    case Ms::SymId::vocalMouthSlightlyOpen:
    case Ms::SymId::vocalMouthWideOpen:
    case Ms::SymId::windOpenHole:
    case Ms::SymId::elecUnmute:
    case Ms::SymId::stringsMuteOff:
        types.emplace(mpe::ArticulationType::Open);
        break;

    case Ms::SymId::pluckedLeftHandPizzicato:
        types.emplace(mpe::ArticulationType::Pizzicato);
        break;
    case Ms::SymId::pluckedSnapPizzicatoAbove:
    case Ms::SymId::pluckedSnapPizzicatoBelow:
        types.emplace(mpe::ArticulationType::SnapPizzicato);
        break;
    case Ms::SymId::stringsUpBow:
        types.emplace(mpe::ArticulationType::UpBow);
        break;
    case Ms::SymId::stringsDownBow:
        types.emplace(mpe::ArticulationType::DownBow);
        break;
    case Ms::SymId::stringsJeteAbove:
    case Ms::SymId::stringsJeteBelow:
        types.emplace(mpe::ArticulationType::Jete);
        break;
    case Ms::SymId::noteheadXWhole:
    case Ms::SymId::noteheadXOrnate:
    case Ms::SymId::noteheadXBlack:
    case Ms::SymId::noteheadXDoubleWhole:
    case Ms::SymId::noteheadWholeWithX:
    case Ms::SymId::noteheadVoidWithX:
        types.emplace(mpe::ArticulationType::CrossNote);
        break;
    case Ms::SymId::noteheadCircleSlash:
    case Ms::SymId::noteheadCircledBlack:
    case Ms::SymId::noteheadCircledHalf:
    case Ms::SymId::noteheadCircledBlackLarge:
    case Ms::SymId::noteheadCircledDoubleWhole:
    case Ms::SymId::noteheadCircledDoubleWholeLarge:
    case Ms::SymId::noteheadCircledHalfLarge:
    case Ms::SymId::noteheadCircledWhole:
    case Ms::SymId::noteheadCircledWholeLarge:
        types.emplace(mpe::ArticulationType::CircleNote);
        break;
    case Ms::SymId::noteheadDiamondBlack:
    case Ms::SymId::noteheadDiamondBlackOld:
    case Ms::SymId::noteheadDiamondBlackWide:
    case Ms::SymId::noteheadDiamondDoubleWhole:
    case Ms::SymId::noteheadDiamondHalfWide:
    case Ms::SymId::noteheadDiamondDoubleWholeOld:
    case Ms::SymId::noteheadDiamondWhole:
    case Ms::SymId::noteheadDiamondWholeOld:
    case Ms::SymId::noteheadDiamondHalf:
    case Ms::SymId::noteheadDiamondHalfFilled:
    case Ms::SymId::noteheadDiamondHalfOld:
        types.emplace(mpe::ArticulationType::DiamondNote);
        break;
    case Ms::SymId::noteheadTriangleDownBlack:
    case Ms::SymId::noteheadTriangleDownDoubleWhole:
    case Ms::SymId::noteheadTriangleDownHalf:
    case Ms::SymId::noteheadTriangleDownWhite:
    case Ms::SymId::noteheadTriangleDownWhole:
    case Ms::SymId::noteheadTriangleLeftBlack:
    case Ms::SymId::noteheadTriangleLeftWhite:
    case Ms::SymId::noteheadTriangleRightBlack:
    case Ms::SymId::noteheadTriangleRightWhite:
    case Ms::SymId::noteheadTriangleRoundDownBlack:
    case Ms::SymId::noteheadTriangleRoundDownWhite:
    case Ms::SymId::noteheadTriangleUpBlack:
    case Ms::SymId::noteheadTriangleUpDoubleWhole:
    case Ms::SymId::noteheadTriangleUpHalf:
    case Ms::SymId::noteheadTriangleUpRightBlack:
    case Ms::SymId::noteheadTriangleUpRightWhite:
    case Ms::SymId::noteheadTriangleUpWhite:
    case Ms::SymId::noteheadTriangleUpWhole:
        types.emplace(mpe::ArticulationType::TriangleNote);
        break;
    case Ms::SymId::brassScoop:
        types.emplace(mpe::ArticulationType::Scoop);
        break;
    case Ms::SymId::brassPlop:
        types.emplace(mpe::ArticulationType::Plop);
        break;
    case Ms::SymId::brassFallLipLong:
    case Ms::SymId::brassFallLipMedium:
    case Ms::SymId::brassFallLipShort:
    case Ms::SymId::brassFallRoughLong:
    case Ms::SymId::brassFallRoughMedium:
    case Ms::SymId::brassFallRoughShort:
        types.emplace(mpe::ArticulationType::QuickFall);
        break;
    case Ms::SymId::brassFallSmoothLong:
    case Ms::SymId::brassFallSmoothMedium:
    case Ms::SymId::brassFallSmoothShort:
        types.emplace(mpe::ArticulationType::Fall);
        break;
    case Ms::SymId::brassDoitLong:
    case Ms::SymId::brassDoitMedium:
    case Ms::SymId::brassDoitShort:
        types.emplace(mpe::ArticulationType::Doit);
        break;
    case Ms::SymId::brassBend:
        types.emplace(mpe::ArticulationType::Bend);
        break;
    case Ms::SymId::dynamicCrescendoHairpin:
        types.emplace(mpe::ArticulationType::Crescendo);
        break;
    case Ms::SymId::dynamicDiminuendoHairpin:
        types.emplace(mpe::ArticulationType::Decrescendo);
        break;
    case Ms::SymId::ornamentUpPrall:
        types.emplace(mpe::ArticulationType::UpPrall);
        break;
    case Ms::SymId::ornamentPrallDown:
        types.emplace(mpe::ArticulationType::PrallDown);
        break;
    case Ms::SymId::ornamentPrallUp:
        types.emplace(mpe::ArticulationType::PrallUp);
        break;
    case Ms::SymId::ornamentLinePrall:
        types.emplace(mpe::ArticulationType::LinePrall);
        break;
    case Ms::SymId::ornamentPrallMordent:
        types.emplace(mpe::ArticulationType::PrallMordent);
        break;
    case Ms::SymId::ornamentUpMordent:
        types.emplace(mpe::ArticulationType::UpMordent);
        break;
    case Ms::SymId::ornamentMordent:
        types.emplace(mpe::ArticulationType::LowerMordent);
        break;
    case Ms::SymId::ornamentDownMordent:
        types.emplace(mpe::ArticulationType::DownMordent);
        break;
    case Ms::SymId::ornamentTurn:
    case Ms::SymId::brassJazzTurn:
        types.emplace(mpe::ArticulationType::Turn);
        break;
    case Ms::SymId::ornamentTurnInverted:
    case Ms::SymId::ornamentTurnSlash:
        types.emplace(mpe::ArticulationType::InvertedTurn);
        break;
    case Ms::SymId::ornamentTrill:
        if (articulationSymbol->ornamentStyle() == Ms::OrnamentStyle::DEFAULT) {
            types.emplace(mpe::ArticulationType::Trill);
        } else {
            types.emplace(mpe::ArticulationType::TrillBaroque);
        }
        break;
    case Ms::SymId::ornamentShortTrill:
        types.emplace(mpe::ArticulationType::UpperMordent);
        break;
    case Ms::SymId::ornamentTremblement:
    case Ms::SymId::ornamentTremblementCouperin:
        types.emplace(mpe::ArticulationType::Tremblement);
        break;
    case Ms::SymId::graceNoteAcciaccaturaStemDown:
    case Ms::SymId::graceNoteAcciaccaturaStemUp:
    case Ms::SymId::graceNoteSlashStemDown:
    case Ms::SymId::graceNoteSlashStemUp:
        types.emplace(mpe::ArticulationType::Acciaccatura);
        break;
    case Ms::SymId::graceNoteAppoggiaturaStemDown:
    case Ms::SymId::graceNoteAppoggiaturaStemUp:
        types.emplace(mpe::ArticulationType::PreAppoggiatura);
        break;
    case Ms::SymId::glissandoUp:
    case Ms::SymId::glissandoDown:
        types.emplace(mpe::ArticulationType::DiscreteGlissando);
        break;
    case Ms::SymId::wiggleArpeggiatoDown:
    case Ms::SymId::wiggleArpeggiatoDownArrow:
        types.emplace(mpe::ArticulationType::ArpeggioDown);
        break;
    case Ms::SymId::wiggleArpeggiatoUp:
    case Ms::SymId::wiggleArpeggiatoUpArrow:
        types.emplace(mpe::ArticulationType::ArpeggioUp);
        break;
    case Ms::SymId::wiggleArpeggiatoDownSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightDown);
        break;
    case Ms::SymId::wiggleArpeggiatoUpSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightUp);
        break;
    case Ms::SymId::wiggleVibratoLargeFaster:
    case Ms::SymId::wiggleVibratoLargeFasterStill:
    case Ms::SymId::wiggleVibratoLargeFastest:
    case Ms::SymId::wiggleVibratoLargeSlow:
    case Ms::SymId::wiggleVibratoLargeSlower:
    case Ms::SymId::wiggleVibratoLargeSlowest:
    case Ms::SymId::wiggleVibratoLargestFast:
    case Ms::SymId::wiggleVibratoLargestFaster:
    case Ms::SymId::wiggleVibratoLargestFasterStill:
    case Ms::SymId::wiggleVibratoLargestFastest:
    case Ms::SymId::wiggleVibratoLargestSlow:
    case Ms::SymId::wiggleVibratoLargestSlowest:
    case Ms::SymId::wiggleVibratoMediumFast:
    case Ms::SymId::wiggleVibratoMediumFaster:
    case Ms::SymId::wiggleVibratoMediumFasterStill:
    case Ms::SymId::wiggleVibratoMediumFastest:
    case Ms::SymId::wiggleVibratoMediumSlow:
    case Ms::SymId::wiggleVibratoMediumSlowest:
    case Ms::SymId::wiggleVibratoSmallFast:
    case Ms::SymId::wiggleVibratoSmallFaster:
    case Ms::SymId::wiggleVibratoSmallFasterStill:
    case Ms::SymId::wiggleVibratoSmallFastest:
    case Ms::SymId::wiggleVibratoSmallSlow:
    case Ms::SymId::wiggleVibratoSmallSlower:
    case Ms::SymId::wiggleVibratoSmallSlowest:
    case Ms::SymId::wiggleVibratoSmallestFast:
    case Ms::SymId::wiggleVibratoSmallestFaster:
    case Ms::SymId::wiggleVibratoSmallestFasterStill:
    case Ms::SymId::wiggleVibratoSmallestFastest:
    case Ms::SymId::wiggleVibratoSmallestSlow:
    case Ms::SymId::wiggleVibratoSmallestSlower:
    case Ms::SymId::wiggleVibratoSmallestSlowest:
    case Ms::SymId::wiggleVibratoStart:
    case Ms::SymId::wiggleVibratoLargeFast:
    case Ms::SymId::wiggleVibrato:
        types.emplace(mpe::ArticulationType::Vibrato);
        break;
    case Ms::SymId::wiggleVibratoWide:
        types.emplace(mpe::ArticulationType::WideVibrato);
        break;
    default:
        break;
    }

    for (const auto& type : types) {
        appendArticulationData(mpe::ArticulationMeta(type, ctx.profile->pattern(type), ctx.nominalTimestamp, ctx.nominalDuration), result);
    }
}
