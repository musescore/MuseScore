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

#include "symbolsmetaparser.h"

#include "dom/articulation.h"

using namespace mu::engraving;
using namespace muse;

static mpe::ArticulationType textTypeToArticulationType(ArticulationTextType textType)
{
    switch (textType) {
    case ArticulationTextType::NO_TEXT: return mpe::ArticulationType::Standard;
    case ArticulationTextType::SLAP: return mpe::ArticulationType::Slap;
    case ArticulationTextType::POP: return mpe::ArticulationType::Pop;
    case ArticulationTextType::LV: return mpe::ArticulationType::LaissezVibrer;
    case ArticulationTextType::R: return mpe::ArticulationType::Ring;
    case ArticulationTextType::TD: return mpe::ArticulationType::ThumbDamp;
    case ArticulationTextType::BD: return mpe::ArticulationType::BrushDamp;
    case ArticulationTextType::RT: return mpe::ArticulationType::RingTouch;
    case ArticulationTextType::PL: return mpe::ArticulationType::Pluck;
    case ArticulationTextType::SB: return mpe::ArticulationType::SingingBell;
    case ArticulationTextType::VIB: return mpe::ArticulationType::SingingVibrate;
    }

    UNREACHABLE;
    return mpe::ArticulationType::Undefined;
}

