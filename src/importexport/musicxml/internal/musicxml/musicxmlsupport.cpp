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

/**
 MusicXML support.
 */

#include "global/serialization/xmlstreamreader.h"

#include "translation.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "types/symnames.h"

#include "musicxmlsupport.h"

#include "log.h"

using AccidentalType = mu::engraving::AccidentalType;
using SymId = mu::engraving::SymId;
const static std::map<mu::String, AccidentalType> smuflAccidentalTypes {
    { u"accidentalDoubleFlatOneArrowDown",                AccidentalType::DOUBLE_FLAT_ONE_ARROW_DOWN },
    { u"accidentalFlatOneArrowDown",                      AccidentalType::FLAT_ONE_ARROW_DOWN },
    { u"accidentalNaturalOneArrowDown",                   AccidentalType::NATURAL_ONE_ARROW_DOWN },
    { u"accidentalSharpOneArrowDown",                     AccidentalType::SHARP_ONE_ARROW_DOWN },
    { u"accidentalDoubleSharpOneArrowDown",               AccidentalType::DOUBLE_SHARP_ONE_ARROW_DOWN },
    { u"accidentalDoubleFlatOneArrowUp",                  AccidentalType::DOUBLE_FLAT_ONE_ARROW_UP },
    { u"accidentalFlatOneArrowUp",                        AccidentalType::FLAT_ONE_ARROW_UP },
    { u"accidentalNaturalOneArrowUp",                     AccidentalType::NATURAL_ONE_ARROW_UP },
    { u"accidentalSharpOneArrowUp",                       AccidentalType::SHARP_ONE_ARROW_UP },
    { u"accidentalDoubleSharpOneArrowUp",                 AccidentalType::DOUBLE_SHARP_ONE_ARROW_UP },
    { u"accidentalDoubleFlatTwoArrowsDown",               AccidentalType::DOUBLE_FLAT_TWO_ARROWS_DOWN },
    { u"accidentalFlatTwoArrowsDown",                     AccidentalType::FLAT_TWO_ARROWS_DOWN },
    { u"accidentalNaturalTwoArrowsDown",                  AccidentalType::NATURAL_TWO_ARROWS_DOWN },
    { u"accidentalSharpTwoArrowsDown",                    AccidentalType::SHARP_TWO_ARROWS_DOWN },
    { u"accidentalDoubleSharpTwoArrowsDown",              AccidentalType::DOUBLE_SHARP_TWO_ARROWS_DOWN },
    { u"accidentalDoubleFlatTwoArrowsUp",                 AccidentalType::DOUBLE_FLAT_TWO_ARROWS_UP },
    { u"accidentalFlatTwoArrowsUp",                       AccidentalType::FLAT_TWO_ARROWS_UP },
    { u"accidentalNaturalTwoArrowsUp",                    AccidentalType::NATURAL_TWO_ARROWS_UP },
    { u"accidentalSharpTwoArrowsUp",                      AccidentalType::SHARP_TWO_ARROWS_UP },
    { u"accidentalDoubleSharpTwoArrowsUp",                AccidentalType::DOUBLE_SHARP_TWO_ARROWS_UP },
    { u"accidentalDoubleFlatThreeArrowsDown",             AccidentalType::DOUBLE_FLAT_THREE_ARROWS_DOWN },
    { u"accidentalFlatThreeArrowsDown",                   AccidentalType::FLAT_THREE_ARROWS_DOWN },
    { u"accidentalNaturalThreeArrowsDown",                AccidentalType::NATURAL_THREE_ARROWS_DOWN },
    { u"accidentalSharpThreeArrowsDown",                  AccidentalType::SHARP_THREE_ARROWS_DOWN },
    { u"accidentalDoubleSharpThreeArrowsDown",            AccidentalType::DOUBLE_SHARP_THREE_ARROWS_DOWN },
    { u"accidentalDoubleFlatThreeArrowsUp",               AccidentalType::DOUBLE_FLAT_THREE_ARROWS_UP },
    { u"accidentalFlatThreeArrowsUp",                     AccidentalType::FLAT_THREE_ARROWS_UP },
    { u"accidentalNaturalThreeArrowsUp",                  AccidentalType::NATURAL_THREE_ARROWS_UP },
    { u"accidentalSharpThreeArrowsUp",                    AccidentalType::SHARP_THREE_ARROWS_UP },
    { u"accidentalDoubleSharpThreeArrowsUp",              AccidentalType::DOUBLE_SHARP_THREE_ARROWS_UP },
    { u"accidentalLowerOneSeptimalComma",                 AccidentalType::LOWER_ONE_SEPTIMAL_COMMA },
    { u"accidentalRaiseOneSeptimalComma",                 AccidentalType::RAISE_ONE_SEPTIMAL_COMMA },
    { u"accidentalLowerTwoSeptimalCommas",                AccidentalType::LOWER_TWO_SEPTIMAL_COMMAS },
    { u"accidentalRaiseTwoSeptimalCommas",                AccidentalType::RAISE_TWO_SEPTIMAL_COMMAS },
    { u"accidentalLowerOneUndecimalQuartertone",          AccidentalType::LOWER_ONE_UNDECIMAL_QUARTERTONE },
    { u"accidentalRaiseOneUndecimalQuartertone",          AccidentalType::RAISE_ONE_UNDECIMAL_QUARTERTONE },
    { u"accidentalLowerOneTridecimalQuartertone",         AccidentalType::LOWER_ONE_TRIDECIMAL_QUARTERTONE },
    { u"accidentalRaiseOneTridecimalQuartertone",         AccidentalType::RAISE_ONE_TRIDECIMAL_QUARTERTONE },
    { u"accidentalDoubleFlatEqualTempered",               AccidentalType::DOUBLE_FLAT_EQUAL_TEMPERED },
    { u"accidentalFlatEqualTempered",                     AccidentalType::FLAT_EQUAL_TEMPERED },
    { u"accidentalNaturalEqualTempered",                  AccidentalType::NATURAL_EQUAL_TEMPERED },
    { u"accidentalSharpEqualTempered",                    AccidentalType::SHARP_EQUAL_TEMPERED },
    { u"accidentalDoubleSharpEqualTempered",              AccidentalType::DOUBLE_SHARP_EQUAL_TEMPERED },
    { u"accidentalQuarterFlatEqualTempered",              AccidentalType::QUARTER_FLAT_EQUAL_TEMPERED },
    { u"accidentalQuarterSharpEqualTempered",             AccidentalType::QUARTER_SHARP_EQUAL_TEMPERED }
};

