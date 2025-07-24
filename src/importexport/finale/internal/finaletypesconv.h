/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include "musx/musx.h"
#include "engraving/types/types.h"
#include "engraving/dom/tuplet.h"

#include "importfinalelogger.h"

namespace mu::iex::finale {
class FinaleLogger;
class FinaleTConv
{
public:
    FinaleTConv() = default;

    static engraving::ID createPartId(int partNumber);
    static engraving::ID createStaffId(musx::dom::InstCmper staffId);

    static int createFinaleVoiceId(musx::dom::LayerIndex layerIndex, bool forV2);

    static engraving::DurationType noteTypeToDurationType(musx::dom::NoteType noteType);
    static engraving::TDuration noteInfoToDuration(std::pair<musx::dom::NoteType, unsigned> noteInfo);
    static engraving::NoteType durationTypeToNoteType(engraving::DurationType type, bool after);
    static engraving::String instrTemplateIdfromUuid(std::string uuid);
    static engraving::BracketType toMuseScoreBracketType(musx::dom::details::StaffGroup::BracketStyle bracketStyle);
    static engraving::TupletNumberType toMuseScoreTupletNumberType(musx::dom::options::TupletOptions::NumberStyle numberStyle);
    static engraving::Align justifyToAlignment(musx::dom::others::NamePositioning::AlignJustify alignJustify);
    static engraving::CourtesyBarlineMode boolToCourtesyBarlineMode(bool useDoubleBarlines);
    static engraving::NoteVal notePropertiesToNoteVal(const musx::dom::Note::NoteProperties& noteProperties, engraving::Key key = engraving::Key::C);
    static engraving::Fraction musxFractionToFraction(const musx::util::Fraction& fraction);
    static engraving::Fraction eduToFraction(musx::dom::Edu edu);
    static engraving::Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, FinaleLoggerPtr& logger);
    static engraving::SymId acciSymbolFromAcciAmount(int acciAmount);
    static engraving::StaffGroup staffGroupFromNotationStyle(musx::dom::others::Staff::NotationStyle notationStyle);
    // unit conversion
    static double doubleFromEvpu(double evpu);
    static engraving::PointF evpuToPointF(double xEvpu, double yEvpu);
    static double doubleFromEfix(double efix);
    static double doubleFromPercent(int percent);
    static double spatiumScaledFontSize(const std::shared_ptr<const musx::dom::FontInfo>& fontInfo);
    static engraving::Spatium absoluteSpatium(double value, engraving::EngravingItem* e);
    static engraving::Spatium absoluteSpatiumFromEvpu(musx::dom::Evpu evpu, engraving::EngravingItem* e);
};

}
