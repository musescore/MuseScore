/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "engraving/dom/chord.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/types/types.h"

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

namespace mu::engraving {
struct Interval;
struct NoteVal;
class TDuration;
}

namespace mu::iex::mnxio {
/// How a MuseScore DynamicType is expressed in MNX.
///
/// MNX spells an accent out as accentPrefix + value + accentSuffix + residualValue, so a
/// single MuseScore dynamic type may need up to four MNX properties to describe it.
struct MnxDynamicMapping
{
    std::optional<mnx::DynamicValue> value;         ///< the dynamic, or the initial attack of an accent
    std::optional<mnx::DynamicValue> residualValue; ///< the value remaining after an accent's attack (accents only)
    bool isAccent = false;                          ///< whether MNX models this as a `DynamicAccent`
    mnx::DynamicPrefix accentPrefix = mnx::DynamicPrefix::None; ///< meaningful only when #isAccent
    mnx::DynamicSuffix accentSuffix = mnx::DynamicSuffix::None; ///< meaningful only when #isAccent
};

// MNX values -> MuseScore values
extern engraving::ArticulationAnchor toMuseScoreArticulationAnchor(mnx::Orientation orient);
extern engraving::BarLineType toMuseScoreBarLineType(mnx::BarlineType blt);
extern engraving::BeamMode toMuseScoreBeamMode(int lowestBeamStart);
extern engraving::BracketType toMuseScoreBracketType(mnx::LayoutSymbol lys);
extern engraving::SymId toMuseScoreBreathMarkSym(std::optional<mnx::BreathMarkSymbol> brSym);
extern engraving::ClefType toMuseScoreClefType(const mnx::part::Clef& mnxClef);
extern std::vector<std::string> toMuseScoreDynamicGlyphNames(mnx::DynamicValue value);
extern std::string toMuseScoreDynamicGlyphName(mnx::DynamicPrefix prefix);
extern std::string toMuseScoreDynamicGlyphName(mnx::DynamicSuffix suffix);
extern engraving::DurationType toMuseScoreDurationType(mnx::NoteValueBase nvb);
extern engraving::TDuration toMuseScoreDuration(mnx::NoteValue nv);
extern engraving::SymId toMuseScoreFermataSymId(const mnx::FermataSymbol fermataSymbol);
extern engraving::Fraction toMuseScoreFraction(const mnx::FractionValue& fraction);
extern engraving::JumpType toMuseScoreJumpType(mnx::JumpType jt);
extern engraving::Key toMuseScoreKey(int fifths);
extern engraving::LyricsSyllabic toMuseScoreLyricsSyllabic(mnx::LyricLineType llt);
extern engraving::NoteVal toMuseScoreNoteVal(const mnx::sequence::Pitch::Required& pitch, engraving::Key key, int octaveShift);
extern engraving::OttavaType toMuseScoreOttavaType(mnx::OttavaAmount ottavaAmount);
extern engraving::PreferSharpFlat toMuseScorePreferSharpFlat(int keyFifthsFlipAt);
extern engraving::PlacementV toMuseScorePlacementV(const mnx::Orientation orient, const engraving::EngravingItem* item);
extern engraving::Fraction toMuseScoreRTick(const mnx::RhythmicPosition& position);
extern engraving::SlurStyleType toMuseScoreSlurStyleType(mnx::LineType lineType);
extern engraving::TimeSigType toMuseScoreTimeSigType(std::optional<mnx::TimeSignatureDisplay> display);
extern engraving::TremoloType toMuseScoreTremoloType(int numberOfBeams);
extern engraving::TupletBracketType toMuseScoreTupletBracketType(mnx::AutoYesNo bracketOption);
extern engraving::TupletNumberType toMuseScoreTupletNumberType(mnx::TupletDisplaySetting numberStyle);

// MuseScore values -> MNX values
extern mnx::BarlineType toMnxBarLineType(engraving::BarLineType blt);
extern std::optional<mnx::BreathMarkSymbol> toMnxBreathMarkSym(engraving::SymId sym);
extern std::optional<mnx::part::Clef::Required> toMnxClef(engraving::ClefType clefType);
extern MnxDynamicMapping toMnxDynamicType(engraving::DynamicType type);
/// Recovers an MNX dynamic from the letters that spell it, for dynamics MuseScore renders but
/// has no DynamicType for. Takes arbitrary text rather than pre-validated letters, and returns
/// std::nullopt unless the whole of it is a dynamic spelling MNX can describe.
extern std::optional<MnxDynamicMapping> toMnxDynamicFromLetters(const std::string& dynamicLetters);
/// #toMnxDynamicFromLetters for text set in a music font. Decomposes each glyph into the letters
/// it spells, so ligatures such as dynamicFFF are read as "fff". Returns std::nullopt if any
/// glyph is not one MuseScore spells a dynamic with.
extern std::optional<MnxDynamicMapping> toMnxDynamicFromSymIds(const std::vector<engraving::SymId>& symIds);
/// The inverse of #toMnxDynamicType. Returns std::nullopt when MuseScore has no type that
/// means exactly what the mapping describes, in which case the dynamic is unclassifiable
/// and belongs in DynamicType::OTHER.
extern std::optional<engraving::DynamicType> toMuseScoreDynamicType(const MnxDynamicMapping& mapping);
extern mnx::FermataDuration toMnxFermataDuration(engraving::FermataType fermataType);
extern mnx::FermataSymbol toMnxFermataSymbol(engraving::SymId sym);
extern mnx::FractionValue toMnxFractionValue(const engraving::Fraction& fraction);
extern std::optional<mnx::JumpType> toMnxJumpType(engraving::JumpType jt);
extern int toMnxKeyFifthsFlipValue(engraving::PreferSharpFlat prefer, const engraving::Interval& keyTransposition);
extern mnx::LayoutSymbol toMnxLayoutSymbol(engraving::BracketType bracketType);
extern mnx::LyricLineType toMnxLyricLineType(engraving::LyricsSyllabic ls);
extern mnx::Orientation toMnxOrientation(engraving::ArticulationAnchor anchor);
extern mnx::Orientation toMnxOrientation(engraving::PlacementV placement);
extern mnx::LineType toMnxSlurLineType(engraving::SlurStyleType sst);
extern std::optional<mnx::NoteValue::Required> toMnxNoteValue(const engraving::TDuration& duration);
extern std::optional<mnx::OttavaAmount> toMnxOttavaAmount(engraving::OttavaType ottavaType);
extern std::optional<mnx::sequence::Pitch::Required> toMnxPitch(const engraving::Note* note);
extern std::optional<mnx::TimeSignatureDisplay> toMnxTimeSignatureDisplay(engraving::TimeSigType type);
extern std::optional<mnx::TimeSignatureUnit> toMnxTimeSignatureUnit(int denominator);
extern std::optional<int> toMnxTremoloMarks(engraving::TremoloType tt);
extern mnx::AutoYesNo toMnxTupletBracketType(engraving::TupletBracketType bracketOption);
extern mnx::TupletDisplaySetting toMnxTupletNumberType(engraving::TupletNumberType numberStyle);

// MuseScore -> MuseScore
extern engraving::NoteType duraTypeToGraceNoteType(engraving::DurationType type, bool useLeft);

// utilities
extern std::string makeMnxVoiceIdFromTrack(int mnxPartStaffNum, engraving::track_idx_t curTrackIdx);
} // namespace mu::iex::musx