namespace mu::engraving {
NoteList::NoteList()
{
    _staffNoteLists.reserve(MAX_STAVES);
    for (int i = 0; i < MAX_STAVES; ++i) {
        _staffNoteLists.push_back(StartStopList());
    }
}

void NoteList::addNote(const int startTick, const int endTick, const size_t staff)
{
    if (staff < _staffNoteLists.size()) {
        _staffNoteLists[staff].push_back(StartStop(startTick, endTick));
    }
}

void NoteList::dump(const int& voice) const
{
    // dump contents
    for (int i = 0; i < MAX_STAVES; ++i) {
        printf("voice %d staff %d:", voice, i);
        for (size_t j = 0; j < _staffNoteLists.at(i).size(); ++j) {
            printf(" %d-%d", _staffNoteLists.at(i).at(j).first, _staffNoteLists.at(i).at(j).second);
        }
        printf("\n");
    }
    // show overlap
    printf("overlap voice %d:", voice);
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            stavesOverlap(i, j);
        }
    }
    printf("\n");
}

/**
 Determine if notes n1 and n2 overlap.
 This is NOT the case if
 - n1 starts when or after n2 stops
 - or n2 starts when or after n1 stops
 */

static bool notesOverlap(const StartStop& n1, const StartStop& n2)
{
    return !(n1.first >= n2.second || n1.second <= n2.first);
}

/**
 Determine if any note in staff1 and staff2 overlaps.
 */

