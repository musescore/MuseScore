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
#include "engraving/dom/chord.h"
#include "engraving/dom/key.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/tuplet.h"

#include "importfinalelogger.h"

namespace mu::iex::finale {
class FinaleLogger;
class FinaleTConv
{
public:
    FinaleTConv() = default;

    static engraving::ID createPartId(int partNumber);
    static engraving::ID createStaffId(musx::dom::StaffCmper staffId);

    static int createFinaleVoiceId(musx::dom::LayerIndex layerIndex, bool forV2);

    static engraving::DurationType noteTypeToDurationType(musx::dom::NoteType noteType);
    static engraving::TDuration noteInfoToDuration(std::pair<musx::dom::NoteType, unsigned> noteInfo);
    static engraving::NoteType durationTypeToNoteType(engraving::DurationType type, bool after);
    static engraving::String instrTemplateIdfromUuid(std::string uuid);
    static engraving::BracketType toMuseScoreBracketType(musx::dom::details::Bracket::BracketStyle bracketStyle);
    static engraving::TupletNumberType toMuseScoreTupletNumberType(musx::dom::options::TupletOptions::NumberStyle numberStyle);
    static engraving::Align justifyToAlignment(musx::dom::others::NamePositioning::AlignJustify alignJustify);
    static engraving::CourtesyBarlineMode boolToCourtesyBarlineMode(bool useDoubleBarlines);
    static engraving::NoteVal notePropertiesToNoteVal(const musx::dom::Note::NoteProperties& noteProperties, engraving::Key key = engraving::Key::C);
    static engraving::Fraction musxFractionToFraction(const musx::util::Fraction& fraction);
    static engraving::Fraction eduToFraction(musx::dom::Edu edu);
    static engraving::Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, FinaleLoggerPtr& logger);
    static engraving::Key keyFromAlteration(int musxAlteration);
    static engraving::KeyMode keyModeFromDiatonicMode(music_theory::DiatonicMode diatonicMode);
    static engraving::SymId acciSymbolFromAcciAmount(int acciAmount);
    static engraving::StaffGroup staffGroupFromNotationStyle(musx::dom::others::Staff::NotationStyle notationStyle);
    static engraving::String metaTagFromFileInfo(musx::dom::texts::FileInfoText::TextType textType);
    static engraving::String metaTagFromTextComponent(const std::string& component);
    static engraving::ElementType elementTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::OttavaType ottavaTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::HairpinType hairpinTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::SlurStyleType slurStyleTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::DirectionV directionVFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::LineType lineTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static std::pair<int, int> hookHeightsFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
    static engraving::String fontStyleSuffixFromCategoryType(musx::dom::others::MarkingCategory::CategoryType categoryType);
    // unit conversion
    static double doubleFromEvpu(double evpu);
    static engraving::PointF evpuToPointF(double xEvpu, double yEvpu);
    static double doubleFromEfix(double efix);
    static double doubleFromPercent(int percent);
    static double spatiumScaledFontSize(const musx::dom::MusxInstance<musx::dom::FontInfo>& fontInfo);
    static engraving::Spatium absoluteSpatium(double value, engraving::EngravingItem* e);
    static engraving::Spatium absoluteSpatiumFromEvpu(musx::dom::Evpu evpu, engraving::EngravingItem* e);
};

}
