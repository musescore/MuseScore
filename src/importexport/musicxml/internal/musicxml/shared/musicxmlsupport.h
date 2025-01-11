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

#include "framework/global/types/string.h"
#include "serialization/xmlstreamreader.h"

namespace mu::engraving {
enum class AccidentalType : unsigned char;
class Articulation;
class Chord;
class Fraction;
enum class SymId;
}

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   MusicXmlSupport -- MusicXML import support functions
//---------------------------------------------------------

class MusicXmlSupport
{
public:
    static int stringToInt(const muse::String& s, bool* ok);
    static engraving::Fraction noteTypeToFraction(const muse::String& type);
    static engraving::Fraction calculateFraction(const muse::String& type, int dots, int normalNotes, int actualNotes);
};

extern muse::String accSymId2MusicXmlString(const engraving::SymId id);
extern muse::String accSymId2SmuflMusicXmlString(const engraving::SymId id);
extern muse::String accidentalType2MusicXmlString(const engraving::AccidentalType type);
extern muse::String accidentalType2SmuflMusicXmlString(const engraving::AccidentalType type);
extern engraving::AccidentalType musicXmlString2accidentalType(const muse::String mxmlName, const muse::String smufl);
extern muse::String musicXmlAccidentalTextToChar(const muse::String mxmlName);
extern engraving::SymId musicXmlString2accSymId(const muse::String mxmlName, const muse::String smufl = {});
extern engraving::AccidentalType microtonalGuess(double val);
extern bool isLaissezVibrer(const engraving::SymId id);
extern muse::String errorStringWithLocation(int line, int col, const muse::String& error);
extern muse::String checkAtEndElement(const muse::XmlStreamReader& e, const muse::String& expName);
} // namespace Ms