bool NoteList::stavesOverlap(const int staff1, const int staff2) const
{
    for (size_t i = 0; i < _staffNoteLists.at(staff1).size(); ++i) {
        for (size_t j = 0; j < _staffNoteLists.at(staff2).size(); ++j) {
            if (notesOverlap(_staffNoteLists.at(staff1).at(i), _staffNoteLists.at(staff2).at(j))) {
                //printf(" %d-%d", staff1, staff2);
                return true;
            }
        }
    }
    return false;
}

/**
 Determine if any note in any staff overlaps.
 */

bool NoteList::anyStaffOverlaps() const
{
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            if (stavesOverlap(i, j)) {
                return true;
            }
        }
    }
    return false;
}

VoiceOverlapDetector::VoiceOverlapDetector()
{
    // LOGD("VoiceOverlapDetector::VoiceOverlapDetector(staves %d)", MAX_STAVES);
}

void VoiceOverlapDetector::addNote(const int startTick, const int endTick, const int& voice, const int staff)
{
    // if necessary, create the note list for voice
    if (!mu::contains(_noteLists, voice)) {
        _noteLists.insert({ voice, NoteList() });
    }
    _noteLists[voice].addNote(startTick, endTick, staff);
}

void VoiceOverlapDetector::dump() const
{
    // LOGD("VoiceOverlapDetector::dump()");
    for (auto p : _noteLists) {
        p.second.dump(p.first);
    }
}

void VoiceOverlapDetector::newMeasure()
{
    // LOGD("VoiceOverlapDetector::newMeasure()");
    _noteLists.clear();
}

bool VoiceOverlapDetector::stavesOverlap(const int& voice) const
{
    if (mu::contains(_noteLists, voice)) {
        return _noteLists.at(voice).anyStaffOverlaps();
    } else {
        return false;
    }
}

String MusicXMLInstrument::toString() const
{
    return String(u"chan %1 prog %2 vol %3 pan %4 unpitched %5 name '%6' sound '%7' head %8 line %9 stemDir %10")
           .arg(midiChannel)
           .arg(midiProgram)
           .arg(midiVolume)
           .arg(midiPan)
           .arg(unpitched)
           .arg(name, sound)
           .arg(int(notehead))
           .arg(line)
           .arg(int(stemDirection));
}

//---------------------------------------------------------
//   errorStringWithLocation
//---------------------------------------------------------

String errorStringWithLocation(int line, int col, const String& error)
{
    return mtrc("iex_musicxml", "line %1, column %2:").arg(line).arg(col) + u" " + error;
}

//---------------------------------------------------------
//   checkAtEndElement
//---------------------------------------------------------

String checkAtEndElement(const XmlStreamReader& e, const String& expName)
{
    if (e.isEndElement() && e.name() == expName.toAscii().constChar()) {
        return String();
    }

    String res = mtrc("iex_musicxml", "expected token type and name ‘EndElement %1’, actual ‘%2 %3’")
                 .arg(expName)
                 .arg(String::fromAscii(e.tokenString().ascii()))
                 .arg(String::fromAscii(e.name().ascii()));
    return res;
}

//---------------------------------------------------------
//   stringToInt
//---------------------------------------------------------

/**
 Convert a string in \a s into an int. Set *ok to true iff conversion was
 successful. \a s may end with ".0", as is generated by Audiveris 3.2 and up,
 in elements <divisions>, <duration>, <alter> and <sound> attributes
 dynamics and tempo.
 In case of error val return a default value of 0.
 Note that non-integer values cannot be handled by mscore.
 */

int MxmlSupport::stringToInt(const String& s, bool* ok)
{
    int res = 0;
    String str = s;
    if (s.endsWith(u".0")) {
        str = s.left(s.size() - 2);
    }
    res = str.toInt(ok);
    return res;
}

//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

