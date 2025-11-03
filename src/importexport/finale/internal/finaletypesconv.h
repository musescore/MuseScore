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
#include "engraving/dom/fret.h"
#include "engraving/dom/key.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/tuplet.h"

#include "importfinalelogger.h"

namespace mu::iex::finale {
class FinaleLogger;

extern engraving::ID createPartId(int partNumber);
extern engraving::ID createStaffId(musx::dom::StaffCmper staffId);

extern int createFinaleVoiceId(musx::dom::LayerIndex layerIndex, bool forV2);

extern engraving::DurationType noteTypeToDurationType(musx::dom::NoteType noteType);
extern engraving::TDuration noteInfoToDuration(std::pair<musx::dom::NoteType, unsigned> noteInfo);
extern engraving::NoteType durationTypeToNoteType(engraving::DurationType type, bool after);
extern bool isValidUuid(std::string uuid);
extern engraving::String instrTemplateIdfromUuid(std::string uuid);
extern engraving::BracketType toMuseScoreBracketType(musx::dom::details::Bracket::BracketStyle bracketStyle);
extern engraving::TupletNumberType toMuseScoreTupletNumberType(musx::dom::options::TupletOptions::NumberStyle numberStyle);
extern engraving::Align justifyToAlignment(musx::dom::others::NamePositioning::AlignJustify alignJustify);
extern engraving::AlignH toAlignH(musx::dom::others::HorizontalTextJustification hTextJustify);
extern engraving::AlignH toAlignH(musx::dom::others::MeasureNumberRegion::AlignJustify align);
extern engraving::AlignH toAlignH(musx::dom::others::PageTextAssign::HorizontalAlignment align);
extern engraving::AlignV toAlignV(musx::dom::others::PageTextAssign::VerticalAlignment align);
extern engraving::CourtesyBarlineMode boolToCourtesyBarlineMode(bool useDoubleBarlines);
extern engraving::NoteVal notePropertiesToNoteVal(const musx::dom::Note::NoteProperties& noteProperties, engraving::Key key = engraving::Key::C);
extern engraving::Fraction musxFractionToFraction(const musx::util::Fraction& fraction);
extern engraving::Fraction eduToFraction(musx::dom::Edu edu);
extern engraving::Fraction simpleMusxTimeSigToFraction(const std::pair<musx::util::Fraction, musx::dom::NoteType>& simpleMusxTimeSig, FinaleLoggerPtr& logger);
extern engraving::Key keyFromAlteration(int musxAlteration);
extern engraving::KeyMode keyModeFromDiatonicMode(music_theory::DiatonicMode diatonicMode);
extern engraving::SymId acciSymbolFromAcciAmount(int acciAmount);
extern engraving::StaffGroup staffGroupFromNotationStyle(musx::dom::others::Staff::NotationStyle notationStyle);
extern engraving::String metaTagFromFileInfo(musx::dom::texts::FileInfoText::TextType textType);
extern engraving::String metaTagFromTextComponent(const std::string& component);
extern engraving::ElementType elementTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::OttavaType ottavaTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::HairpinType hairpinTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::SlurStyleType slurStyleTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::GlissandoType glissandoTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::VibratoType vibratoTypeFromSymId(engraving::SymId vibratoSym);
extern engraving::DirectionV directionVFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::LineType lineTypeFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern std::pair<int, int> hookHeightsFromShapeType(musx::dom::others::SmartShape::ShapeType shapeType);
extern engraving::String fontStylePrefixFromElementType(engraving::ElementType elementType);
extern engraving::TremoloType tremoloTypeFromSymId(engraving::SymId sym);
extern engraving::BarLineType toMuseScoreBarLineType(musx::dom::others::Measure::BarlineType blt);
extern engraving::FretDotType toFretDotType(musx::dom::details::FretboardDiagram::Shape shape);
extern engraving::FretMarkerType toFretMarkerType(musx::dom::details::FretboardDiagram::Shape shape);

// unit conversion
extern double doubleFromEvpu(double evpu);
extern engraving::PointF evpuToPointF(double xEvpu, double yEvpu);
extern double doubleFromEfix(double efix);
extern double doubleFromPercent(int percent);
extern double spatiumScaledFontSize(const musx::dom::MusxInstance<musx::dom::FontInfo>& fontInfo);
extern engraving::Spatium absoluteSpatium(double value, engraving::EngravingItem* e);
extern engraving::Spatium absoluteSpatiumFromEvpu(musx::dom::Evpu evpu, engraving::EngravingItem* e);
}
