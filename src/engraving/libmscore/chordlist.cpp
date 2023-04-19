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

#include "chordlist.h"

#include "io/file.h"
#include "io/fileinfo.h"
#include "rw/xmlreader.h"
#include "rw/400/readcontext.h"
#include "rw/xmlwriter.h"

#include "style/style.h"

#include "mscore.h"
#include "pitchspelling.h"

#include "log.h"

using namespace mu;
using namespace mu::io;

namespace mu::engraving {
//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const String& str)
{
    static const char* const scaleNames[2][12] = {
        { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
        { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
    };
    keys = 0;
    StringList sl = str.split(u' ', mu::SkipEmptyParts);
    for (const String& s : sl) {
        for (int i = 0; i < 12; ++i) {
            if (s == scaleNames[0][i] || s == scaleNames[1][i]) {
                operator+=(i);
                break;
            }
        }
    }
}

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
{
    keys = 0;
    if (a >= 0) {
        operator+=(a);
    }
    if (b >= 0) {
        operator+=(b);
    }
    if (c >= 0) {
        operator+=(c);
    }
    if (d >= 0) {
        operator+=(d);
    }
    if (e >= 0) {
        operator+=(e);
    }
    if (f >= 0) {
        operator+=(f);
    }
    if (g >= 0) {
        operator+=(g);
    }
    if (h >= 0) {
        operator+=(h);
    }
    if (i >= 0) {
        operator+=(i);
    }
    if (k >= 0) {
        operator+=(k);
    }
    if (l >= 0) {
        operator+=(l);
    }
}

//---------------------------------------------------------
//   rotate
//    rotate 12 Bits
//---------------------------------------------------------

void HChord::rotate(int semiTones)
{
    while (semiTones > 0) {
        if (keys & 0x800) {
            keys = ((keys & ~0x800) << 1) + 1;
        } else {
            keys <<= 1;
        }
        --semiTones;
    }
    while (semiTones < 0) {
        if (keys & 1) {
            keys = (keys >> 1) | 0x800;
        } else {
            keys >>= 1;
        }
        ++semiTones;
    }
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

String HChord::name(int tpc) const
{
    static const HChord C0(0, 3, 6, 9);
    static const HChord C1(0, 3);

    String buf = tpc2name(tpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, false);
    HChord c(*this);

    int key = tpc2pitch(tpc);

    c.rotate(-key);          // transpose to C

    // special cases
    if (c == C0) {
        buf += u"dim";
        return buf;
    }
    if (c == C1) {
        buf += u"no5";
        return buf;
    }

    bool seven   = false;
    bool sharp9  = false;
    bool nat11   = false;
    bool sharp11 = false;
    bool nat13   = false;
    bool flat13  = false;

    // minor?
    if (c.contains(3)) {
        if (!c.contains(4)) {
            buf += u"m";
        } else {
            sharp9 = true;
        }
    }

    // 7
    if (c.contains(11)) {
        buf += u"Maj7";
        seven = true;
    } else if (c.contains(10)) {
        buf += u"7";
        seven = true;
    }

    // 4
    if (c.contains(5)) {
        if (!c.contains(4)) {
            buf += u"sus4";
        } else {
            nat11 = true;
        }
    }

    // 5
    if (c.contains(7)) {
        if (c.contains(6)) {
            sharp11 = true;
        }
        if (c.contains(8)) {
            flat13 = true;
        }
    } else {
        if (c.contains(6)) {
            buf += u"b5";
        }
        if (c.contains(8)) {
            buf += u"#5";
        }
    }

    // 6
    if (c.contains(9)) {
        if (!seven) {
            buf += u"6";
        } else {
            nat13 = true;
        }
    }

    // 9
    if (c.contains(1)) {
        buf += u"b9";
    }
    if (c.contains(2)) {
        buf += u"9";
    }
    if (sharp9) {
        buf += u"#9";
    }

    // 11
    if (nat11) {
        buf += u"11 ";
    }
    if (sharp11) {
        buf += u"#11";
    }

    // 13
    if (flat13) {
        buf += u"b13";
    }
    if (nat13) {
        if (c.contains(1) || c.contains(2) || sharp9 || nat11 || sharp11) {
            buf += u"13";
        } else {
            buf += u"add13";
        }
    }
    return buf;
}

//---------------------------------------------------------
//   voicing
//---------------------------------------------------------

String HChord::voicing() const
{
    String s = u"C";
    const char* names[] = { "C", " Db", " D", " Eb", " E", " F", " Gb", " G", " Ab", " A", " Bb", " B" };

    for (int i = 1; i < 12; i++) {
        if (contains(i)) {
            s += String::fromAscii(names[i]);
        }
    }
    return s;
}

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void HChord::print() const
{
    const char* names[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    for (int i = 0; i < 12; i++) {
        if (contains(i)) {
            LOGD(" %s", names[i]);
        }
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void HChord::add(const std::vector<HDegree>& degreeList)
{
// LOGD("HChord::add   ");print();
    // convert degrees to semitones
    static const int degreeTable[] = {
        // 1  2  3  4  5  6   7
        // C  D  E  F  G  A   B
        0, 2, 4, 5, 7, 9, 11
    };
    // factor in the degrees
    for (const HDegree& d : degreeList) {
        int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
        int dv1 = degreeTable[(d.value() - 1) % 7];

        if (d.value() == 7 && d.alter() == 0) {
            dv -= 1;
        }

        if (d.type() == HDegreeType::ADD) {
            *this += dv;
        } else if (d.type() == HDegreeType::ALTER) {
            if (contains(dv1)) {
                *this -= dv1;
                *this += dv;
            } else {
//                        LOGD("HDegreeType::ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                *this += dv;              // DEBUG: default to add
            }
        } else if (d.type() == HDegreeType::SUBTRACT) {
            if (contains(dv1)) {
                *this -= dv1;
            } else {
                LOGD("SUB: chord does not contain degree %d(%d):",
                     d.value(), d.alter());
            }
        } else {
            LOGD("degree type %d not supported", static_cast<int>(d.type()));
        }

// LOGD("  HCHord::added  "); print();
    }
}

//---------------------------------------------------------
//   readRenderList
//---------------------------------------------------------

static void readRenderList(String val, std::list<RenderAction>& renderList)
{
    renderList.clear();
    StringList sl = val.split(u' ', mu::SkipEmptyParts);
    for (const String& s : sl) {
        if (s.startsWith(u"m:")) {
            StringList ssl = s.split(u':', mu::SkipEmptyParts);
            if (ssl.size() == 3) {
                // m:x:y
                RenderAction a;
                a.type = RenderAction::RenderActionType::MOVE;
                a.movex = ssl[1].toDouble();
                a.movey = ssl[2].toDouble();
                renderList.push_back(a);
            }
        } else if (s == u":push") {
            renderList.push_back(RenderAction(RenderAction::RenderActionType::PUSH));
        } else if (s == u":pop") {
            renderList.push_back(RenderAction(RenderAction::RenderActionType::POP));
        } else if (s == u":n") {
            renderList.push_back(RenderAction(RenderAction::RenderActionType::NOTE));
        } else if (s == u":a") {
            renderList.push_back(RenderAction(RenderAction::RenderActionType::ACCIDENTAL));
        } else {
            RenderAction a(RenderAction::RenderActionType::SET);
            a.text = s;
            renderList.push_back(a);
        }
    }
}

//---------------------------------------------------------
//   writeRenderList
//---------------------------------------------------------

static void writeRenderList(XmlWriter& xml, const std::list<RenderAction>& al, const AsciiStringView& name)
{
    String s;

    for (const RenderAction& a : al) {
        if (!s.isEmpty()) {
            s += u" ";
        }
        switch (a.type) {
        case RenderAction::RenderActionType::SET:
            s += a.text;
            break;
        case RenderAction::RenderActionType::MOVE:
            if (a.movex != 0.0 || a.movey != 0.0) {
                s += String(u"m:%1:%2").arg(a.movex).arg(a.movey);
            }
            break;
        case RenderAction::RenderActionType::PUSH:
            s += u":push";
            break;
        case RenderAction::RenderActionType::POP:
            s += u":pop";
            break;
        case RenderAction::RenderActionType::NOTE:
            s += u":n";
            break;
        case RenderAction::RenderActionType::ACCIDENTAL:
            s += u":a";
            break;
        }
    }
    xml.tag(name, s);
}

//---------------------------------------------------------
//  read
//---------------------------------------------------------

void ChordToken::read(XmlReader& e)
{
    String c = e.attribute("class");
    if (c == "quality") {
        tokenClass = ChordTokenClass::QUALITY;
    } else if (c == "extension") {
        tokenClass = ChordTokenClass::EXTENSION;
    } else if (c == "modifier") {
        tokenClass = ChordTokenClass::MODIFIER;
    } else {
        tokenClass = ChordTokenClass::ALL;
    }
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            names << e.readText();
        } else if (tag == "render") {
            readRenderList(e.readText(), renderList);
        }
    }
}

//---------------------------------------------------------
//  write
//---------------------------------------------------------

void ChordToken::write(XmlWriter& xml) const
{
    XmlWriter::Attributes attrs;
    switch (tokenClass) {
    case ChordTokenClass::QUALITY:
        attrs.push_back({ "class", "quality" });
        break;
    case ChordTokenClass::EXTENSION:
        attrs.push_back({ "class", "extension" });
        break;
    case ChordTokenClass::MODIFIER:
        attrs.push_back({ "class", "modifier" });
        break;
    default:
        break;
    }
    xml.startElement("token", attrs);
    for (const String& s : names) {
        xml.tag("name", s);
    }
    writeRenderList(xml, renderList, "render");
    xml.endElement();
}

//---------------------------------------------------------
//  ParsedChord
//---------------------------------------------------------

ParsedChord::ParsedChord()
{
    _parseable = false;
    _understandable = false;
}

//---------------------------------------------------------
//  configure
//---------------------------------------------------------

void ParsedChord::configure(const ChordList* cl)
{
    if (!cl) {
        return;
    }
    // TODO: allow this to be parameterized via chord list
    major << u"ma" << u"maj" << u"major" << u"t" << u"^";
    minor << u"mi" << u"min" << u"minor" << u"-" << u"=";
    diminished << u"dim" << u"o";
    augmented << u"aug" << u"+";
    lower << u"b" << u"-" << u"dim";
    raise << u"#" << u"+" << u"aug";
    mod1 << u"sus" << u"alt";
    mod2 << u"sus" << u"add" << u"no" << u"omit" << u"^";
    symbols << u"t" << u"^" << u"-" << u"+" << u"o" << u"0";
}

//---------------------------------------------------------
//  correctXmlText
//    remove digits from _xmlText, optionally replace with s
//    needed for m(Maj9) et al
//---------------------------------------------------------

void ParsedChord::correctXmlText(const String& s)
{
    String xmlText = _xmlText;
    xmlText.remove(std::regex("[0-9]"));
    if (s != "") {
        size_t pos = xmlText.lastIndexOf(u')');
        if (pos == mu::nidx) {
            pos = xmlText.size();
        }
        xmlText.insert(pos, s);
    }
    _xmlText = xmlText;
}

//---------------------------------------------------------
//  parse
//    returns true if chord was parseable
//---------------------------------------------------------

bool ParsedChord::parse(const String& s, const ChordList* cl, bool syntaxOnly, bool preferMinor)
{
    String tok1, tok1L, tok2, tok2L;
    String extensionDigits = u"123456789";
    String special = u"()[],/\\ ";
    String leading = u"([ ";
    String trailing = u")],/\\ ";
    String initial;
    bool take6 = false, take7 = false, take9 = false, take11 = false, take13 = false;
    size_t lastLeadingToken = 0;
    size_t len = s.size();
    size_t i = 0;
    int thirdKey = 0, seventhKey = 0;
    bool susChord = false;
    std::vector<HDegree> hdl;
    int key[] = { 0, 0, 2, 4, 5, 7, 9, 11, 0, 2, 4, 5, 7, 9, 11 };

    configure(cl);
    _name = s;
    _parseable = true;
    _understandable = true;

    lastLeadingToken = _tokenList.size();
    // get quality
    for (tok1 = u"", tok1L = u"", initial = u""; i < len; ++i) {
        // up to first (non-zero) digit, paren, or comma
        if (extensionDigits.contains(s.at(i)) || special.contains(s.at(i))) {
            break;
        }
        tok1.append(s.at(i));
        tok1L.append(s.at(i).toLower());
        if (tok1L == u"m" || major.contains(tok1L) || minor.contains(tok1L) || diminished.contains(tok1L) || augmented.contains(tok1L)) {
            initial = tok1;
        }
    }
    // special case for "madd", which needs to parse as m,add rather than ma,dd
    if (tok1L.startsWith(u"madd")) {
        initial = tok1.at(0);
    }
    // quality and first modifier ran together with no separation - eg, mima7, augadd
    // keep quality portion, reset index to read modifier portion later
    if (initial != "" && initial != tok1 && tok1L != "tristan" && tok1L != "omit") {
        i -= (tok1.size() - initial.size());
        tok1 = initial;
        tok1L = initial.toLower();
    }
    // determine quality
    if (tok1 == u"M" || major.contains(tok1L)) {
        _quality = u"major";
        take6 = true;
        take7 = true;
        take9 = true;
        take11 = true;
        take13 = true;
        if (!syntaxOnly) {
            chord = HChord(u"C E G");
        }
    } else if (tok1 == u"m" || minor.contains(tok1L)) {
        _quality = u"minor";
        take6 = true;
        take7 = true;
        take9 = true;
        take11 = true;
        take13 = true;
        if (!syntaxOnly) {
            chord = HChord(u"C Eb G");
        }
    } else if (diminished.contains(tok1L)) {
        _quality = u"diminished";
        take7 = true;
        if (!syntaxOnly) {
            chord = HChord(u"C Eb Gb");
        }
    } else if (augmented.contains(tok1L)) {
        _quality = u"augmented";
        take7 = true;
        if (!syntaxOnly) {
            chord = HChord(u"C E G#");
        }
    } else if (tok1L == "0") {
        _quality = u"half-diminished";
        if (!syntaxOnly) {
            chord = HChord(u"C Eb Gb Bb");
        }
    } else if (tok1L == "") {
        // empty quality - this will turn out to be major or dominant (or minor if preferMinor)
        _quality = u"";
        if (!syntaxOnly) {
            chord = preferMinor ? HChord(u"C Eb G") : HChord(u"C E G");
        }
        if (preferMinor) {
            _name = u"=" + _name;
        }
    } else {
        // anything else is not a quality after all, but a modifier
        // reset to read again as modifier
        _quality = u"";
        tok1 = u"";
        tok1L = u"";
        i = static_cast<int>(lastLeadingToken);
        if (!syntaxOnly) {
            chord = HChord(u"C E G");
        }
    }
    if (tok1L == "=") {
        tok1 = u"";
        tok1L = u"";
    }
    if (tok1 != "") {
        addToken(tok1, ChordTokenClass::QUALITY);
    }
    if (!syntaxOnly) {
        _xmlKind = _quality;
        _xmlParens = u"no";
        if (symbols.contains(tok1)) {
            _xmlSymbols = u"yes";
            _xmlText = u"";
        } else {
            _xmlSymbols = u"no";
            _xmlText = tok1;
        }
    }
    // eat trailing parens and commas
    while (i < len && trailing.contains(s.at(i))) {
        addToken(String(s.at(i++)), ChordTokenClass::QUALITY);
    }

    lastLeadingToken = _tokenList.size();
    // get extension - up to first non-digit other than comma or slash
    for (tok1 = u""; i < len; ++i) {
        if (!s.at(i).isDigit() && s.at(i) != ',' && s.at(i) != '/') {
            break;
        }
        tok1.append(s.at(i));
    }
    _extension = tok1;
    if (_quality == "") {
        if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13") {
            _quality = preferMinor ? u"minor" : u"dominant";
            if (!syntaxOnly) {
                _xmlKind = preferMinor ? u"minor" : u"dominant";
            }
            take7 = true;
            take9 = true;
            take11 = true;
            take13 = true;
        } else {
            _quality = preferMinor ? u"minor" : u"major";
            if (!syntaxOnly) {
                _xmlKind = preferMinor ? u"minor" : u"major";
            }
            take6 = true;
            take7 = true;
            take9 = true;
            take11 = true;
            take13 = true;
        }
    }
    if (tok1 != "") {
        addToken(tok1, ChordTokenClass::EXTENSION);
    }
    if (!syntaxOnly) {
        if (_quality == "minor") {
            thirdKey = 3;
        } else {
            thirdKey = 4;
        }
        if (_quality == "major") {
            seventhKey = 11;
        } else if (_quality == "diminished") {
            seventhKey = 9;
        } else {
            seventhKey = 10;
        }
        _xmlText += _extension;
        StringList extl;
        if (tok1 == "2") {
            String d = u"add" + tok1;
            _xmlDegrees << d;
            _xmlText.remove(tok1);
            chord += 2;
        } else if (tok1 == "4") {
            String d = u"add" + tok1;
            _xmlDegrees << d;
            _xmlText.remove(tok1);
            chord += 5;
        } else if (tok1 == "5") {
            _xmlKind = u"power";
            chord -= thirdKey;
        } else if (tok1 == "6") {
            if (take6) {
                _xmlKind += u"-sixth";
            } else {
                extl << u"6";
            }
            chord += 9;
        } else if (tok1 == "7") {
            if (take7) {
                _xmlKind += u"-seventh";
            } else if (_xmlKind != "half-diminished") {
                extl << u"7";
            }
            chord += seventhKey;
        } else if (tok1 == "9") {
            if (take9) {
                _xmlKind += u"-ninth";
            } else if (take7) {
                _xmlKind += u"-seventh";
                extl << u"9";
                correctXmlText(u"7");
            } else if (_xmlKind == u"half-diminished") {
                extl << u"9";
                correctXmlText();
            } else {
                extl << u"7" << u"9";
                correctXmlText();
            }
            chord += seventhKey;
            chord += 2;
        } else if (tok1 == u"11") {
            if (take11) {
                _xmlKind += u"-11th";
            } else if (take7) {
                _xmlKind += u"-seventh";
                extl << u"9" << u"11";
                correctXmlText(u"7");
            } else if (_xmlKind == u"half-diminished") {
                extl << u"9" << u"11";
                correctXmlText();
            } else {
                extl << u"7" << u"9" << u"11";
                correctXmlText();
            }
            chord += seventhKey;
            chord += 2;
            chord += 5;
        } else if (tok1 == u"13") {
            if (take13) {
                _xmlKind += u"-13th";
            } else if (take7) {
                _xmlKind += u"-seventh";
                extl << u"9" << u"11" << u"13";
                correctXmlText(u"7");
            } else if (_xmlKind == u"half-diminished") {
                extl << u"9" << u"11" << u"13";
                correctXmlText();
            } else {
                extl << u"7" << u"9" << u"11" << u"13";
                correctXmlText();
            }
            chord += seventhKey;
            chord += 2;
            chord += 5;
            chord += 9;
        } else if (tok1 == u"69" || tok1 == u"6,9" || tok1 == u"6/9") {
            if (take6) {
                _xmlKind += u"-sixth";
                extl << u"9";
                correctXmlText(u"6");
            } else {
                extl << u"6" << u"9";
                correctXmlText();
            }
            _extension = u"69";
            chord += 9;
            chord += 2;
        }
        for (const String& e : extl) {
            String d = u"add" + e;
            _xmlDegrees << d;
        }
        if (_xmlKind == u"dominant-seventh") {
            _xmlKind = u"dominant";
        }
    }
    // eat trailing parens and commas
    while (i < len && trailing.contains(s.at(i))) {
        addToken(String(s.at(i++)), ChordTokenClass::EXTENSION);
    }

    // get modifiers
    bool addPending = false;
    _modifierList.clear();
    while (i < len) {
        // eat leading parens
        while (i < len && leading.contains(s.at(i))) {
            addToken(String(s.at(i++)), ChordTokenClass::MODIFIER);
            _xmlParens = u"yes";
        }
        // get first token - up to first digit, paren, or comma
        for (tok1 = u"", tok1L = u"", initial = u""; i < len; ++i) {
            if (s.at(i).isDigit() || special.contains(s.at(i))) {
                break;
            }
            tok1.append(s.at(i));
            tok1L.append(s.at(i).toLower());
            if (mod2.contains(tok1L)) {
                initial = tok1;
            }
        }
        // if we reached the end of the string and never got a token,
        // then nothing to do, and no sense in looking for a second token
        if (i == len && tok1 == "") {
            break;
        }
        if (initial != "" && initial != tok1) {
            // two modifiers ran together with no separation - eg, susb9
            // keep first, reset index to read second later
            i -= (tok1.size() - initial.size());
            tok1 = initial;
            tok1L = initial.toLower();
        }
        // for "add", just add the token and then read argument as a separate modifier
        // this allows the argument to itself be a two-part string
        // thus allowing addb9 -> add;b,9
        if (tok1L == "add") {
            addToken(tok1, ChordTokenClass::MODIFIER);
            addPending = true;
            continue;
        }
        // eat spaces
        while (i < len && s.at(i) == u' ') {
            ++i;
        }
        // get second token - a number <= 13
        for (tok2 = u""; i < len; ++i) {
            if (!s.at(i).isDigit()) {
                break;
            }
            if (tok2.size() == 1 && (tok2.at(0) != u'1' || s.at(i) > u'3')) {
                break;
            }
            tok2.append(s.at(i));
        }
        tok2L = tok2.toLower();
        // re-attach "add"
        if (addPending) {
            if (raise.contains(tok1L)) {
                tok1L = u"#";
            } else if (lower.contains(tok1L)) {
                tok1L = u"b";
            } else if (tok1 == "M" || major.contains(tok1L)) {
                tok1L = u"major";
            }
            tok2L = tok1L + tok2L;
            tok1L = u"add";
        }
        // standardize spelling
        if (tok1 == "M" || major.contains(tok1L)) {
            tok1L = u"major";
        } else if (tok1L == "omit") {
            tok1L = u"no";
        } else if (tok1L == "sus" && tok2L == "") {
            tok2L = u"4";
        } else if (augmented.contains(tok1L) && tok2L == "") {
            if (_quality == "dominant" && _extension == "7") {
                _quality = u"augmented";
                if (!syntaxOnly) {
                    _xmlKind = u"augmented-seventh";
                    _xmlText = _extension + tok1;
                    chord -= 7;
                    chord += 8;
                }
                tok1L = u"";
            } else {
                tok1L = u"#";
                tok2L = u"5";
            }
        } else if (diminished.contains(tok1)) {
            _quality = u"diminished";
            if (!syntaxOnly) {
                _xmlKind = u"diminished";
                _xmlText = _extension + tok1;
                chord -= 4;
                chord += 3;
                chord -= 7;
                chord += 6;
            }
            tok1L = u"";
        } else if ((lower.contains(tok1L) || raise.contains(tok1L)) && tok2L == "") {
            // trailing alteration - treat as applying to extension (and convert to modifier)
            // this handles C5b, C9#, etc
            tok2L = _extension;
            if (!syntaxOnly) {
                _xmlKind = (_quality == u"dominant") ? u"major" : _quality;
                _xmlText.remove(_extension);
                if (_extension == "5") {
                    chord += thirdKey;
                } else {
                    chord -= seventhKey;
                }
            }
            if (_quality == "dominant") {
                _quality = u"major";
            }
            _extension = u"";
            if (lower.contains(tok1L)) {
                tok1L = u"b";
            } else {
                tok1L = u"#";
            }
        } else if (lower.contains(tok1L)) {
            tok1L = u"b";
        } else if (raise.contains(tok1L)) {
            tok1L = u"#";
        }
        String m = tok1L + tok2L;
        if (m != "") {
            _modifierList << m;
        }
        if (tok1 != "") {
            addToken(tok1, ChordTokenClass::MODIFIER);
        }
        if (tok2 != "") {
            addToken(tok2, ChordTokenClass::MODIFIER);
        }
        if (!syntaxOnly) {
            int d;
            if (tok2L == "") {
                d = 0;
            } else {
                d = tok2L.toInt();
            }
            if (d > 13) {
                d = 13;
            }
            String degree;
            bool alter = false;
            if (tok1L == "add") {
                if (d) {
                    hdl.push_back(HDegree(d, 0, HDegreeType::ADD));
                } else if (tok2L != "") {
                    // this was result of addPending
                    // alteration; tok1 = alter, tok2 = value
                    d = tok2.toInt();
                    if (raise.contains(tok1) || tok1 == "M" || major.contains(tok1.toLower())) {
                        if (d == 7) {
                            chord += 11;
                            tok2L = u"#7";
                        } else if (raise.contains(tok1)) {
                            hdl.push_back(HDegree(d, 1, HDegreeType::ADD));
                        }
                    } else if (lower.contains(tok1)) {
                        if (d == 7) {
                            chord += 10;
                        } else {
                            hdl.push_back(HDegree(d, -1, HDegreeType::ADD));
                        }
                    } else if (d) {
                        hdl.push_back(HDegree(d, 0, HDegreeType::ADD));
                    }
                }
                degree = u"add" + tok2L;
            } else if (tok1L == u"no") {
                degree = u"sub" + tok2L;
                if (d) {
                    hdl.push_back(HDegree(d, 0, HDegreeType::SUBTRACT));
                }
            } else if (tok1L == "sus") {
                // convert chords with sus into suspended "kind"
                // extension then becomes a series of degree adds
                if (tok2L == "4") {
                    _xmlKind = u"suspended-fourth";
                } else if (tok2L == "2") {
                    _xmlKind = u"suspended-second";
                }
                _xmlText = tok1 + tok2;
                if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13") {
                    _xmlDegrees << ((_quality == u"major") ? u"add#7" : u"add7");
                    // hack for programs that cannot assemble names well
                    // even though the kind is suspended, set text to also include the extension
                    // in export, we will set the degree text to null
                    _xmlText = _extension + _xmlText;
                    degree = u"";
                } else if (_extension != "") {
                    degree = u"add" + _extension;
                }
                if (_extension == u"13") {
                    _xmlDegrees << u"add9";
                    _xmlDegrees << u"add11";
                    _xmlDegrees << u"add13";
                } else if (_extension == u"11") {
                    _xmlDegrees << u"add9";
                    _xmlDegrees << u"add11";
                } else if (_extension == u"9") {
                    _xmlDegrees << u"add9";
                }
                susChord = true;
                chord -= thirdKey;
                if (d) {
                    chord += key[d];
                }
            } else if (tok1L == u"major") {
                if (_xmlKind.startsWith(u"minor")) {
                    _xmlKind = u"major-minor";
                    if (_extension == u"9" || tok2L == u"9") {
                        _xmlDegrees << u"add9";
                    }
                    if (_extension == "11" || tok2L == u"11") {
                        _xmlDegrees << u"add9";
                        _xmlDegrees << u"add11";
                    }
                    if (_extension == u"13" || tok2L == u"13") {
                        _xmlDegrees << u"add9";
                        _xmlDegrees << u"add11";
                        _xmlDegrees << u"add13";
                    }
                    _xmlText += tok1 + tok2;
                    correctXmlText(u"7");
                } else {
                    tok1L = u"add";
                }
                chord -= 10;
                chord += 11;
                if (d && d != 7) {
                    hdl.push_back(HDegree(d, 0, HDegreeType::ADD));
                }
            } else if (tok1L == u"alt") {
                _xmlDegrees << u"altb5";
                _xmlDegrees << u"add#5";
                _xmlDegrees << u"addb9";
                _xmlDegrees << u"add#9";
                chord -= 7;
                chord += 6;
                chord += 8;
                chord += 1;
                chord += 3;
            } else if (tok1L == u"blues") {
                // this isn't really well-defined, but it might as well mean something
                if (_extension == u"11" || _extension == u"13") {
                    _xmlDegrees << u"alt#9";
                } else {
                    _xmlDegrees << u"add#9";
                }
                chord += 3;
            } else if (tok1L == u"lyd") {
                if (_extension == u"13") {
                    _xmlDegrees << u"alt#11";
                } else {
                    _xmlDegrees << u"add#11";
                }
                chord += 6;
            } else if (tok1L == "phryg") {
                if (!_xmlKind.startsWith(u"minor")) {
                    _xmlKind = u"minor-seventh";
                }
                if (_extension == "11" || _extension == "13") {
                    _xmlDegrees << u"altb9";
                } else {
                    _xmlDegrees << u"addb9";
                }
                _xmlText += tok1;
                chord = HChord(u"C Db Eb G Bb");
            } else if (tok1L == "tristan") {
                _xmlKind = u"Tristan";
                _xmlText = tok1;
                chord = HChord(u"C F# A# D#");
            } else if (addPending) {
                degree = u"add" + tok1L + tok2L;
                if (raise.contains(tok1L)) {
                    hdl.push_back(HDegree(d, 1, HDegreeType::ADD));
                } else if (lower.contains(tok1L)) {
                    hdl.push_back(HDegree(d, -1, HDegreeType::ADD));
                } else {
                    hdl.push_back(HDegree(d, 0, HDegreeType::ADD));
                }
            } else if (tok1L == "" && tok2L != "") {
                degree = u"add" + tok2L;
                hdl.push_back(HDegree(d, 0, HDegreeType::ADD));
            } else if (lower.contains(tok1L)) {
                tok1L = u"b";
                alter = true;
            } else if (raise.contains(tok1L)) {
                tok1L = u"#";
                alter = true;
            } else if (tok1L == "") {
                // token was already handled fully
            } else {
                _understandable = false;
                if (s.startsWith(tok1)) {
                    // unrecognized token right from very beginning
                    _xmlKind = u"other";
                    _xmlText = tok1;
                }
            }
            if (alter) {
                if (tok2L == "4" && _xmlKind == "suspended-fourth") {
                    degree = u"alt";
                } else if (tok2L == "5") {
                    degree = u"alt";
                } else if (tok2L == "9" && (_extension == "11" || _extension == "13")) {
                    degree = u"alt";
                } else if (tok2L == "11" && _extension == "13") {
                    degree = u"alt";
                } else {
                    degree = u"add";
                }
                degree += tok1L + tok2L;
                if (chord.contains(key[d]) && !(susChord && (d == 11))) {
                    hdl.push_back(HDegree(d, 0, HDegreeType::SUBTRACT));
                }
                if (tok1L == "#") {
                    hdl.push_back(HDegree(d, 1, HDegreeType::ADD));
                } else if (tok1L == "b") {
                    hdl.push_back(HDegree(d, -1, HDegreeType::ADD));
                }
            }
            if (degree != "") {
                _xmlDegrees << degree;
            }
        }
        // eat trailing parens and commas
        while (i < len && trailing.contains(s.at(i))) {
            addToken(String(s.at(i++)), ChordTokenClass::MODIFIER);
        }
        addPending = false;
    }
    if (!syntaxOnly) {
        chord.add(hdl);
        // fix "add" / "alt" conflicts
        // so add9,altb9 -> addb9
        StringList altList = _xmlDegrees.filter(u"alt");
        for (const String& d : altList) {
            String unalt(d);
            unalt.replace(std::regex("alt[b#]"), u"add");
            if (_xmlDegrees.removeAll(unalt)) {
                String alt(d);
                alt.replace(u"alt", u"add");
                size_t i1 = _xmlDegrees.indexOf(d);
                _xmlDegrees.replace(i1, alt);
            }
        }
    }

    // construct handle
    if (!_modifierList.empty()) {
        std::sort(_modifierList.begin(), _modifierList.end());
        _modifiers = u"<" + _modifierList.join(u"><") + u">";
    }
    _handle = u"<" + _quality + u"><" + _extension + u">" + _modifiers;

    // force <minor><7><b5> to export as half-diminished
    if (!syntaxOnly && _handle == u"<minor><7><b5>") {
        _xmlKind = u"half-diminished";
        _xmlText = s;
        _xmlDegrees.clear();
    }
    if (MScore::debugMode) {
        LOGD("parse: source = <%s>, handle = %s", muPrintable(s), muPrintable(_handle));
        if (!syntaxOnly) {
            LOGD("parse: HChord = <%s> (%d)", muPrintable(chord.voicing()), chord.getKeys());
            LOGD("parse: xmlKind = <%s>, text = <%s>", muPrintable(_xmlKind), muPrintable(_xmlText));
            LOGD("parse: xmlSymbols = %s, xmlParens = %s", muPrintable(_xmlSymbols), muPrintable(_xmlParens));
            LOGD("parse: xmlDegrees = <%s>", muPrintable(_xmlDegrees.join(u",")));
        }
    }
    return _parseable;
}

//---------------------------------------------------------
//   fromXml
//---------------------------------------------------------

String ParsedChord::fromXml(const String& rawKind, const String& rawKindText, const String& useSymbols, const String& useParens,
                            const std::list<HDegree>& dl, const ChordList* cl)
{
    String kind = rawKind;
    String kindText = rawKindText;
    bool syms = (useSymbols == "yes");
    bool parens = (useParens == "yes");
    bool implied = false;
    bool extend = false;
    int extension = 0;
    _parseable = true;
    _understandable = true;

    // get quality info from kind
    if (kind == "major-minor") {
        _quality = u"minor";
        _modifierList << u"major7";
        extend = true;
    } else if (kind.contains(u"major")) {
        _quality = u"major";
        if (kind == "major" || kind == "major-sixth") {
            implied = true;
        }
    } else if (kind.contains(u"minor")) {
        _quality = u"minor";
    } else if (kind.contains(u"dominant")) {
        _quality = u"dominant";
        implied = true;
        extension = 7;
    } else if (kind == "augmented-seventh") {
        _quality = u"augmented";
        extension = 7;
        extend = true;
    } else if (kind == "augmented") {
        _quality = u"augmented";
    } else if (kind == "half-diminished") {
        _quality = u"half-diminished";
        if (syms) {
            extension = 7;
            extend = true;
        }
    } else if (kind == "diminished-seventh") {
        _quality = u"diminished";
        extension = 7;
    } else if (kind == "diminished") {
        _quality = u"diminished";
    } else if (kind == "suspended-fourth") {
        _quality = u"major";
        implied = true;
        _modifierList << u"sus4";
    } else if (kind == "suspended-second") {
        _quality = u"major";
        implied = true;
        _modifierList << u"sus2";
    } else if (kind == "power") {
        _quality = u"major";
        implied = true;
        extension = 5;
    } else {
        _quality = kind;
    }

    // get extension info from kind
    if (kind.contains(u"seventh")) {
        extension = 7;
    } else if (kind.contains(u"ninth")) {
        extension = 9;
    } else if (kind.contains(u"11th")) {
        extension = 11;
    } else if (kind.contains(u"13th")) {
        extension = 13;
    } else if (kind.contains(u"sixth")) {
        extension = 6;
    }

    // get modifier info from degree list
    for (const HDegree& d : dl) {
        String mod;
        int v = d.value();
        switch (d.type()) {
        case HDegreeType::ADD:
        case HDegreeType::ALTER:
            switch (d.alter()) {
            case -1:    mod = u"b";
                break;
            case 1:     mod = u"#";
                break;
            case 0:     mod = u"add";
                break;
            }
            break;
        case HDegreeType::SUBTRACT:
            mod = u"no";
            break;
        case HDegreeType::UNDEF:
        default:
            break;
        }
        mod += String::number(v);
        if (mod == "add7" && kind.contains(u"suspended")) {
            _quality = u"dominant";
            implied = true;
            extension = 7;
            extend = true;
            mod = u"";
        } else if (mod == "add#7" && kind.contains(u"suspended")) {
            _quality = u"major";
            implied = false;
            extension = 7;
            extend = true;
            mod = u"";
        } else if (mod == "add9" && extend) {
            if (extension < 9) {
                extension = 9;
            }
            mod = u"";
        } else if (mod == "add11" && extend) {
            if (extension < 11) {
                extension = 11;
            }
            mod = u"";
        } else if (mod == "add13" && extend) {
            extension = 13;
            mod = u"";
        } else if (mod == "add9" && kind.contains(u"sixth")) {
            extension = 69;
            mod = u"";
        }
        if (mod != "") {
            _modifierList << mod;
        }
    }
    // convert no3,add[42] into sus[42]
    size_t no3 = _modifierList.indexOf(u"no3");
    if (no3 != mu::nidx) {
        size_t addn = _modifierList.indexOf(u"add4");
        if (addn == mu::nidx) {
            addn = _modifierList.indexOf(u"add2");
        }
        if (addn != mu::nidx) {
            String& s = _modifierList[addn];
            s.replace(u"add", u"sus");
            _modifierList.removeAt(no3);
        }
    }
    // convert kind=minor-seventh, degree=altb5 to kind=half-diminished (suppression of degree=altb comes later)
    if (kind == "minor-seventh" && _modifierList.size() == 1 && _modifierList.front() == "b5") {
        kind = u"half-diminished";
    }
    // force parens where necessary)
    if (!parens && extension == 0 && !_modifierList.empty()) {
        String firstMod = _modifierList.front();
        if (firstMod != "" && (firstMod.startsWith(u'#') || firstMod.startsWith(u'b'))) {
            parens = true;
        }
    }

    // record extension
    if (extension) {
        _extension = String::number(extension);
    }

    // validate kindText
    if (kindText != "" && kind != "none" && kind != "other") {
        ParsedChord validate;
        validate.parse(kindText, cl, false);
        // kindText should parse to produce same kind, no degrees
        if (validate._xmlKind != kind || !validate._xmlDegrees.empty()) {
            kindText = u"";
        }
    }

    // construct name & handle
    _name = u"";
    if (kindText != "") {
        if (_extension != "" && kind.contains(u"suspended")) {
            _name += _extension;
        }
        _name += kindText;
        if (extension == 69) {
            _name += u"9";
        }
    } else if (implied) {
        _name = _extension;
    } else {
        if (_quality == "major") {
            _name = syms ? u"^" : u"maj";
        } else if (_quality == "minor") {
            _name = syms ? u"-" : u"m";
        } else if (_quality == "augmented") {
            _name = syms ? u"+" : u"aug";
        } else if (_quality == "diminished") {
            _name = syms ? u"o" : u"dim";
        } else if (_quality == "half-diminished") {
            _name = syms ? u"0" : u"m7b5";
        } else {
            _name = _quality;
        }
        _name += _extension;
    }
    if (parens) {
        _name += u"(";
    }
    for (String mod : _modifierList) {
        mod.replace(u"major", u"maj");
        if (kindText != "" && kind.contains(u"suspended") && mod.startsWith(u"sus")) {
            continue;
        } else if (kindText != "" && kind == "major-minor" && mod.startsWith(u"maj")) {
            continue;
        }
        _name += mod;
    }
    if (parens) {
        _name += u")";
    }

    // parse name to construct handle & tokenList
    parse(_name, cl, true);

    // record original MusicXML
    _xmlKind = kind;
    _xmlText = kindText;
    _xmlSymbols = useSymbols;
    _xmlParens = useParens;
    for (const HDegree& d : dl) {
        if (kind == "half-diminished" && d.type() == HDegreeType::ALTER && d.alter() == -1 && d.value() == 5) {
            continue;
        }
        _xmlDegrees << d.text();
    }

    return _name;
}

//---------------------------------------------------------
//   position
//---------------------------------------------------------

double ChordList::position(const StringList& names, ChordTokenClass ctc) const
{
    String name = names.empty() ? u"" : names.front();
    switch (ctc) {
    case ChordTokenClass::EXTENSION:
        return _eadjust;
    case ChordTokenClass::MODIFIER: {
        Char c = name.isEmpty() ? name.at(0) : u'0';
        if (c.isDigit() || c.isPunct()) {
            return _madjust;
        } else {
            return 0.0;
        }
    }
    default:
        if (name == "o" || name == "0") {
            return _eadjust;
        } else {
            return 0.0;
        }
    }
}

//---------------------------------------------------------
//   renderList
//---------------------------------------------------------

const std::list<RenderAction>& ParsedChord::renderList(const ChordList* cl)
{
    // generate anew on each call,
    // in case chord list has changed since last time
    if (!_renderList.empty()) {
        _renderList.clear();
    }
    bool adjust = cl ? cl->autoAdjust() : false;
    for (const ChordToken& tok : _tokenList) {
        String n = tok.names.front();
        std::list<RenderAction> rl;
        std::list<ChordToken> definedTokens;
        bool found = false;
        // potential definitions for token
        if (cl) {
            for (const ChordToken& ct : cl->chordTokenList) {
                for (const String& ctn : ct.names) {
                    if (ctn == n) {
                        definedTokens.push_back(ct);
                    }
                }
            }
        }
        // find matching class, fallback on ChordTokenClass::ALL
        ChordTokenClass ctc = ChordTokenClass::ALL;
        for (const ChordToken& matchingTok : definedTokens) {
            if (tok.tokenClass == matchingTok.tokenClass) {
                rl = matchingTok.renderList;
                ctc = tok.tokenClass;
                found = true;
                break;
            } else if (matchingTok.tokenClass == ChordTokenClass::ALL) {
                rl = matchingTok.renderList;
                found = true;
            }
        }
        // check for adjustments
        // stop adjusting when first non-adjusted modifier found
        double p = adjust ? cl->position(tok.names, ctc) : 0.0;
        if (tok.tokenClass == ChordTokenClass::MODIFIER && p == 0.0) {
            adjust = false;
        }
        // build render list
        if (p != 0.0) {
            RenderAction m1 = RenderAction(RenderAction::RenderActionType::MOVE);
            m1.movex = 0.0;
            m1.movey = p;
            _renderList.push_back(m1);
        }
        if (found) {
            _renderList.insert(_renderList.end(), rl.begin(), rl.end());
        } else {
            // no definition for token, so render as literal
            RenderAction a(RenderAction::RenderActionType::SET);
            a.text = tok.names.front();
            _renderList.push_back(a);
        }
        if (p != 0.0) {
            RenderAction m2 = RenderAction(RenderAction::RenderActionType::MOVE);
            m2.movex = 0.0;
            m2.movey = -p;
            _renderList.push_back(m2);
        }
    }
    return _renderList;
}

//---------------------------------------------------------
//   addToken
//---------------------------------------------------------

void ParsedChord::addToken(String s, ChordTokenClass tc)
{
    if (s == "") {
        return;
    }
    ChordToken tok;
    tok.names << s;
    tok.tokenClass = tc;
    _tokenList.push_back(tok);
}

//---------------------------------------------------------
//   ChordDescription
//    this form is used when reading from file
//    a private id is assigned for id = 0
//---------------------------------------------------------

ChordDescription::ChordDescription(int i)
{
    if (!i) {
//            i = --(cl->privateID);
        i = --ChordList::privateID;
    }
    id = i;
    generated = false;
    renderListGenerated = false;
    exportOk = true;
}

//---------------------------------------------------------
//   ChordDescription
//    this form is used when generating from name
//    a private id is always assigned
//---------------------------------------------------------

ChordDescription::ChordDescription(const String& name)
{
    id = --ChordList::privateID;
    generated = true;
    names.push_back(name);
    renderListGenerated = false;
    exportOk = false;
}

//---------------------------------------------------------
//   complete
//    generate missing renderList and semantic (Xml) info
//---------------------------------------------------------

void ChordDescription::complete(ParsedChord* pc, const ChordList* cl)
{
    ParsedChord tempPc;
    if (!pc) {
        // generate parsed chord for its rendering & semantic (xml) info
        pc = &tempPc;
        String n;
        if (!names.empty()) {
            n = names.front();
        }
        pc->parse(n, cl);
    }
    parsedChords.push_back(*pc);
    if (renderList.empty() || renderListGenerated) {
        renderList = pc->renderList(cl);
        renderListGenerated = true;
    }
    if (xmlKind == "") {
        xmlKind = pc->xmlKind();
        xmlDegrees = pc->xmlDegrees();
    }
    // these fields are not read from chord description files
    // so get them from the parsed representation in all cases
    xmlText = pc->xmlText();
    xmlSymbols = pc->xmlSymbols();
    xmlParens = pc->xmlParens();
    if (chord.getKeys() == 0) {
        chord = HChord(pc->keys());
    }
    _quality = pc->quality();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(XmlReader& e)
{
    int ni = 0;
    id = e.attribute("id").toInt();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            String n = e.readText();
            // stack names for this file on top of the list
            names.insert(ni++, n);
        } else if (tag == "xml") {
            xmlKind = e.readText();
        } else if (tag == "degree") {
            xmlDegrees.push_back(e.readText());
        } else if (tag == "voicing") {
            chord = HChord(e.readText());
        } else if (tag == "render") {
            readRenderList(e.readText(), renderList);
            renderListGenerated = false;
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordDescription::write(XmlWriter& xml) const
{
    if (generated && !exportOk) {
        return;
    }
    if (id > 0) {
        xml.startElement("chord", { { "id", id } });
    } else {
        xml.startElement("chord");
    }
    for (const String& s : names) {
        xml.tag("name", s);
    }
    xml.tag("xml", xmlKind);
    xml.tag("voicing", chord.voicing());
    for (const String& s : xmlDegrees) {
        xml.tag("degree", s);
    }
    writeRenderList(xml, renderList, "render");
    xml.endElement();
}

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

int ChordList::privateID = -1000;

//---------------------------------------------------------
//   configureAutoAdjust
//---------------------------------------------------------

void ChordList::configureAutoAdjust(double emag, double eadjust, double mmag, double madjust)
{
    _emag = emag;
    _eadjust = eadjust;
    _mmag = mmag;
    _madjust = madjust;
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordList::read(XmlReader& e)
{
    int fontIdx = static_cast<int>(fonts.size());
    _autoAdjust = false;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "font") {
            ChordFont f;
            f.family = e.attribute("family", u"default");
            if (f.family == u"MuseJazz") {
                f.family = u"MuseJazz Text";
            }
            f.mag    = 1.0;
            f.fontClass = e.attribute("class");
            while (e.readNextStartElement()) {
                if (e.name() == "sym") {
                    ChordSymbol cs;
                    cs.fontIdx = fontIdx;
                    cs.name    = e.attribute("name");
                    cs.value   = e.attribute("value");
                    String code = e.attribute("code");
                    String symClass = e.attribute("class");
                    if (!code.empty()) {
                        bool ok = true;
                        char32_t val = code.toUInt(&ok, 0);
                        if (!ok) {
                            cs.code = 0;
                            cs.value = code;
                        } else if (Char::requiresSurrogates(val)) {
                            cs.code = 0;
                            cs.value = String::fromUcs4(val);
                        } else {
                            cs.code = val;
                            cs.value = String(cs.code);
                        }
                    } else {
                        cs.code = 0;
                    }
                    if (cs.value.empty()) {
                        cs.value = cs.name;
                    }
                    cs.name = symClass + cs.name;
                    symbols.insert({ cs.name, cs });
                    e.readNext();
                } else if (e.name() == "mag") {
                    f.mag = e.readDouble();
                } else {
                    e.unknown();
                }
            }
            if (_autoAdjust) {
                if (f.fontClass == "extension") {
                    f.mag *= _emag;
                } else if (f.fontClass == "modifier") {
                    f.mag *= _mmag;
                }
            }
            fonts.push_back(f);
            ++fontIdx;
        } else if (tag == "autoAdjust") {
            String nmag = e.attribute("mag");
            _nmag = nmag.toDouble();
            String nadjust = e.attribute("adjust");
            _nadjust = nadjust.toDouble();
            _autoAdjust = e.readBool();
        } else if (tag == "token") {
            ChordToken t;
            t.read(e);
            chordTokenList.push_back(t);
        } else if (tag == "chord") {
            int id = e.intAttribute("id");
            // if no id attribute (id == 0), then assign it a private id
            // user chords that match these ChordDescriptions will be treated as normal recognized chords
            // except that the id will not be written to the score file
            ChordDescription cd = (id && mu::contains(*this, id)) ? mu::take(*this, id) : ChordDescription(id);

            // record updated id
            id = cd.id;
            // read rest of description
            cd.read(e);
            // restore updated id
            cd.id = id;
            // throw away previously parsed chords
            cd.parsedChords.clear();
            // generate any missing info (including new parsed chords)
            cd.complete(0, this);
            // add to list
            insert({ id, cd });
        } else if (tag == "renderRoot") {
            readRenderList(e.readText(), renderListRoot);
        } else if (tag == "renderFunction") {
            readRenderList(e.readText(), renderListFunction);
        } else if (tag == "renderBase") {
            readRenderList(e.readText(), renderListBase);
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordList::write(XmlWriter& xml) const
{
    int fontIdx = 0;
    for (const ChordFont& f : fonts) {
        xml.startElement("font", { { "id", fontIdx }, { "family", f.family } });
        xml.tag("mag", f.mag);
        for (const auto& p : symbols) {
            const ChordSymbol& s = p.second;
            if (s.fontIdx == fontIdx) {
                if (s.code.isNull()) {
                    xml.tag("sym", { { "name", s.name }, { "value", s.value } });
                } else {
                    xml.tag("sym", { { "name", s.name }, { "code", String::number(s.code.unicode(), 16) } });
                }
            }
        }
        xml.endElement();
        ++fontIdx;
    }
    if (_autoAdjust) {
        xml.tag("autoAdjust", { { "mag", _nmag }, { "adjust", _nadjust } });
    }
    for (const ChordToken& t : chordTokenList) {
        t.write(xml);
    }
    if (!renderListRoot.empty()) {
        writeRenderList(xml, renderListRoot, "renderRoot");
    }
    if (!renderListFunction.empty()) {
        writeRenderList(xml, renderListFunction, "renderFunction");
    }
    if (!renderListBase.empty()) {
        writeRenderList(xml, renderListBase, "renderBase");
    }
    for (const auto& p : *this) {
        const ChordDescription& cd = p.second;
        cd.write(xml);
    }
}

//---------------------------------------------------------
//   read
//    read Chord List, return false on error
//---------------------------------------------------------

bool ChordList::read(const String& name)
{
//      LOGD("ChordList::read <%s>", muPrintable(name));
    io::path_t path;
    FileInfo ftest(name);
    if (ftest.isAbsolute()) {
        path = name;
    } else {
        path = configuration()->appDataPath() + "/styles/" + name;
    }

    // default to chords_std.xml
    if (!FileInfo::exists(path)) {
        path = configuration()->appDataPath() + "/styles/chords_std.xml";
    }

    if (name.isEmpty()) {
        return false;
    }
    File f(path);
    if (!f.open(IODevice::ReadOnly)) {
        LOGE() << "Cannot open chord description: " << f.filePath();
        return false;
    }

    return read(&f);
}

bool ChordList::read(IODevice* device)
{
    XmlReader e(device);

    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            // String version = e.attribute(String("version"));
            // StringList sl = version.split('.');
            // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
            read(e);
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   writeChordList
//---------------------------------------------------------

bool ChordList::write(const String& name) const
{
    FileInfo info(name);

    if (info.suffix().isEmpty()) {
        String path = info.filePath();
        path += u".xml";
        info = FileInfo(path);
    }

    File f(info.filePath());

    if (!f.open(IODevice::WriteOnly)) {
        LOGE() << "Failed open chord description: " << f.filePath();
        return false;
    }

    write(&f);

    return true;
}

bool ChordList::write(IODevice* device) const
{
    XmlWriter xml(device);
    xml.startDocument();
    xml.startElement("museScore", { { "version", MSC_VERSION } });

    write(xml);

    xml.endElement();

    return true;
}

//---------------------------------------------------------
//   loaded
//---------------------------------------------------------

bool ChordList::loaded() const
{
    // track whether a description file has been loaded
    // since chords.xml really doesn't load enough to stand alone,
    // we need a way to track when a "real" chord list has been loaded
    // for lack of anything better, key off renderListRoot
    return !renderListRoot.empty();
}

//---------------------------------------------------------
//   unload
//---------------------------------------------------------

void ChordList::unload()
{
    clear();
    symbols.clear();
    fonts.clear();
    renderListRoot.clear();
    renderListBase.clear();
    chordTokenList.clear();
    _autoAdjust = false;
}

const ChordDescription* ChordList::description(int id) const
{
    auto it = this->find(id);
    if (it == this->end()) {
        return nullptr;
    }
    return &it->second;
}

void ChordList::checkChordList(const MStyle& style)
{
    // make sure we have a chordlist
    if (!loaded()) {
        double emag = style.value(Sid::chordExtensionMag).toReal();
        double eadjust = style.value(Sid::chordExtensionAdjust).toReal();
        double mmag = style.value(Sid::chordModifierMag).toReal();
        double madjust = style.value(Sid::chordModifierAdjust).toReal();
        configureAutoAdjust(emag, eadjust, mmag, madjust);

        if (style.value(Sid::chordsXmlFile).toBool()) {
            read(u"chords.xml");
        }

        read(style.value(Sid::chordDescriptionFile).value<String>());
    }
}

//---------------------------------------------------------
//   print
//    only for debugging
//---------------------------------------------------------

void RenderAction::print() const
{
    static const char* names[] = {
        "SET", "MOVE", "PUSH", "POP",
        "NOTE", "ACCIDENTAL"
    };
    LOGD("%10s <%s> %f %f", names[int(type)], muPrintable(text), movex, movey);
}
}