Fraction MxmlSupport::noteTypeToFraction(const String& type)
{
    if (type == u"1024th") {
        return Fraction(1, 1024);
    } else if (type == u"512th") {
        return Fraction(1, 512);
    } else if (type == u"256th") {
        return Fraction(1, 256);
    } else if (type == u"128th") {
        return Fraction(1, 128);
    } else if (type == u"64th") {
        return Fraction(1, 64);
    } else if (type == u"32nd") {
        return Fraction(1, 32);
    } else if (type == u"16th") {
        return Fraction(1, 16);
    } else if (type == u"eighth") {
        return Fraction(1, 8);
    } else if (type == u"quarter") {
        return Fraction(1, 4);
    } else if (type == u"half") {
        return Fraction(1, 2);
    } else if (type == u"whole") {
        return Fraction(1, 1);
    } else if (type == u"breve") {
        return Fraction(2, 1);
    } else if (type == u"long") {
        return Fraction(4, 1);
    } else if (type == u"maxima") {
        return Fraction(8, 1);
    } else {
        return Fraction(0, 0);
    }
}

//---------------------------------------------------------
//   calculateFraction
//---------------------------------------------------------

/**
 Convert note type, number of dots and actual and normal notes into a duration
 */

Fraction MxmlSupport::calculateFraction(const String& type, int dots, int normalNotes, int actualNotes)
{
    // type
    Fraction f = MxmlSupport::noteTypeToFraction(type);
    if (f.isValid()) {
        // dot(s)
        Fraction f_no_dots = f;
        for (int i = 0; i < dots; ++i) {
            f += (f_no_dots / Fraction(2 << i, 1));
        }
        // tuplet
        if (actualNotes > 0 && normalNotes > 0) {
            f *= normalNotes;
            f /= Fraction(actualNotes, 1);
        }
        // clean up (just in case)
        f.reduce();
    }
    return f;
}

//---------------------------------------------------------
//   accSymId2MxmlString
//---------------------------------------------------------

String accSymId2MxmlString(const SymId id)
{
    String s;
    switch (id) {
    case SymId::accidentalSharp:                 s = u"sharp";
        break;
    case SymId::accidentalNatural:               s = u"natural";
        break;
    case SymId::accidentalFlat:                  s = u"flat";
        break;
    case SymId::accidentalDoubleSharp:           s = u"double-sharp";
        break;
    //case SymId::accidentalDoubleSharp:           s = u"sharp-sharp"; break; // see above
    //case SymId::accidentalDoubleFlat:            s = u"double-flat"; break; // doesn't exist in MusicXML, but see below
    case SymId::accidentalDoubleFlat:            s = u"flat-flat";
        break;
    case SymId::accidentalNaturalSharp:          s = u"natural-sharp";
        break;
    case SymId::accidentalNaturalFlat:           s = u"natural-flat";
        break;
    case SymId::accidentalQuarterToneFlatStein:  s = u"quarter-flat";
        break;
    case SymId::accidentalQuarterToneSharpStein: s = u"quarter-sharp";
        break;
    case SymId::accidentalThreeQuarterTonesFlatZimmermann: s = u"three-quarters-flat";
        break;
    //case SymId::noSym:                                     s = u"three-quarters-flat";  break; // AccidentalType::FLAT_FLAT_SLASH, MuseScore 1?
    case SymId::accidentalThreeQuarterTonesSharpStein:     s = u"three-quarters-sharp";
        break;
    case SymId::accidentalQuarterToneSharpArrowDown:       s = u"sharp-down";
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowUp:   s = u"sharp-up";
        break;
    case SymId::accidentalQuarterToneFlatNaturalArrowDown: s = u"natural-down";
        break;
    case SymId::accidentalQuarterToneSharpNaturalArrowUp:  s = u"natural-up";
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowDown:  s = u"flat-down";
        break;
    case SymId::accidentalQuarterToneFlatArrowUp:          s = u"flat-up";
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowDown: s = u"double-sharp-down";
        break;
    case SymId::accidentalFiveQuarterTonesSharpArrowUp:    s = u"double-sharp-up";
        break;
    case SymId::accidentalFiveQuarterTonesFlatArrowDown:   s = u"flat-flat-down";
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowUp:    s = u"flat-flat-up";
        break;

    case SymId::accidentalArrowDown:             s = u"arrow-down";
        break;
    case SymId::accidentalArrowUp:               s = u"arrow-up";
        break;

    case SymId::accidentalTripleSharp:           s = u"triple-sharp";
        break;
    case SymId::accidentalTripleFlat:            s = u"triple-flat";
        break;

    case SymId::accidentalKucukMucennebSharp:    s = u"slash-quarter-sharp";
        break;
    case SymId::accidentalBuyukMucennebSharp:    s = u"slash-sharp";
        break;
    case SymId::accidentalBakiyeFlat:            s = u"slash-flat";
        break;
    case SymId::accidentalBuyukMucennebFlat:     s = u"double-slash-flat";
        break;

    case SymId::accidental1CommaSharp:           s = u"sharp-1";
        break;
    case SymId::accidental2CommaSharp:           s = u"sharp-2";
        break;
    case SymId::accidental3CommaSharp:           s = u"sharp-3";
        break;
    case SymId::accidental5CommaSharp:           s = u"sharp-5";
        break;
    case SymId::accidental1CommaFlat:            s = u"flat-1";
        break;
    case SymId::accidental2CommaFlat:            s = u"flat-2";
        break;
    case SymId::accidental3CommaFlat:            s = u"flat-3";
        break;
    case SymId::accidental4CommaFlat:            s = u"flat-4";
        break;

    case SymId::accidentalSori:                  s = u"sori";
        break;
    case SymId::accidentalKoron:                 s = u"koron";
        break;
    default:
        s = u"other";
        LOGD("accSymId2MxmlString: unknown accidental %d", static_cast<int>(id));
    }
    return s;
}

