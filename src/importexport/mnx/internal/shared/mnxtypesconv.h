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
#pragma once

#include <optional>

#include "engraving/dom/chord.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/tuplet.h"
#include "engraving/types/types.h"

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

namespace mu::engraving {
struct NoteVal;
class TDuration;
}

namespace mu::iex::mnxio {
// MNX values -> MuseScore values
extern engraving::BarLineType toMuseScoreBarLineType(mnx::BarlineType blt);
extern engraving::BeamMode toMuseScoreBeamMode(int lowestBeamStart);
extern engraving::BracketType toMuseScoreBracketType(mnx::LayoutSymbol lys);
extern engraving::SymId toMuseScoreBreathMarkSym(std::optional<mnx::BreathMarkSymbol> brSym);
extern engraving::ClefType toMuseScoreClefType(const mnx::part::Clef& mnxClef);
extern engraving::DynamicType toMuseScoreDynamicType(const engraving::String& glyph);
extern engraving::DurationType toMuseScoreDurationType(mnx::NoteValueBase nvb);
extern engraving::TDuration toMuseScoreDuration(mnx::NoteValue nv);
extern engraving::Fraction toMuseScoreFraction(const mnx::FractionValue& fraction);
extern engraving::JumpType toMuseScoreJumpType(mnx::JumpType jt);
extern engraving::Key toMuseScoreKey(int fifths);
extern engraving::LyricsSyllabic toMuseScoreLyricsSyllabic(mnx::LyricLineType llt);
extern engraving::NoteVal toMuseScoreNoteVal(const mnx::sequence::Pitch::Required& pitch, engraving::Key key, int octaveShift);
extern engraving::OttavaType toMuseScoreOttavaType(mnx::OttavaAmount ottavaAmount);
extern engraving::PreferSharpFlat toMuseScorePreferSharpFlat(int keyFifthsFlipAt);
extern engraving::Fraction toMuseScoreRTick(const mnx::RhythmicPosition& position);
extern engraving::SlurStyleType toMuseScoreSlurStyleType(mnx::LineType lineType);
extern engraving::TremoloType toMuseScoreTremoloType(int numberOfBeams);
extern engraving::TupletBracketType toMuseScoreTupletBracketType(mnx::AutoYesNo bracketOption);
extern engraving::TupletNumberType toMuseScoreTupletNumberType(mnx::TupletDisplaySetting numberStyle);

// MuseScore values -> MNX values
extern mnx::BarlineType toMnxBarLineType(engraving::BarLineType blt);
extern std::optional<mnx::BreathMarkSymbol> toMnxBreathMarkSym(engraving::SymId sym);
extern std::optional<mnx::part::Clef::Required> toMnxClef(engraving::ClefType clefType);
extern mnx::FractionValue toMnxFractionValue(const engraving::Fraction& fraction);
extern std::optional<mnx::JumpType> toMnxJumpType(engraving::JumpType jt);
extern int toMnxKeyFifthsFlipValue(engraving::PreferSharpFlat prefer, const engraving::Interval& keyTransposition);
extern mnx::LayoutSymbol toMnxLayoutSymbol(engraving::BracketType bracketType);
extern mnx::LyricLineType toMnxLyricLineType(engraving::LyricsSyllabic ls);
extern mnx::LineType toMnxSlurLineType(engraving::SlurStyleType sst);
extern std::optional<mnx::NoteValue::Required> toMnxNoteValue(const engraving::TDuration& duration);
extern std::optional<mnx::OttavaAmount> toMnxOttavaAmount(engraving::OttavaType ottavaType);
extern std::optional<mnx::sequence::Pitch::Required> toMnxPitch(const engraving::Note* note);
extern std::optional<mnx::TimeSignatureUnit> toMnxTimeSignatureUnit(int denominator);
extern std::optional<int> toMnxTremoloMarks(engraving::TremoloType tt);
extern mnx::AutoYesNo toMnxTupletBracketType(engraving::TupletBracketType bracketOption);
extern mnx::TupletDisplaySetting toMnxTupletNumberType(engraving::TupletNumberType numberStyle);

// MuseScore -> MuseScore
extern engraving::NoteType duraTypeToGraceNoteType(engraving::DurationType type, bool useLeft);

// utilities
extern std::string makeMnxVoiceIdFromTrack(int mnxPartStaffNum, engraving::track_idx_t curTrackIdx);
} // namespace mu::iex::musx
