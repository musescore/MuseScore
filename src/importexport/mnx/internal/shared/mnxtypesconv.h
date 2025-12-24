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

#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/tuplet.h"
#include "engraving/types/types.h"

#include "mnxdom.h"

namespace mu::engraving {
struct NoteVal;
}

namespace mu::iex::mnxio {
// MNX vales -> MuseScore values
extern engraving::BarLineType toMuseScoreBarLineType(mnx::BarlineType blt);
extern engraving::BeamMode toMuseScoreBeamMode(int lowestBeamStart);
extern engraving::BracketType toMuseScoreBracketType(mnx::LayoutSymbol lys);
extern engraving::ClefType toMuseScoreClefType(const mnx::part::Clef& mnxClef);
extern engraving::DynamicType toMuseScoreDynamicType(const engraving::String& glyph);
extern engraving::DurationType toMuseScoreDurationType(mnx::NoteValueBase nvb);
extern engraving::TDuration toMuseScoreDuration(mnx::NoteValue nv);
extern engraving::Fraction toMuseScoreFraction(const mnx::FractionValue& fraction);
extern engraving::JumpType toMuseScoreJumpType(mnx::JumpType jt);
extern engraving::Key toMuseScoreKey(int fifths);
extern engraving::LyricsSyllabic toMuseScoreLyricsSyllabic(mnx::LyricLineType llt);
extern engraving::OttavaType toMuseScoreOttavaType(mnx::OttavaAmount ottavaAmount);
extern engraving::Fraction toMuseScoreRTick(const mnx::RhythmicPosition& position);
extern engraving::SlurStyleType toMuseScoreSlurStyleType(mnx::LineType lineType);
extern engraving::TremoloType toMuseScoreTremoloType(int numberOfBeams);
extern engraving::TupletBracketType toMuseScoreTupletBracketType(mnx::AutoYesNo bracketOption);
extern engraving::TupletNumberType toMuseScoreTupletNumberType(mnx::TupletDisplaySetting numberStyle);

// MuseScore -> MuseScore
extern engraving::NoteType duraTypeToGraceNoteType(engraving::DurationType type, bool useLeft);

// pitch conversion
extern engraving::NoteVal toNoteVal(const mnx::sequence::Pitch::Fields& pitch, engraving::Key key, int octaveShift);
} // namespace mu::iex::musx