//---------------------------------------------------------
//   accSymId2SmuflMxmlString
//---------------------------------------------------------

String accSymId2SmuflMxmlString(const SymId id)
{
    return String::fromAscii(SymNames::nameForSymId(id).ascii());
}

//---------------------------------------------------------
//   mxmlString2accSymId
// see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
//---------------------------------------------------------

SymId mxmlString2accSymId(const String mxmlName, const String smufl)
{
    static std::map<String, SymId> map;   // map MusicXML accidental name to MuseScore enum SymId
    if (map.empty()) {
        map[u"sharp"] = SymId::accidentalSharp;
        map[u"natural"] = SymId::accidentalNatural;
        map[u"flat"] = SymId::accidentalFlat;
        map[u"double-sharp"] = SymId::accidentalDoubleSharp;
        map[u"sharp-sharp"] = SymId::accidentalDoubleSharp;
        //map[u"double-flat"] = SymId::accidentalDoubleFlat; // shouldn't harm, but doesn't exist in MusicXML
        map[u"flat-flat"] = SymId::accidentalDoubleFlat;
        map[u"natural-sharp"] = SymId::accidentalNaturalSharp;
        map[u"natural-flat"] = SymId::accidentalNaturalFlat;

        map[u"quarter-flat"] = SymId::accidentalQuarterToneFlatStein;
        map[u"quarter-sharp"] = SymId::accidentalQuarterToneSharpStein;
        map[u"three-quarters-flat"] = SymId::accidentalThreeQuarterTonesFlatZimmermann;
        map[u"three-quarters-sharp"] = SymId::accidentalThreeQuarterTonesSharpStein;

        map[u"sharp-down"] = SymId::accidentalQuarterToneSharpArrowDown;
        map[u"sharp-up"] = SymId::accidentalThreeQuarterTonesSharpArrowUp;
        map[u"natural-down"] = SymId::accidentalQuarterToneFlatNaturalArrowDown;
        map[u"natural-up"] = SymId::accidentalQuarterToneSharpNaturalArrowUp;
        map[u"flat-down"] = SymId::accidentalThreeQuarterTonesFlatArrowDown;
        map[u"flat-up"] = SymId::accidentalQuarterToneFlatArrowUp;
        map[u"double-sharp-down"] = SymId::accidentalThreeQuarterTonesSharpArrowDown;
        map[u"double-sharp-up"] = SymId::accidentalFiveQuarterTonesSharpArrowUp;
        map[u"flat-flat-down"] = SymId::accidentalFiveQuarterTonesFlatArrowDown;
        map[u"flat-flat-up"] = SymId::accidentalThreeQuarterTonesFlatArrowUp;

        map[u"arrow-down"] = SymId::accidentalArrowDown;
        map[u"arrow-up"] = SymId::accidentalArrowUp;

        map[u"triple-sharp"] = SymId::accidentalTripleSharp;
        map[u"triple-flat"] = SymId::accidentalTripleFlat;

        map[u"slash-quarter-sharp"] = SymId::accidentalKucukMucennebSharp;
        map[u"slash-sharp"] = SymId::accidentalBuyukMucennebSharp;
        map[u"slash-flat"] = SymId::accidentalBakiyeFlat;
        map[u"double-slash-flat"] = SymId::accidentalBuyukMucennebFlat;

        map[u"sharp-1"] = SymId::accidental1CommaSharp;
        map[u"sharp-2"] = SymId::accidental2CommaSharp;
        map[u"sharp-3"] = SymId::accidental3CommaSharp;
        map[u"sharp-5"] = SymId::accidental5CommaSharp;
        map[u"flat-1"] = SymId::accidental1CommaFlat;
        map[u"flat-2"] = SymId::accidental2CommaFlat;
        map[u"flat-3"] = SymId::accidental3CommaFlat;
        map[u"flat-4"] = SymId::accidental4CommaFlat;

        map[u"sori"] = SymId::accidentalSori;
        map[u"koron"] = SymId::accidentalKoron;
    }

    if (mu::contains(map, mxmlName)) {
        return map.at(mxmlName);
    } else if (mxmlName == u"other") {
        return SymNames::symIdByName(smufl);
    } else {
        LOGD("mxmlString2accSymId: unknown accidental '%s'", muPrintable(mxmlName));
    }

    // default
    return SymId::noSym;
}

