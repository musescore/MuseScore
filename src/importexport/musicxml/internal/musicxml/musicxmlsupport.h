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

#include "engraving/types/fraction.h"
#include "engraving/dom/note.h"
#include "engraving/dom/fret.h"

namespace muse {
class XmlStreamReader;
}

namespace mu::engraving {
//---------------------------------------------------------
//   MusicXMLSupport -- MusicXML import support functions
//---------------------------------------------------------

class MusicXMLSupport
{
public:
    static int stringToInt(const String& s, bool* ok);
    static Fraction noteTypeToFraction(const String& type);
    static Fraction calculateFraction(const String& type, int dots, int normalNotes, int actualNotes);
};

extern String accSymId2MusicXMLString(const SymId id);
extern String accSymId2SmuflMusicXMLString(const SymId id);
extern String accidentalType2MusicXMLString(const AccidentalType type);
extern String accidentalType2SmuflMusicXMLString(const AccidentalType type);
extern AccidentalType musicXMLString2accidentalType(const String mxmlName, const String smufl);
extern String musicXMLAccidentalTextToChar(const String mxmlName);
extern SymId musicXMLString2accSymId(const String mxmlName, const String smufl = {});
extern AccidentalType microtonalGuess(double val);
extern bool isLaissezVibrer(const SymId id);
extern const Articulation* findLaissezVibrer(const Chord* chord);
extern String errorStringWithLocation(int line, int col, const String& error);
extern String checkAtEndElement(const muse::XmlStreamReader& e, const String& expName);
} // namespace Ms