mpe::ArticulationTypeSet SymbolsMetaParser::symbolToArticulations(SymId symId, OrnamentStyle ornamentStyle)
{
    mpe::ArticulationTypeSet types;

    switch (symId) {
    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::Accent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articLaissezVibrerAbove:
    case SymId::articLaissezVibrerBelow:
        types.emplace(mpe::ArticulationType::LaissezVibrer);
        break;
    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        break;
    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        types.emplace(mpe::ArticulationType::Marcato);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case SymId::articSoftAccentAbove:
    case SymId::articSoftAccentBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        break;
    case SymId::articSoftAccentStaccatoAbove:
    case SymId::articSoftAccentStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articSoftAccentTenutoAbove:
    case SymId::articSoftAccentTenutoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case SymId::articSoftAccentTenutoStaccatoAbove:
    case SymId::articSoftAccentTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::SoftAccent);
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articStaccatissimoStrokeAbove:
    case SymId::articStaccatissimoStrokeBelow:
    case SymId::articStaccatissimoWedgeAbove:
    case SymId::articStaccatissimoWedgeBelow:
        types.emplace(mpe::ArticulationType::Staccatissimo);
        break;
    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        break;
    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Staccato);
        break;
    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        types.emplace(mpe::ArticulationType::Tenuto);
        types.emplace(mpe::ArticulationType::Accent);
        break;
    case SymId::dynamicSforzando:
    case SymId::dynamicSforzando1:
    case SymId::dynamicSforzandoPianissimo:
    case SymId::dynamicSforzandoPiano:
    case SymId::dynamicSforzato:
    case SymId::dynamicSforzatoFF:
    case SymId::dynamicSforzatoPiano:
        types.emplace(mpe::ArticulationType::Subito);
        break;
    case SymId::guitarFadeIn:
        types.emplace(mpe::ArticulationType::FadeIn);
        break;
    case SymId::guitarVolumeSwell:
        types.emplace(mpe::ArticulationType::VolumeSwell);
        break;
    case SymId::guitarFadeOut:
        types.emplace(mpe::ArticulationType::FadeOut);
        break;
    case SymId::stringsHalfHarmonic:
    case SymId::stringsHarmonic:
        types.emplace(mpe::ArticulationType::Harmonic);
        break;
    case SymId::stringsMuteOn:
    case SymId::elecMute:
    case SymId::brassMuteHalfClosed:
    case SymId::brassMuteClosed:
    case SymId::brassHarmonMuteStemHalfRight:
    case SymId::brassHarmonMuteStemHalfLeft:
    case SymId::brassHarmonMuteClosed:
        types.emplace(mpe::ArticulationType::Mute);
        break;
    case SymId::brassHarmonMuteStemOpen:
    case SymId::brassMuteOpen:
    case SymId::guitarHalfOpenPedal:
    case SymId::guitarOpenPedal:
    case SymId::vocalMouthOpen:
    case SymId::vocalMouthSlightlyOpen:
    case SymId::vocalMouthWideOpen:
    case SymId::windOpenHole:
    case SymId::elecUnmute:
    case SymId::stringsMuteOff:
        types.emplace(mpe::ArticulationType::Open);
        break;
    case SymId::pluckedLeftHandPizzicato:
        types.emplace(mpe::ArticulationType::Pizzicato);
        break;
    case SymId::pluckedSnapPizzicatoAbove:
    case SymId::pluckedSnapPizzicatoBelow:
        types.emplace(mpe::ArticulationType::SnapPizzicato);
        break;
    case SymId::stringsUpBow:
        types.emplace(mpe::ArticulationType::UpBow);
        break;
    case SymId::stringsDownBow:
        types.emplace(mpe::ArticulationType::DownBow);
        break;
    case SymId::stringsJeteAbove:
    case SymId::stringsJeteBelow:
        types.emplace(mpe::ArticulationType::Jete);
        break;
    case SymId::noteheadXWhole:
    case SymId::noteheadXBlack:
    case SymId::noteheadXDoubleWhole:
    case SymId::noteheadWholeWithX:
    case SymId::noteheadVoidWithX:
        types.emplace(mpe::ArticulationType::CrossNote);
        break;
    case SymId::noteheadSlashX:
        types.emplace(mpe::ArticulationType::CrossLargeNote);
        break;
    case SymId::noteheadXOrnate:
    case SymId::noteheadXOrnateEllipse:
        types.emplace(mpe::ArticulationType::CrossOrnateNote);
        break;
    case SymId::noteheadCircleSlash:
    case SymId::noteheadCircledBlack:
    case SymId::noteheadCircledHalf:
    case SymId::noteheadCircledBlackLarge:
    case SymId::noteheadCircledDoubleWhole:
    case SymId::noteheadCircledDoubleWholeLarge:
    case SymId::noteheadCircledHalfLarge:
    case SymId::noteheadCircledWhole:
    case SymId::noteheadCircledWholeLarge:
        types.emplace(mpe::ArticulationType::CircleNote);
        break;
    case SymId::noteheadCircleX:
    case SymId::noteheadCircleXDoubleWhole:
    case SymId::noteheadCircleXHalf:
    case SymId::noteheadCircleXWhole:
        types.emplace(mpe::ArticulationType::CircleCrossNote);
        break;
    case SymId::noteheadRoundWhiteWithDot:
    case SymId::noteheadRoundWhiteWithDotLarge:
        types.emplace(mpe::ArticulationType::CircleDotNote);
        break;
    case SymId::noteheadDiamondBlack:
    case SymId::noteheadDiamondBlackOld:
    case SymId::noteheadDiamondBlackWide:
    case SymId::noteheadDiamondDoubleWhole:
    case SymId::noteheadDiamondHalfWide:
    case SymId::noteheadDiamondDoubleWholeOld:
    case SymId::noteheadDiamondWhole:
    case SymId::noteheadDiamondWholeOld:
    case SymId::noteheadDiamondHalf:
    case SymId::noteheadDiamondHalfFilled:
    case SymId::noteheadDiamondHalfOld:
        types.emplace(mpe::ArticulationType::DiamondNote);
        break;
    case SymId::noteShapeMoonBlack:
    case SymId::noteShapeMoonDoubleWhole:
    case SymId::noteShapeMoonWhite:
        types.emplace(mpe::ArticulationType::MoonNote);
        break;
    case SymId::noteheadTriangleLeftBlack:
    case SymId::noteheadTriangleLeftWhite:
        types.emplace(mpe::ArticulationType::TriangleLeftNote);
        break;
    case SymId::noteheadTriangleRightBlack:
    case SymId::noteheadTriangleRightWhite:
    case SymId::noteShapeTriangleRightWhite:
    case SymId::noteShapeTriangleRightBlack:
    case SymId::noteShapeTriangleRightDoubleWhole:
        types.emplace(mpe::ArticulationType::TriangleRightNote);
        break;
    case SymId::noteheadTriangleDownBlack:
    case SymId::noteheadTriangleDownDoubleWhole:
    case SymId::noteheadTriangleDownHalf:
    case SymId::noteheadTriangleDownWhite:
    case SymId::noteheadTriangleDownWhole:
        types.emplace(mpe::ArticulationType::TriangleDownNote);
        break;
    case SymId::noteheadTriangleUpBlack:
    case SymId::noteheadTriangleUpDoubleWhole:
    case SymId::noteheadTriangleUpHalf:
    case SymId::noteheadTriangleUpRightBlack:
    case SymId::noteheadTriangleUpRightWhite:
    case SymId::noteheadTriangleUpWhite:
    case SymId::noteheadTriangleUpWhole:
    case SymId::noteShapeTriangleUpBlack:
    case SymId::noteShapeTriangleUpWhite:
    case SymId::noteShapeTriangleUpDoubleWhole:
        types.emplace(mpe::ArticulationType::TriangleUpNote);
        break;
    case SymId::noteheadTriangleRoundDownBlack:
    case SymId::noteheadTriangleRoundDownWhite:
    case SymId::noteShapeTriangleRoundBlack:
    case SymId::noteShapeTriangleRoundWhite:
    case SymId::noteShapeTriangleRoundDoubleWhole:
        types.emplace(mpe::ArticulationType::TriangleRoundDownNote);
        break;
    case SymId::noteheadPlusBlack:
    case SymId::noteheadPlusDoubleWhole:
    case SymId::noteheadPlusHalf:
    case SymId::noteheadPlusWhole:
        types.emplace(mpe::ArticulationType::PlusNote);
        break;
    case SymId::noteheadSlashWhiteWhole:
    case SymId::noteheadSlashWhiteHalf:
    case SymId::noteheadSlashHorizontalEnds:
    case SymId::noteheadSlashWhiteDoubleWhole:
        types.emplace(mpe::ArticulationType::SlashNote);
        break;
    case SymId::noteShapeSquareBlack:
    case SymId::noteShapeSquareWhite:
    case SymId::noteShapeSquareDoubleWhole:
        types.emplace(mpe::ArticulationType::SquareNote);
        break;
    case SymId::noteheadSlashedWhole1:
    case SymId::noteheadSlashedHalf1:
    case SymId::noteheadSlashedBlack1:
    case SymId::noteheadSlashedDoubleWhole1:
        types.emplace(mpe::ArticulationType::SlashedForwardsNote);
        break;
    case SymId::noteheadSlashedWhole2:
    case SymId::noteheadSlashedHalf2:
    case SymId::noteheadSlashedBlack2:
    case SymId::noteheadSlashedDoubleWhole2:
        types.emplace(mpe::ArticulationType::SlashedBackwardsNote);
        break;
    case SymId::brassScoop:
        types.emplace(mpe::ArticulationType::Scoop);
        break;
    case SymId::brassPlop:
        types.emplace(mpe::ArticulationType::Plop);
        break;
    case SymId::brassFallLipLong:
    case SymId::brassFallLipMedium:
    case SymId::brassFallLipShort:
    case SymId::brassFallRoughLong:
    case SymId::brassFallRoughMedium:
    case SymId::brassFallRoughShort:
        types.emplace(mpe::ArticulationType::QuickFall);
        break;
    case SymId::brassFallSmoothLong:
    case SymId::brassFallSmoothMedium:
    case SymId::brassFallSmoothShort:
        types.emplace(mpe::ArticulationType::Fall);
        break;
    case SymId::brassDoitLong:
    case SymId::brassDoitMedium:
    case SymId::brassDoitShort:
        types.emplace(mpe::ArticulationType::Doit);
        break;
    case SymId::brassBend:
        types.emplace(mpe::ArticulationType::BrassBend);
        break;
    case SymId::dynamicCrescendoHairpin:
        types.emplace(mpe::ArticulationType::Crescendo);
        break;
    case SymId::dynamicDiminuendoHairpin:
        types.emplace(mpe::ArticulationType::Diminuendo);
        break;
    case SymId::ornamentUpPrall:
        types.emplace(mpe::ArticulationType::UpPrall);
        break;
    case SymId::ornamentPrallDown:
        types.emplace(mpe::ArticulationType::PrallDown);
        break;
    case SymId::ornamentPrallUp:
        types.emplace(mpe::ArticulationType::PrallUp);
        break;
    case SymId::ornamentLinePrall:
        types.emplace(mpe::ArticulationType::LinePrall);
        break;
    case SymId::ornamentPrallMordent:
        types.emplace(mpe::ArticulationType::PrallMordent);
        break;
    case SymId::ornamentUpMordent:
        types.emplace(mpe::ArticulationType::UpMordent);
        break;
    case SymId::ornamentMordent:
    case SymId::ornamentPinceCouperin:
        types.emplace(mpe::ArticulationType::LowerMordent);
        break;
    case SymId::ornamentDownMordent:
        types.emplace(mpe::ArticulationType::DownMordent);
        break;
    case SymId::ornamentTurn:
    case SymId::ornamentTurnUp:
    case SymId::ornamentHaydn:
    case SymId::brassJazzTurn:
        types.emplace(mpe::ArticulationType::Turn);
        break;
    case SymId::ornamentTurnInverted:
    case SymId::ornamentTurnUpS:
    case SymId::ornamentTurnSlash:
        types.emplace(mpe::ArticulationType::InvertedTurn);
        break;
    case SymId::ornamentTrill:
    case SymId::ornamentShake3:
    case SymId::ornamentShakeMuffat1:
        if (ornamentStyle == OrnamentStyle::DEFAULT) {
            types.emplace(mpe::ArticulationType::Trill);
        } else {
            types.emplace(mpe::ArticulationType::TrillBaroque);
        }
        break;
    case SymId::ornamentShortTrill:
        if (ornamentStyle == OrnamentStyle::DEFAULT) {
            types.emplace(mpe::ArticulationType::UpperMordent);
        } else {
            types.emplace(mpe::ArticulationType::UpperMordentBaroque);
        }
        break;
    case SymId::ornamentTremblement:
    case SymId::ornamentTremblementCouperin:
        types.emplace(mpe::ArticulationType::Tremblement);
        break;
    case SymId::graceNoteAcciaccaturaStemDown:
    case SymId::graceNoteAcciaccaturaStemUp:
    case SymId::graceNoteSlashStemDown:
    case SymId::graceNoteSlashStemUp:
        types.emplace(mpe::ArticulationType::Acciaccatura);
        break;
    case SymId::graceNoteAppoggiaturaStemDown:
    case SymId::graceNoteAppoggiaturaStemUp:
        types.emplace(mpe::ArticulationType::PreAppoggiatura);
        break;
    case SymId::glissandoUp:
    case SymId::glissandoDown:
        types.emplace(mpe::ArticulationType::DiscreteGlissando);
        break;
    case SymId::wiggleArpeggiatoDown:
    case SymId::wiggleArpeggiatoDownArrow:
        types.emplace(mpe::ArticulationType::ArpeggioDown);
        break;
    case SymId::wiggleArpeggiatoUp:
    case SymId::wiggleArpeggiatoUpArrow:
        types.emplace(mpe::ArticulationType::ArpeggioUp);
        break;
    case SymId::wiggleArpeggiatoDownSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightDown);
        break;
    case SymId::wiggleArpeggiatoUpSwash:
        types.emplace(mpe::ArticulationType::ArpeggioStraightUp);
        break;
    case SymId::wiggleVibratoLargeFaster:
    case SymId::wiggleVibratoLargeFasterStill:
    case SymId::wiggleVibratoLargeFastest:
    case SymId::wiggleVibratoLargeSlow:
    case SymId::wiggleVibratoLargeSlower:
    case SymId::wiggleVibratoLargeSlowest:
    case SymId::wiggleVibratoLargestFast:
    case SymId::wiggleVibratoLargestFaster:
    case SymId::wiggleVibratoLargestFasterStill:
    case SymId::wiggleVibratoLargestFastest:
    case SymId::wiggleVibratoLargestSlow:
    case SymId::wiggleVibratoLargestSlowest:
    case SymId::wiggleVibratoMediumFast:
    case SymId::wiggleVibratoMediumFaster:
    case SymId::wiggleVibratoMediumFasterStill:
    case SymId::wiggleVibratoMediumFastest:
    case SymId::wiggleVibratoMediumSlow:
    case SymId::wiggleVibratoMediumSlowest:
    case SymId::wiggleVibratoSmallFast:
    case SymId::wiggleVibratoSmallFaster:
    case SymId::wiggleVibratoSmallFasterStill:
    case SymId::wiggleVibratoSmallFastest:
    case SymId::wiggleVibratoSmallSlow:
    case SymId::wiggleVibratoSmallSlower:
    case SymId::wiggleVibratoSmallSlowest:
    case SymId::wiggleVibratoSmallestFast:
    case SymId::wiggleVibratoSmallestFaster:
    case SymId::wiggleVibratoSmallestFasterStill:
    case SymId::wiggleVibratoSmallestFastest:
    case SymId::wiggleVibratoSmallestSlow:
    case SymId::wiggleVibratoSmallestSlower:
    case SymId::wiggleVibratoSmallestSlowest:
    case SymId::wiggleVibratoStart:
    case SymId::wiggleVibratoLargeFast:
    case SymId::wiggleVibrato:
        types.emplace(mpe::ArticulationType::Vibrato);
        break;
    case SymId::wiggleVibratoWide:
        types.emplace(mpe::ArticulationType::WideVibrato);
        break;
    case SymId::handbellsMalletBellOnTable:
        types.emplace(mpe::ArticulationType::MalletBellOnTable);
        break;
    case SymId::handbellsMalletBellSuspended:
        types.emplace(mpe::ArticulationType::MalletBellSuspended);
        break;
    case SymId::handbellsMalletLft:
        types.emplace(mpe::ArticulationType::MalletLift);
        break;
    case SymId::handbellsPluckLift:
        types.emplace(mpe::ArticulationType::PluckLift);
        break;
    case SymId::handbellsGyro:
        types.emplace(mpe::ArticulationType::Gyro);
        break;
    case SymId::handbellsMartellato:
        types.emplace(mpe::ArticulationType::Martellato);
        break;
    case SymId::handbellsMartellatoLift:
        types.emplace(mpe::ArticulationType::MartellatoLift);
        break;
    case SymId::handbellsHandMartellato:
        types.emplace(mpe::ArticulationType::HandMartellato);
        break;
    case SymId::handbellsMutedMartellato:
        types.emplace(mpe::ArticulationType::MutedMartellato);
        break;
    default:
        break;
    }

    return types;
}

void SymbolsMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->isArticulationFamily()) {
        return;
    }

    const Articulation* articulationSymbol = toArticulation(item);
    if (!articulationSymbol->playArticulation()) {
        return;
    }

    mpe::ArticulationTypeSet types;

    if (articulationSymbol->textType() != ArticulationTextType::NO_TEXT) {
        types.insert(textTypeToArticulationType(articulationSymbol->textType()));
    } else {
        types = symbolToArticulations(articulationSymbol->symId(), articulationSymbol->ornamentStyle());
    }

    for (mpe::ArticulationType type : types) {
        const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type, mpe::ArticulationType::Standard);
        if (pattern.empty()) {
            continue;
        }

        appendArticulationData(mpe::ArticulationMeta(type, pattern, ctx.nominalTimestamp, ctx.nominalDuration), result);
    }
}