//---------------------------------------------------------
//   accidentalType2MxmlString
//---------------------------------------------------------

String accidentalType2MxmlString(const AccidentalType type)
{
    String s;
    switch (type) {
    case AccidentalType::SHARP:              s = u"sharp";
        break;
    case AccidentalType::NATURAL:            s = u"natural";
        break;
    case AccidentalType::FLAT:               s = u"flat";
        break;
    case AccidentalType::SHARP2:             s = u"double-sharp";
        break;
    //case AccidentalType::SHARP2:             s = u"sharp-sharp"; break; // see above
    //case AccidentalType::FLAT2:              s = u"double-flat"; break; // doesn't exist in MusicXML, but see below
    case AccidentalType::FLAT2:              s = u"flat-flat";
        break;
    case AccidentalType::NATURAL_SHARP:      s = u"natural-sharp";
        break;
    case AccidentalType::NATURAL_FLAT:       s = u"natural-flat";
        break;
    case AccidentalType::SHARP_ARROW_UP:     s = u"sharp-up";
        break;
    case AccidentalType::MIRRORED_FLAT:      s = u"quarter-flat";
        break;
    case AccidentalType::SHARP_SLASH:        s = u"quarter-sharp";
        break;
    case AccidentalType::MIRRORED_FLAT2:     s = u"three-quarters-flat";
        break;
    //case AccidentalType::FLAT_FLAT_SLASH:    s = u"three-quarters-flat";  break; // MuseScore 1?
    case AccidentalType::SHARP_SLASH4:       s = u"three-quarters-sharp";
        break;
    case AccidentalType::SHARP_ARROW_DOWN:   s = u"sharp-down";
        break;
    case AccidentalType::NATURAL_ARROW_UP:   s = u"natural-up";
        break;
    case AccidentalType::NATURAL_ARROW_DOWN: s = u"natural-down";
        break;
    case AccidentalType::FLAT_ARROW_DOWN:    s = u"flat-down";
        break;
    case AccidentalType::FLAT_ARROW_UP:      s = u"flat-up";
        break;
    case AccidentalType::SHARP2_ARROW_DOWN:  s = u"double-sharp-down";
        break;
    case AccidentalType::SHARP2_ARROW_UP:    s = u"double-sharp-up";
        break;
    case AccidentalType::FLAT2_ARROW_DOWN:   s = u"flat-flat-down";
        break;
    case AccidentalType::FLAT2_ARROW_UP:     s = u"flat-flat-up";
        break;

    case AccidentalType::ARROW_DOWN:         s = u"arrow-down";
        break;
    case AccidentalType::ARROW_UP:           s = u"arrow-up";
        break;

    case AccidentalType::SHARP3:             s = u"triple-sharp";
        break;
    case AccidentalType::FLAT3:              s = u"triple-flat";
        break;

    case AccidentalType::SHARP_SLASH3:       s = u"slash-quarter-sharp";
        break;
    case AccidentalType::SHARP_SLASH2:       s = u"slash-sharp";
        break;
    case AccidentalType::FLAT_SLASH:         s = u"slash-flat";
        break;
    case AccidentalType::FLAT_SLASH2:        s = u"double-slash-flat";
        break;

    case AccidentalType::ONE_COMMA_SHARP:    s = u"sharp-1";
        break;
    case AccidentalType::TWO_COMMA_SHARP:    s = u"sharp-2";
        break;
    case AccidentalType::THREE_COMMA_SHARP:  s = u"sharp-3";
        break;
    case AccidentalType::FIVE_COMMA_SHARP:   s = u"sharp-5";
        break;
    case AccidentalType::ONE_COMMA_FLAT:     s = u"flat-1";
        break;
    case AccidentalType::TWO_COMMA_FLAT:     s = u"flat-2";
        break;
    case AccidentalType::THREE_COMMA_FLAT:   s = u"flat-3";
        break;
    case AccidentalType::FOUR_COMMA_FLAT:    s = u"flat-4";
        break;

    case AccidentalType::SORI:               s = u"sori";
        break;
    case AccidentalType::KORON:              s = u"koron";
        break;
    default:
        s = u"other";
    }
    return s;
}

//---------------------------------------------------------
//   accidentalType2SmuflMxmlString
//---------------------------------------------------------

String accidentalType2SmuflMxmlString(const AccidentalType type)
{
    return mu::key(smuflAccidentalTypes, type);
}

//---------------------------------------------------------
//   mxmlString2accidentalType
//---------------------------------------------------------

/**
 Convert a MusicXML accidental name to a MuseScore enum AccidentalType.
 see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
 */

AccidentalType mxmlString2accidentalType(const String mxmlName, const String smufl)
{
    static std::map<String, AccidentalType> map;   // map MusicXML accidental name to MuseScore enum AccidentalType
    if (map.empty()) {
        map[u"sharp"] = AccidentalType::SHARP;
        map[u"natural"] = AccidentalType::NATURAL;
        map[u"flat"] = AccidentalType::FLAT;
        map[u"double-sharp"] = AccidentalType::SHARP2;
        map[u"sharp-sharp"] = AccidentalType::SHARP2;
        //map[u"double-flat"] = AccidentalType::FLAT2; // shouldn't harm, but doesn't exist in MusicXML
        map[u"flat-flat"] = AccidentalType::FLAT2;
        map[u"natural-sharp"] = AccidentalType::SHARP;
        map[u"natural-flat"] = AccidentalType::FLAT;

        map[u"quarter-flat"] = AccidentalType::MIRRORED_FLAT;
        map[u"quarter-sharp"] = AccidentalType::SHARP_SLASH;
        map[u"three-quarters-flat"] = AccidentalType::MIRRORED_FLAT2;
        map[u"three-quarters-sharp"] = AccidentalType::SHARP_SLASH4;

        map[u"sharp-up"] = AccidentalType::SHARP_ARROW_UP;
        map[u"natural-down"] = AccidentalType::NATURAL_ARROW_DOWN;
        map[u"natural-up"] = AccidentalType::NATURAL_ARROW_UP;
        map[u"sharp-down"] = AccidentalType::SHARP_ARROW_DOWN;
        map[u"flat-down"] = AccidentalType::FLAT_ARROW_DOWN;
        map[u"flat-up"] = AccidentalType::FLAT_ARROW_UP;
        map[u"double-sharp-down"] = AccidentalType::SHARP2_ARROW_DOWN;
        map[u"double-sharp-up"] = AccidentalType::SHARP2_ARROW_UP;
        map[u"flat-flat-down"] = AccidentalType::FLAT2_ARROW_DOWN;
        map[u"flat-flat-up"] = AccidentalType::FLAT2_ARROW_UP;

        map[u"arrow-down"] = AccidentalType::ARROW_DOWN;
        map[u"arrow-up"] = AccidentalType::ARROW_UP;

        map[u"triple-sharp"] = AccidentalType::SHARP3;
        map[u"triple-flat"] = AccidentalType::FLAT3;

        map[u"slash-quarter-sharp"] = AccidentalType::SHARP_SLASH3; // MIRRORED_FLAT_SLASH; ?
        map[u"slash-sharp"] = AccidentalType::SHARP_SLASH2; // SHARP_SLASH; ?
        map[u"slash-flat"] = AccidentalType::FLAT_SLASH;
        map[u"double-slash-flat"] = AccidentalType::FLAT_SLASH2;

        map[u"sharp-1"] = AccidentalType::ONE_COMMA_SHARP;
        map[u"sharp-2"] = AccidentalType::TWO_COMMA_SHARP;
        map[u"sharp-3"] = AccidentalType::THREE_COMMA_SHARP;
        map[u"sharp-5"] = AccidentalType::FIVE_COMMA_SHARP;
        map[u"flat-1"] = AccidentalType::ONE_COMMA_FLAT;
        map[u"flat-2"] = AccidentalType::TWO_COMMA_FLAT;
        map[u"flat-3"] = AccidentalType::THREE_COMMA_FLAT;
        map[u"flat-4"] = AccidentalType::FOUR_COMMA_FLAT;

        map[u"sori"] = AccidentalType::SORI;
        map[u"koron"] = AccidentalType::KORON;
    }

    if (mu::contains(map, mxmlName)) {
        return map.at(mxmlName);
    } else if (mxmlName == "other" && mu::contains(smuflAccidentalTypes, smufl)) {
        return smuflAccidentalTypes.at(smufl);
    } else {
        LOGD("mxmlString2accidentalType: unknown accidental '%s'", muPrintable(mxmlName));
    }
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   isAppr
//---------------------------------------------------------

/**
 Check if v approximately equals ref.
 Used to prevent floating point comparison for equality from failing
 */

static bool isAppr(const double v, const double ref, const double epsilon)
{
    return v > ref - epsilon && v < ref + epsilon;
}

//---------------------------------------------------------
//   microtonalGuess
//---------------------------------------------------------

/**
 Convert a MusicXML alter tag into a microtonal accidental in MuseScore enum AccidentalType.
 Works only for quarter tone, half tone, three-quarters tone and whole tone accidentals.
 */

AccidentalType microtonalGuess(double val)
{
    const double eps = 0.001;
    if (isAppr(val, -2, eps)) {
        return AccidentalType::FLAT2;
    } else if (isAppr(val, -1.5, eps)) {
        return AccidentalType::MIRRORED_FLAT2;
    } else if (isAppr(val, -1, eps)) {
        return AccidentalType::FLAT;
    } else if (isAppr(val, -0.5, eps)) {
        return AccidentalType::MIRRORED_FLAT;
    } else if (isAppr(val, 0, eps)) {
        return AccidentalType::NATURAL;
    } else if (isAppr(val, 0.5, eps)) {
        return AccidentalType::SHARP_SLASH;
    } else if (isAppr(val, 1, eps)) {
        return AccidentalType::SHARP;
    } else if (isAppr(val, 1.5, eps)) {
        return AccidentalType::SHARP_SLASH4;
    } else if (isAppr(val, 2, eps)) {
        return AccidentalType::SHARP2;
    } else {
        LOGD("Guess for microtonal accidental corresponding to value %f failed.", val);
    }
    // default
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   isLaissezVibrer
//---------------------------------------------------------

bool isLaissezVibrer(const SymId id)
{
    return id == SymId::articLaissezVibrerAbove || id == SymId::articLaissezVibrerBelow;
}

//---------------------------------------------------------
//   findLaissezVibrer
//---------------------------------------------------------

// TODO: there should be a lambda hiding somewhere ...

const Articulation* findLaissezVibrer(const Chord* chord)
{
    for (const Articulation* a : chord->articulations()) {
        if (isLaissezVibrer(a->symId())) {
            return a;
        }
    }
    return nullptr;
}
}
