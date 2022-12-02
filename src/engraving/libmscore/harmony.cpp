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

#include "harmony.h"

#include "containers.h"
#include "translation.h"
#include "types/translatablestring.h"

#include "draw/fontmetrics.h"
#include "draw/types/brush.h"
#include "draw/types/pen.h"
#include "rw/writecontext.h"
#include "rw/xml.h"

#include "chordlist.h"
#include "fret.h"
#include "linkedobjects.h"
#include "measure.h"
#include "mscore.h"
#include "part.h"
#include "pitchspelling.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

String Harmony::harmonyName() const
{
    // Hack:
    const_cast<Harmony*>(this)->determineRootBaseSpelling();

    HChord hc = descr() ? descr()->chord : HChord();
    String s, r, e, b;

    if (_leftParen) {
        s = u"(";
    }

    if (_rootTpc != Tpc::TPC_INVALID) {
        r = tpc2name(_rootTpc, _rootSpelling, _rootCase);
    } else if (_harmonyType != HarmonyType::STANDARD) {
        r = _function;
    }

    if (_textName != "") {
        e = _textName;
        if (_harmonyType != HarmonyType::ROMAN) {
            e.remove(u'=');
        }
    } else if (!_degreeList.empty()) {
        hc.add(_degreeList);
        // try to find the chord in chordList
        const ChordDescription* newExtension = 0;
        const ChordList* cl = score()->chordList();
        for (const auto& p : *cl) {
            const ChordDescription& cd = p.second;
            if (cd.chord == hc && !cd.names.empty()) {
                newExtension = &cd;
                break;
            }
        }
        // now determine the chord name
        if (newExtension) {
            e = newExtension->names.front();
        } else {
            // not in table, fallback to using HChord.name()
            r = hc.name(_rootTpc);
            e = u"";
        }
    }

    if (_baseTpc != Tpc::TPC_INVALID) {
        b = u"/" + tpc2name(_baseTpc, _baseSpelling, _baseCase);
    }

    s += r + e + b;

    if (_rightParen) {
        s += u")";
    }

    return s;
}

//---------------------------------------------------------
//   rootName
//---------------------------------------------------------

String Harmony::rootName()
{
    determineRootBaseSpelling();
    return tpc2name(_rootTpc, _rootSpelling, _rootCase);
}

//---------------------------------------------------------
//   baseName
//---------------------------------------------------------

String Harmony::baseName()
{
    determineRootBaseSpelling();
    if (_baseTpc == Tpc::TPC_INVALID) {
        return rootName();
    }
    return tpc2name(_baseTpc, _baseSpelling, _baseCase);
}

bool Harmony::isRealizable() const
{
    return (_rootTpc != Tpc::TPC_INVALID)
           || (_harmonyType == HarmonyType::NASHVILLE);        // unable to fully check at for nashville at the moment
}

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
{
    if (_degreeList.empty()) {
        return;
    }

    HChord hc = descr() ? descr()->chord : HChord();

    hc.add(_degreeList);

// LOGD("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
// hc.print();
// _descr->chord.print();

    // try to find the chord in chordList
    const ChordList* cl = score()->chordList();
    for (const auto& p : *cl) {
        const ChordDescription& cd = p.second;
        if ((cd.chord == hc) && !cd.names.empty()) {
            LOGD("ResolveDegreeList: found in table as %s", muPrintable(cd.names.front()));
            _id = cd.id;
            _degreeList.clear();
            return;
        }
    }
    LOGD("ResolveDegreeList: not found in table");
}

//---------------------------------------------------------
//   chordSymbolStyle
//---------------------------------------------------------

const ElementStyle chordSymbolStyle {
    { Sid::harmonyPlacement, Pid::PLACEMENT },
    { Sid::minHarmonyDistance, Pid::MIN_DISTANCE },
    { Sid::harmonyVoiceLiteral, Pid::HARMONY_VOICE_LITERAL },
    { Sid::harmonyVoicing, Pid::HARMONY_VOICING },
    { Sid::harmonyDuration, Pid::HARMONY_DURATION }
};

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Segment* parent)
    : TextBase(ElementType::HARMONY, parent, TextStyleType::HARMONY_A, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    _rootTpc    = Tpc::TPC_INVALID;
    _baseTpc    = Tpc::TPC_INVALID;
    _rootSpelling = NoteSpellingType::STANDARD;
    _baseSpelling = NoteSpellingType::STANDARD;
    _rootCase   = NoteCaseType::CAPITAL;
    _baseCase   = NoteCaseType::CAPITAL;
    _rootRenderCase = NoteCaseType::CAPITAL;
    _baseRenderCase = NoteCaseType::CAPITAL;
    _id         = -1;
    _parsedForm = 0;
    _harmonyType = HarmonyType::STANDARD;
    _leftParen  = false;
    _rightParen = false;
    _play = true;
    _realizedHarmony = RealizedHarmony(this);
    initElementStyle(&chordSymbolStyle);
}

Harmony::Harmony(const Harmony& h)
    : TextBase(h)
{
    _rootTpc    = h._rootTpc;
    _baseTpc    = h._baseTpc;
    _rootSpelling = h._rootSpelling;
    _baseSpelling = h._baseSpelling;
    _rootCase   = h._rootCase;
    _baseCase   = h._baseCase;
    _rootRenderCase = h._rootRenderCase;
    _baseRenderCase = h._baseRenderCase;
    _id         = h._id;
    _leftParen  = h._leftParen;
    _rightParen = h._rightParen;
    _degreeList = h._degreeList;
    _parsedForm = h._parsedForm ? new ParsedChord(*h._parsedForm) : 0;
    _harmonyType = h._harmonyType;
    _textName   = h._textName;
    _userName   = h._userName;
    _function   = h._function;
    _play       = h._play;
    _realizedHarmony = h._realizedHarmony;
    _realizedHarmony.setHarmony(this);
    for (const TextSegment* s : h.textList) {
        TextSegment* ns = new TextSegment();
        ns->set(s->text, s->m_font, s->x, s->y, s->offset);
        textList.push_back(ns);
    }
}

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
{
    for (const TextSegment* ts : textList) {
        delete ts;
    }
    if (_parsedForm) {
        delete _parsedForm;
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Harmony::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);
    writeProperty(xml, Pid::HARMONY_TYPE);
    writeProperty(xml, Pid::PLAY);
    if (_leftParen) {
        xml.tag("leftParen");
    }
    if (_rootTpc != Tpc::TPC_INVALID || _baseTpc != Tpc::TPC_INVALID) {
        int rRootTpc = _rootTpc;
        int rBaseTpc = _baseTpc;
        if (staff()) {
            // parent can be a fret diagram
            Segment* segment = getParentSeg();
            Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
            const Interval& interval = part()->instrument(tick)->transpose();
            if (xml.context()->clipboardmode() && !score()->styleB(Sid::concertPitch) && interval.chromatic) {
                rRootTpc = transposeTpc(_rootTpc, interval, true);
                rBaseTpc = transposeTpc(_baseTpc, interval, true);
            }
        }
        if (rRootTpc != Tpc::TPC_INVALID) {
            xml.tag("root", rRootTpc);
            if (_rootCase != NoteCaseType::CAPITAL) {
                xml.tag("rootCase", static_cast<int>(_rootCase));
            }
        }
        if (_id > 0) {
            xml.tag("extension", _id);
        }
        // parser uses leading "=" as a hidden specifier for minor
        // this may or may not currently be incorporated into _textName
        String writeName = _textName;
        if (_parsedForm && _parsedForm->name().startsWith(u'=') && !writeName.startsWith(u'=')) {
            writeName = u"=" + writeName;
        }
        if (!writeName.isEmpty()) {
            xml.tag("name", writeName);
        }

        if (rBaseTpc != Tpc::TPC_INVALID) {
            xml.tag("base", rBaseTpc);
            if (_baseCase != NoteCaseType::CAPITAL) {
                xml.tag("baseCase", static_cast<int>(_baseCase));
            }
        }
        for (const HDegree& hd : _degreeList) {
            HDegreeType tp = hd.type();
            if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                xml.startElement("degree");
                xml.tag("degree-value", hd.value());
                xml.tag("degree-alter", hd.alter());
                switch (tp) {
                case HDegreeType::ADD:
                    xml.tag("degree-type", "add");
                    break;
                case HDegreeType::ALTER:
                    xml.tag("degree-type", "alter");
                    break;
                case HDegreeType::SUBTRACT:
                    xml.tag("degree-type", "subtract");
                    break;
                default:
                    break;
                }
                xml.endElement();
            }
        }
    } else {
        xml.tag("name", _textName);
    }
    if (!_function.isEmpty()) {
        xml.tag("function", _function);
    }
    TextBase::writeProperties(xml, false, true);
    //Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION
    //written by the above function call because they are part of element style
    if (_rightParen) {
        xml.tag("rightParen");
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Harmony::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "base") {
            setBaseTpc(e.readInt());
        } else if (tag == "baseCase") {
            _baseCase = static_cast<NoteCaseType>(e.readInt());
        } else if (tag == "extension") {
            setId(e.readInt());
        } else if (tag == "name") {
            _textName = e.readText();
        } else if (tag == "root") {
            setRootTpc(e.readInt());
        } else if (tag == "rootCase") {
            _rootCase = static_cast<NoteCaseType>(e.readInt());
        } else if (tag == "function") {
            _function = e.readText();
        } else if (tag == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            String degreeType;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "degree-value") {
                    degreeValue = e.readInt();
                } else if (t == "degree-alter") {
                    degreeAlter = e.readInt();
                } else if (t == "degree-type") {
                    degreeType = e.readText();
                } else {
                    e.unknown();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                LOGD("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                     degreeValue, degreeAlter, muPrintable(degreeType));
            } else {
                if (degreeType == "add") {
                    addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (tag == "leftParen") {
            _leftParen = true;
            e.readNext();
        } else if (tag == "rightParen") {
            _rightParen = true;
            e.readNext();
        } else if (readProperty(tag, e, Pid::POS_ABOVE)) {
        } else if (readProperty(tag, e, Pid::HARMONY_TYPE)) {
        } else if (readProperty(tag, e, Pid::PLAY)) {
        } else if (readProperty(tag, e, Pid::HARMONY_VOICE_LITERAL)) {
        } else if (readProperty(tag, e, Pid::HARMONY_VOICING)) {
        } else if (readProperty(tag, e, Pid::HARMONY_DURATION)) {
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }

    // TODO: now that we can render arbitrary chords,
    // we could try to construct a full representation from a degree list.
    // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
    // or constructed in the Chord Symbol Properties dialog.

    if (_rootTpc != Tpc::TPC_INVALID) {
        if (_id > 0) {
            // positive id will happen only for scores that were created with explicit chord lists
            // lookup id in chord list and generate new description if necessary
            getDescription();
        } else {
            // default case: look up by name
            // description will be found for any chord already read in this score
            // and we will generate a new one if necessary
            getDescription(_textName);
        }
    } else if (_textName == "") {
        // unrecognized chords prior to 2.0 were stored as text with markup
        // we need to strip away the markup
        // this removes any user-applied formatting,
        // but we no longer support user-applied formatting for chord symbols anyhow
        // with any luck, the resulting text will be parseable now, so give it a shot
        createLayout();
        String s = plainText();
        if (!s.isEmpty()) {
            setHarmony(s);
            return;
        }
        // empty text could also indicate a root-less slash chord ("/E")
        // we'll fall through and render it normally
    }

    // render chord from description (or _textName)
    render();
    setPlainText(harmonyName());
}

//---------------------------------------------------------
//   determineRootBaseSpelling
//---------------------------------------------------------

void Harmony::determineRootBaseSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase,
                                        NoteSpellingType& baseSpelling, NoteCaseType& baseCase)
{
    // spelling
    if (score()->styleB(Sid::useStandardNoteNames)) {
        rootSpelling = NoteSpellingType::STANDARD;
    } else if (score()->styleB(Sid::useGermanNoteNames)) {
        rootSpelling = NoteSpellingType::GERMAN;
    } else if (score()->styleB(Sid::useFullGermanNoteNames)) {
        rootSpelling = NoteSpellingType::GERMAN_PURE;
    } else if (score()->styleB(Sid::useSolfeggioNoteNames)) {
        rootSpelling = NoteSpellingType::SOLFEGGIO;
    } else if (score()->styleB(Sid::useFrenchNoteNames)) {
        rootSpelling = NoteSpellingType::FRENCH;
    }
    baseSpelling = rootSpelling;

    // case

    // always use case as typed if automatic capitalization is off
    if (!score()->styleB(Sid::automaticCapitalization)) {
        rootCase = _rootCase;
        baseCase = _baseCase;
        return;
    }

    // set default
    if (score()->styleB(Sid::allCapsNoteNames)) {
        rootCase = NoteCaseType::UPPER;
        baseCase = NoteCaseType::UPPER;
    } else {
        rootCase = NoteCaseType::CAPITAL;
        baseCase = NoteCaseType::CAPITAL;
    }

    // override for bass note
    if (score()->styleB(Sid::lowerCaseBassNotes)) {
        baseCase = NoteCaseType::LOWER;
    }

    // override for minor chords
    if (score()->styleB(Sid::lowerCaseMinorChords)) {
        const ChordDescription* cd = descr();
        String quality;
        if (cd) {
            // use chord description if possible
            // this is the usual case
            quality = cd->quality();
        } else if (_parsedForm) {
            // this happens on load of new chord list
            // for chord symbols that were added/edited since the score was loaded
            // or read aloud with screenreader
            // parsed form is usable even if out of date with respect to chord list
            quality = _parsedForm->quality();
        } else {
            // this happens on load of new chord list
            // for chord symbols that have not been edited since the score was loaded
            // we need to parse this chord for now to determine quality
            // but don't keep the parsed form around as we're not ready for it yet
            quality = parsedForm()->quality();
            delete _parsedForm;
            _parsedForm = 0;
        }
        if (quality == "minor" || quality == "diminished" || quality == "half-diminished") {
            rootCase = NoteCaseType::LOWER;
        }
    }
}

//---------------------------------------------------------
//   determineRootBaseSpelling
//---------------------------------------------------------

void Harmony::determineRootBaseSpelling()
{
    determineRootBaseSpelling(_rootSpelling, _rootRenderCase,
                              _baseSpelling, _baseRenderCase);
}

//---------------------------------------------------------
//   convertNote
//    convert something like "C#" into tpc 21
//---------------------------------------------------------

static int convertNote(const String& s, NoteSpellingType noteSpelling, NoteCaseType& noteCase, size_t& idx)
{
    bool useGerman = false;
    bool useSolfeggio = false;
    static const int spellings[] = {
        // bb  b   -   #  ##
        0,  7, 14, 21, 28,      // C
        2,  9, 16, 23, 30,      // D
        4, 11, 18, 25, 32,      // E
        -1,  6, 13, 20, 27,     // F
        1,  8, 15, 22, 29,      // G
        3, 10, 17, 24, 31,      // A
        5, 12, 19, 26, 33,      // B
    };
    if (s == "") {
        return Tpc::TPC_INVALID;
    }
    noteCase = s.at(0).isLower() ? NoteCaseType::LOWER : NoteCaseType::CAPITAL;
    int acci;
    switch (noteSpelling) {
    case NoteSpellingType::SOLFEGGIO:
    case NoteSpellingType::FRENCH:
        useSolfeggio = true;
        if (s.startsWith(u"sol", mu::CaseInsensitive)) {
            acci = 3;
        } else {
            acci = 2;
        }
        break;
    case NoteSpellingType::GERMAN:
    case NoteSpellingType::GERMAN_PURE:
        useGerman = true;
    // fall through
    default:
        acci = 1;
    }
    idx = acci;
    int alter = 0;
    size_t n = s.size();
    String acc = s.right(n - acci);
    if (acc != "") {
        if (acc.startsWith(u"bb")) {
            alter = -2;
            idx += 2;
        } else if (acc.startsWith(u"b")) {
            alter = -1;
            idx += 1;
        } else if (useGerman && acc.startsWith(u"eses")) {
            alter = -2;
            idx += 4;
        } else if (useGerman && (acc.startsWith(u"ses") || acc.startsWith(u"sas"))) {
            alter = -2;
            idx += 3;
        } else if (useGerman && acc.startsWith(u"es")) {
            alter = -1;
            idx += 2;
        } else if (useGerman && acc.startsWith(u"s") && !acc.startsWith(u"su")) {
            alter = -1;
            idx += 1;
        } else if (acc.startsWith(u"##")) {
            alter = 2;
            idx += 2;
        } else if (acc.startsWith(u"x")) {
            alter = 2;
            idx += 1;
        } else if (acc.startsWith(u"#")) {
            alter = 1;
            idx += 1;
        } else if (useGerman && acc.startsWith(u"isis")) {
            alter = 2;
            idx += 4;
        } else if (useGerman && acc.startsWith(u"is")) {
            alter = 1;
            idx += 2;
        }
    }
    int r;
    if (useGerman) {
        switch (s.at(0).toLower().toAscii()) {
        case 'c':   r = 0;
            break;
        case 'd':   r = 1;
            break;
        case 'e':   r = 2;
            break;
        case 'f':   r = 3;
            break;
        case 'g':   r = 4;
            break;
        case 'a':   r = 5;
            break;
        case 'h':   r = 6;
            break;
        case 'b':
            if (alter && alter != -1) {
                return Tpc::TPC_INVALID;
            }
            r = 6;
            alter = -1;
            break;
        default:
            return Tpc::TPC_INVALID;
        }
    } else if (useSolfeggio) {
        if (s.size() < 2) {
            return Tpc::TPC_INVALID;
        }
        if (s.at(1).isUpper()) {
            noteCase = NoteCaseType::UPPER;
        }
        String ss = s.toLower().left(2);
        if (ss == "do") {
            r = 0;
        } else if (ss == "re" || ss == "ré") {
            r = 1;
        } else if (ss == "mi") {
            r = 2;
        } else if (ss == "fa") {
            r = 3;
        } else if (ss == "so") {    // sol, but only check first 2 characters
            r = 4;
        } else if (ss == "la") {
            r = 5;
        } else if (ss == "si") {
            r = 6;
        } else {
            return Tpc::TPC_INVALID;
        }
    } else {
        switch (s.at(0).toLower().toAscii()) {
        case 'c':   r = 0;
            break;
        case 'd':   r = 1;
            break;
        case 'e':   r = 2;
            break;
        case 'f':   r = 3;
            break;
        case 'g':   r = 4;
            break;
        case 'a':   r = 5;
            break;
        case 'b':   r = 6;
            break;
        default:    return Tpc::TPC_INVALID;
        }
    }
    r = spellings[r * 5 + alter + 2];
    return r;
}

//---------------------------------------------------------
//   parseHarmony
//    determine root and bass tpc & case
//    compare body of chordname against chord list
//    return true if chord is recognized
//---------------------------------------------------------

const ChordDescription* Harmony::parseHarmony(const String& ss, int* root, int* base, bool syntaxOnly)
{
    _id = -1;
    if (_parsedForm) {
        delete _parsedForm;
        _parsedForm = 0;
    }
    _textName.clear();
    bool useLiteral = false;
    if (ss.endsWith(' ')) {
        useLiteral = true;
    }

    if (_harmonyType == HarmonyType::ROMAN) {
        _userName = ss;
        _textName = ss;
        *root = Tpc::TPC_INVALID;
        *base = Tpc::TPC_INVALID;
        return 0;
    }

    // pre-process for parentheses
    String s = ss.simplified();
    if ((_leftParen = s.startsWith('('))) {
        s.remove(0, 1);
    }
    if ((_rightParen = (s.endsWith(')') && s.count('(') < s.count(')')))) {
        s.remove(s.size() - 1, 1);
    }
    if (_leftParen || _rightParen) {
        s = s.simplified();         // in case of spaces inside parentheses
    }
    if (s.isEmpty()) {
        return 0;
    }

    // pre-process for lower case minor chords
    bool preferMinor;
    if (score()->styleB(Sid::lowerCaseMinorChords) && s.at(0).isLower()) {
        preferMinor = true;
    } else {
        preferMinor = false;
    }

    if (_harmonyType == HarmonyType::NASHVILLE) {
        int n = 0;
        if (s.at(0).isDigit()) {
            n = 1;
        } else if (s.at(1).isDigit()) {
            n = 2;
        }
        _function = s.mid(0, n);
        s = s.mid(n);
        *root = Tpc::TPC_INVALID;
        *base = Tpc::TPC_INVALID;
    } else {
        determineRootBaseSpelling();
        size_t idx;
        int r = convertNote(s, _rootSpelling, _rootCase, idx);
        if (r == Tpc::TPC_INVALID) {
            if (s.at(0) == '/') {
                idx = 0;
            } else {
                LOGD("failed <%s>", muPrintable(ss));
                _userName = s;
                _textName = s;
                return 0;
            }
        }
        *root = r;
        *base = Tpc::TPC_INVALID;
        size_t slash = s.lastIndexOf(u'/');
        if (slash != mu::nidx) {
            String bs = s.mid(slash + 1).simplified();
            s = s.mid(idx, slash - idx).simplified();
            size_t idx2;
            *base = convertNote(bs, _baseSpelling, _baseCase, idx2);
            if (idx2 != bs.size()) {
                *base = Tpc::TPC_INVALID;
            }
            if (*base == Tpc::TPC_INVALID) {
                // if what follows after slash is not (just) a TPC
                // then reassemble chord and try to parse with the slash
                s = s + u"/" + bs;
            }
        } else {
            s = s.mid(idx);         // don't simplify; keep leading space before extension if present
        }
    }

    _userName = s;
    const ChordList* cl = score()->chordList();
    const ChordDescription* cd = 0;
    if (useLiteral) {
        cd = descr(s);
    } else {
        _parsedForm = new ParsedChord();
        _parsedForm->parse(s, cl, syntaxOnly, preferMinor);
        // parser prepends "=" to name of implied minor chords
        // use this here as well
        if (preferMinor) {
            s = _parsedForm->name();
        }
        // look up to see if we already have a descriptor (chord has been used before)
        cd = descr(s, _parsedForm);
    }
    if (cd) {
        // descriptor found; use its information
        _id = cd->id;
        if (!cd->names.empty()) {
            _textName = cd->names.front();
        }
    } else {
        // no descriptor yet; just set textname
        // we will generate descriptor later if necessary (when we are done editing this chord)
        _textName = s;
    }
    return cd;
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Harmony::startEdit(EditData& ed)
{
    if (!textList.empty()) {
        // convert chord symbol to plain text
        setPlainText(harmonyName());
        // clear rendering
        for (const TextSegment* t : textList) {
            delete t;
        }
        textList.clear();
    }

    // layout as text, without position reset
    TextBase::layout1();
    triggerLayout();

    TextBase::startEdit(ed);
}

bool Harmony::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    if (ed.key == Key_Semicolon || ed.key == Key_Colon) {
        return false;
    }

    if (ed.key == Key_Return || ed.key == Key_Enter) {
        // This "edit" is actually handled in NotationInteraction::editElement
        return true;
    }

    return TextBase::isEditAllowed(ed);
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Harmony::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    bool rv = TextBase::edit(ed);

    // layout as text, without position reset
    TextBase::layout1();
    triggerLayout();

    // check spelling
    int root = TPC_INVALID;
    int base = TPC_INVALID;
    String str = xmlText();
    _isMisspelled = !str.isEmpty()
                    && !parseHarmony(str, &root, &base, true)
                    && root == TPC_INVALID
                    && _harmonyType == HarmonyType::STANDARD;
    if (_isMisspelled) {
        LOGD("bad spell");
    }

    return rv;
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit(EditData& ed)
{
    // get plain text
    String s = plainText();

    // if user explicitly added symbols to the text,
    // convert them back to their respective replacement texts
    if (harmonyType() != HarmonyType::ROMAN) {
        s.replace(u"\u1d12b", u"bb");     // double-flat
        s.replace(u"\u266d",  u"b");      // flat
        s.replace(u"\ue260",  u"b");      // flat
        // do not replace natural sign
        // (right now adding the symbol explicitly is the only way to force a natural sign to appear at all)
        //s.replace("\u266e",  "n");  // natural, if one day we support that too
        //s.replace("\ue261",  "n");  // natural, if one day we support that too
        s.replace(u"\u266f",  u"#");      // sharp
        s.replace(u"\ue262",  u"#");      // sharp
        s.replace(u"\u1d12a", u"x");      // double-sharp
        s.replace(u"\u0394",  u"^");      // &Delta;
        s.replace(u"\u00d0",  u"o");      // &deg;
        s.replace(u"\u00f8",  u"0");      // &oslash;
        s.replace(u"\u00d8",  u"0");      // &Oslash;
    } else {
        s.replace(u"\ue260",  u"\u266d");         // flat
        s.replace(u"\ue261",  u"\u266e");         // natural
        s.replace(u"\ue262",  u"\u266f");         // sharp
    }

    //play chord on edit and set dirty
    score()->setPlayChord(true);
    _realizedHarmony.setDirty(true);

    setHarmony(s);
    setPlainText(harmonyName());

    // disable spell check
    _isMisspelled = false;

    if (links()) {
        for (EngravingObject* e : *links()) {
            if (e == this) {
                continue;
            }
            Harmony* h = toHarmony(e);
            // transpose if necessary
            // at this point chord will already have been rendered in same key as original
            // (as a result of TextBase::endEdit() calling setText() for linked elements)
            // we may now need to change the TPC's and the text, and re-render
            if (score()->styleB(Sid::concertPitch) != h->score()->styleB(Sid::concertPitch)) {
                Part* partDest = h->part();
                Segment* segment = getParentSeg();
                Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
                Interval interval = partDest->instrument(tick)->transpose();
                if (!interval.isZero()) {
                    if (!h->score()->styleB(Sid::concertPitch)) {
                        interval.flip();
                    }
                    int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                    int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                    //score()->undoTransposeHarmony(h, rootTpc, baseTpc);
                    h->setRootTpc(rootTpc);
                    h->setBaseTpc(baseTpc);
                    h->setPlainText(h->harmonyName());
                    h->setHarmony(h->plainText());
                    h->triggerLayout();
                }
            }
        }
    }

    TextBase::endEdit(ed);
}

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void Harmony::setHarmony(const String& s)
{
    int r, b;
    const ChordDescription* cd = parseHarmony(s, &r, &b);
    if (!cd && _parsedForm && _parsedForm->parseable()) {
        // our first time encountering this chord
        // generate a descriptor and use it
        cd = generateDescription();
        _id = cd->id;
    }
    if (cd) {
        setRootTpc(r);
        setBaseTpc(b);
        render();
    } else {
        // unparseable chord, render as plain text
        for (const TextSegment* ts : textList) {
            delete ts;
        }
        textList.clear();
        setRootTpc(Tpc::TPC_INVALID);
        setBaseTpc(Tpc::TPC_INVALID);
        _id = -1;
        render();
    }
}

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

double Harmony::baseLine() const
{
    return (textList.empty()) ? TextBase::baseLine() : 0.0;
}

//---------------------------------------------------------
//   text
//---------------------------------------------------------

String HDegree::text() const
{
    if (_type == HDegreeType::UNDEF) {
        return String();
    }
    String degree;
    switch (_type) {
    case HDegreeType::UNDEF: break;
    case HDegreeType::ADD:         degree = u"add";
        break;
    case HDegreeType::ALTER:       degree = u"alt";
        break;
    case HDegreeType::SUBTRACT:    degree = u"sub";
        break;
    }

    switch (_alter) {
    case -1:          degree += u"b";
        break;
    case 1:           degree += u"#";
        break;
    default:          break;
    }
    String s = String::number(_value);
    String ss = degree + s;
    return ss;
}

//---------------------------------------------------------
//   findInSeg
///   find a Harmony in a given segment on the same track as this harmony.
///
///   returns 0 if there is none
//---------------------------------------------------------

Harmony* Harmony::findInSeg(Segment* seg) const
{
    // Find harmony as parent of fret diagram on same track
    EngravingItem* fde = seg->findAnnotation(ElementType::FRET_DIAGRAM, track(), track());
    if (fde) {
        FretDiagram* fd = toFretDiagram(fde);
        if (fd->harmony()) {
            return toHarmony(fd->harmony());
        }
    }

    // Find harmony on same track
    EngravingItem* e = seg->findAnnotation(ElementType::HARMONY, track(), track());
    if (e) {
        return toHarmony(e);
    }
    return nullptr;
}

//---------------------------------------------------------
//   getParentSeg
///   gets the parent segment of this harmony
//---------------------------------------------------------

Segment* Harmony::getParentSeg() const
{
    Segment* seg;
    if (explicitParent()->isFretDiagram()) {
        // When this harmony is the child of a fret diagram, we need to go up twice
        // to get to the parent seg.
        seg = toSegment(explicitParent()->explicitParent());
    } else {
        seg = toSegment(explicitParent());
    }
    return seg;
}

//---------------------------------------------------------
//   findNext
///   find the next Harmony in the score
///
///   returns 0 if there is none
//---------------------------------------------------------

Harmony* Harmony::findNext() const
{
    Segment* cur = getParentSeg()->next1();
    while (cur) {
        Harmony* h = findInSeg(cur);
        if (h) {
            return h;
        }
        cur = cur->next1();
    }
    return 0;
}

//---------------------------------------------------------
//   findPrev
///   find the previous Harmony in the score
///
///   returns 0 if there is none
//---------------------------------------------------------

Harmony* Harmony::findPrev() const
{
    Segment* cur = getParentSeg()->prev1();
    while (cur) {
        Harmony* h = findInSeg(cur);
        if (h) {
            return h;
        }
        cur = cur->prev1();
    }
    return 0;
}

//---------------------------------------------------------
//   ticksTillNext
///   finds ticks until the next chord symbol or end of score
///
///   utick is our current playback position to start from
///
///   stopAtMeasureEnd being set to true will have the loop
///   stop at measure end.
//---------------------------------------------------------
Fraction Harmony::ticksTillNext(int utick, bool stopAtMeasureEnd) const
{
    Segment* seg = getParentSeg();
    Fraction duration = seg->ticks();
    Segment* cur = seg->next();
    auto rsIt = score()->repeatList().findRepeatSegmentFromUTick(utick);

    Measure const* currentMeasure = seg->measure();
    Measure const* endMeasure = (stopAtMeasureEnd) ? currentMeasure : (*rsIt)->lastMeasure();
    Harmony const* nextHarmony = nullptr;

    do {
        // Loop over segments of this measure
        while (cur) {
            //find harmony on same track
            EngravingItem* e = cur->findAnnotation(ElementType::HARMONY, track(), track());
            if (e) {
                nextHarmony = toHarmony(e);
            } else {
                // no harmony; look for fret diagram
                e = cur->findAnnotation(ElementType::FRET_DIAGRAM, track(), track());
                if (e) {
                    nextHarmony = toFretDiagram(e)->harmony();
                }
            }
            if (nextHarmony != nullptr) {
                //we have found the next chord symbol
                break;
            }
            //keep adding the duration of the current segment
            //in case we are not able to find a next
            //chord symbol
            duration += cur->ticks();
            cur = cur->next();
        }
        // Move segment reference to next measure
        if (currentMeasure != endMeasure) {
            currentMeasure = currentMeasure->nextMeasure();
            cur = (currentMeasure) ? currentMeasure->first() : nullptr;
        } else {
            // End of repeatSegment or search boundary reached
            if (stopAtMeasureEnd) {
                break;
            } else {
                // move to next RepeatSegment
                if (++rsIt != score()->repeatList().end()) {
                    currentMeasure = (*rsIt)->firstMeasure();
                    endMeasure     = (*rsIt)->lastMeasure();
                    cur = currentMeasure->first();
                }
            }
        }
    } while ((nextHarmony == nullptr) && (cur != nullptr));

    return duration;
}

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const String& kind, const std::list<HDegree>& dl)
{
    StringList degrees;

    for (const HDegree& d : dl) {
        degrees.push_back(d.text());
    }

    String lowerCaseKind = kind.toLower();
    const ChordList* cl = score()->chordList();
    for (const auto& p : *cl) {
        const ChordDescription& cd = p.second;
        String k     = cd.xmlKind;
        String lowerCaseK = k.toLower();     // required for xmlKind Tristan
        StringList d = cd.xmlDegrees;
        if ((lowerCaseKind == lowerCaseK) && (d == degrees)) {
//                  LOGD("harmony found in db: %s %s -> %d", muPrintable(kind), muPrintable(degrees), cd->id);
            return &cd;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string only
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const String& kind)
{
    String lowerCaseKind = kind.toLower();
    const ChordList* cl = score()->chordList();
    for (const auto& p : *cl) {
        const ChordDescription& cd = p.second;
        if (lowerCaseKind == cd.xmlKind) {
            return &cd;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   fromXml
//    construct harmony directly from XML
//    build name first
//    then generate chord description from that
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const String& kind, const String& kindText, const String& symbols, const String& parens,
                                         const std::list<HDegree>& dl)
{
    ParsedChord* pc = new ParsedChord;
    _textName = pc->fromXml(kind, kindText, symbols, parens, dl, score()->chordList());
    _parsedForm = pc;
    const ChordDescription* cd = getDescription(_textName, pc);
    return cd;
}

//---------------------------------------------------------
//   descr
//    look up id in chord list
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* Harmony::descr() const
{
    return score()->chordList()->description(_id);
}

//---------------------------------------------------------
//   descr
//    look up name in chord list
//    optionally look up by parsed chord as fallback
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* Harmony::descr(const String& name, const ParsedChord* pc) const
{
    const ChordList* cl = score()->chordList();
    const ChordDescription* match = 0;
    if (cl) {
        for (const auto& p : *cl) {
            const ChordDescription& cd = p.second;
            for (const String& s : cd.names) {
                if (s == name) {
                    return &cd;
                } else if (pc) {
                    for (const ParsedChord& sParsed : cd.parsedChords) {
                        if (sParsed == *pc) {
                            match = &cd;
                        }
                    }
                }
            }
        }
    }
    // exact match failed, so fall back on parsed match if one was found
    return match;
}

//---------------------------------------------------------
//   getDescription
//    look up id in chord list
//    return chord description if found
//    if not found, and chord is parseable,
//    generate a new chord description
//    and add to chord list
//---------------------------------------------------------

const ChordDescription* Harmony::getDescription()
{
    const ChordDescription* cd = descr();
    if (cd && !cd->names.empty()) {
        _textName = cd->names.front();
    } else if (_textName != "") {
        cd = generateDescription();
        _id = cd->id;
    }
    return cd;
}

//---------------------------------------------------------
//   getDescription
//    same but lookup by name and optionally parsed chord
//---------------------------------------------------------

const ChordDescription* Harmony::getDescription(const String& name, const ParsedChord* pc)
{
    const ChordDescription* cd = descr(name, pc);
    if (cd) {
        _id = cd->id;
    } else {
        cd = generateDescription();
        _id = cd->id;
    }
    return cd;
}

//---------------------------------------------------------
//   getRealizedHarmony
//    get realized harmony or create one for the current symbol
//    also updates the realized harmony and accounts for
//    transposition. RealizedHarmony objects cannot be cached
//    since the notes generated depends on context rather than
//    just root, bass, chord symbol, and voicing.
//---------------------------------------------------------

const RealizedHarmony& Harmony::getRealizedHarmony() const
{
    Staff* st = staff();
    int capo = st->capo(tick()) - 1;
    int offset = (capo < 0 ? 0 : capo);   //semitone offset for pitch adjustment
    Interval interval = st->part()->instrument(tick())->transpose();
    if (!score()->styleB(Sid::concertPitch)) {
        offset += interval.chromatic;
    }

    //Adjust for Nashville Notation, might be temporary
    // TODO: set dirty on add/remove of keysig
    if (_harmonyType == HarmonyType::NASHVILLE && !_realizedHarmony.valid()) {
        Key key = staff()->key(tick());
        //parse root
        int rootTpc = function2Tpc(_function, key);

        //parse bass
        size_t slash = _textName.lastIndexOf('/');
        int bassTpc;
        if (slash == mu::nidx) {
            bassTpc = Tpc::TPC_INVALID;
        } else {
            bassTpc = function2Tpc(_textName.mid(slash + 1), key);
        }
        _realizedHarmony.update(rootTpc, bassTpc, offset);
    } else {
        _realizedHarmony.update(_rootTpc, _baseTpc, offset);
    }
    return _realizedHarmony;
}

//---------------------------------------------------------
//   realizedHarmony
//    get realized harmony or create one for the current symbol
//    without updating the realized harmony
//---------------------------------------------------------

RealizedHarmony& Harmony::realizedHarmony()
{
    return _realizedHarmony;
}

//---------------------------------------------------------
//   generateDescription
//    generate new chord description from _textName
//    add to chord list using private id
//---------------------------------------------------------

const ChordDescription* Harmony::generateDescription()
{
    ChordList* cl = score()->chordList();
    ChordDescription cd(_textName);
    cd.complete(_parsedForm, cl);
    // remove parsed chord from description
    // so we will only match it literally in the future
    cd.parsedChords.clear();
    cl->insert({ cd.id, cd });
    return &cl->at(cd.id);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Harmony::layout()
{
    if (!explicitParent()) {
        setPos(0.0, 0.0);
        setOffset(0.0, 0.0);
        layout1();
        return;
    }
    //if (isStyled(Pid::OFFSET))
    //      setOffset(propertyDefault(Pid::OFFSET).value<PointF>());

    layout1();
    setPos(calculateBoundingRect());
}

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Harmony::layout1()
{
    if (isLayoutInvalid()) {
        createLayout();
    }
    if (textBlockList().empty()) {
        textBlockList().push_back(TextBlock());
    }
    calculateBoundingRect();
    if (hasFrame()) {
        layoutFrame();
    }
    score()->addRefresh(canvasBoundingRect());
}

//---------------------------------------------------------
//   calculateBoundingRect
//---------------------------------------------------------

PointF Harmony::calculateBoundingRect()
{
    const double ypos = (placeBelow() && staff()) ? staff()->height() : 0.0;
    const FretDiagram* fd = (explicitParent() && explicitParent()->isFretDiagram()) ? toFretDiagram(explicitParent()) : nullptr;
    const double cw = symWidth(SymId::noteheadBlack);

    double newPosX = 0.0;
    double newPosY = 0.0;

    if (textList.empty()) {
        TextBase::layout1();

        if (fd) {
            newPosY = this->ypos();
        } else {
            newPosY = ypos - ((align() == AlignV::BOTTOM) ? _harmonyHeight - bbox().height() : 0.0);
        }
    } else {
        RectF bb;
        for (TextSegment* ts : textList) {
            bb.unite(ts->tightBoundingRect().translated(ts->x, ts->y));
        }

        double xx = 0.0;
        switch (align().horizontal) {
        case AlignH::LEFT:
            xx = -bb.left();
            break;
        case AlignH::HCENTER:
            xx = -(bb.center().x());
            break;
        case AlignH::RIGHT:
            xx = -bb.right();
            break;
        }

        double yy = -bb.y();      // Align::TOP
        if (align() == AlignV::VCENTER) {
            yy = -bb.y() / 2.0;
        } else if (align() == AlignV::BASELINE) {
            yy = 0.0;
        } else if (align() == AlignV::BOTTOM) {
            yy = -bb.height() - bb.y();
        }

        if (fd) {
            newPosY = ypos - yy - score()->styleMM(Sid::harmonyFretDist);
        } else {
            newPosY = ypos;
        }

        for (TextSegment* ts : textList) {
            ts->offset = PointF(xx, yy);
        }

        setbbox(bb.translated(xx, yy));
        _harmonyHeight = bbox().height();
    }

    if (fd) {
        switch (align().horizontal) {
        case AlignH::LEFT:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = fd->centerX();
            break;
        case AlignH::RIGHT:
            newPosX = fd->rightX();
            break;
        }
    } else {
        switch (align().horizontal) {
        case AlignH::LEFT:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = cw * 0.5;
            break;
        case AlignH::RIGHT:
            newPosX = cw;
            break;
        }
    }

    return PointF(newPosX, newPosY);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Harmony::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    // painter->setPen(curColor());
    if (textList.empty()) {
        TextBase::draw(painter);
        return;
    }
    if (hasFrame()) {
        if (frameWidth().val() != 0.0) {
            Color color = frameColor();
            Pen pen(color, frameWidth().val() * spatium(), PenStyle::SolidLine,
                    PenCapStyle::SquareCap, PenJoinStyle::MiterJoin);
            painter->setPen(pen);
        } else {
            painter->setNoPen();
        }
        Color bg(bgColor());
        painter->setBrush(bg.alpha() ? Brush(bg) : BrushStyle::NoBrush);
        if (circle()) {
            painter->drawArc(frame, 0, 5760);
        } else {
            int r2 = frameRound();
            if (r2 > 99) {
                r2 = 99;
            }
            painter->drawRoundedRect(frame, frameRound(), r2);
        }
    }
    painter->setBrush(BrushStyle::NoBrush);
    Color color = textColor();
    painter->setPen(color);
    for (const TextSegment* ts : textList) {
        mu::draw::Font f(ts->m_font);
        f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
#ifndef Q_OS_MACOS
        TextBase::drawTextWorkaround(painter, f, ts->pos(), ts->text);
#else
        painter->setFont(f);
        painter->drawText(ts->pos(), ts->text);
#endif
    }
}

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void Harmony::drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling)
{
    TextBase::drawEditMode(p, ed, currentViewScaling);

    mu::draw::Color originalColor = color();
    if (_isMisspelled) {
        setColor(engravingConfiguration()->criticalColor());
        setSelected(false);
    }
    PointF pos(canvasPos());
    p->translate(pos);
    TextBase::draw(p);
    p->translate(-pos);
    if (_isMisspelled) {
        setColor(originalColor);
        setSelected(true);
    }
}

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

TextSegment::TextSegment(const String& s, const mu::draw::Font& f, double x, double y)
{
    set(s, f, x, y, PointF());
    select = false;
}

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double TextSegment::width() const
{
    return mu::draw::FontMetrics::width(m_font, text);
}

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

RectF TextSegment::boundingRect() const
{
    return mu::draw::FontMetrics::boundingRect(m_font, text);
}

//---------------------------------------------------------
//   tightBoundingRect
//---------------------------------------------------------

RectF TextSegment::tightBoundingRect() const
{
    return mu::draw::FontMetrics::tightBoundingRect(m_font, text);
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextSegment::set(const String& s, const mu::draw::Font& f, double _x, double _y, PointF _offset)
{
    m_font   = f;
    x      = _x;
    y      = _y;
    offset = _offset;
    setText(s);
}

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void Harmony::render(const String& s, double& x, double& y)
{
    int fontIdx = 0;
    if (!s.isEmpty()) {
        mu::draw::Font f = _harmonyType != HarmonyType::ROMAN ? fontList[fontIdx] : font();
        TextSegment* ts = new TextSegment(s, f, x, y);
        textList.push_back(ts);
        x += ts->width();
    }
}

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void Harmony::render(const std::list<RenderAction>& renderList, double& x, double& y, int tpc, NoteSpellingType noteSpelling,
                     NoteCaseType noteCase)
{
    ChordList* chordList = score()->chordList();
    std::stack<PointF> stack;
    int fontIdx    = 0;
    double _spatium = spatium();
    double mag      = magS();

// LOGD("===");
    for (const RenderAction& a : renderList) {
// a.print();
        if (a.type == RenderAction::RenderActionType::SET) {
            TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
            ChordSymbol cs = chordList->symbol(a.text);
            if (cs.isValid()) {
                ts->m_font = fontList[cs.fontIdx];
                ts->setText(cs.value);
            } else {
                ts->setText(a.text);
            }
            if (_harmonyType == HarmonyType::NASHVILLE) {
                double nmag = chordList->nominalMag();
                ts->m_font.setPointSizeF(ts->m_font.pointSizeF() * nmag);
            }
            textList.push_back(ts);
            x += ts->width();
        } else if (a.type == RenderAction::RenderActionType::MOVE) {
            x += a.movex * mag * _spatium * .2;
            y += a.movey * mag * _spatium * .2;
        } else if (a.type == RenderAction::RenderActionType::PUSH) {
            stack.push(PointF(x, y));
        } else if (a.type == RenderAction::RenderActionType::POP) {
            if (!stack.empty()) {
                PointF pt = stack.top();
                stack.pop();
                x = pt.x();
                y = pt.y();
            } else {
                LOGD("RenderAction::RenderActionType::POP: stack empty");
            }
        } else if (a.type == RenderAction::RenderActionType::NOTE) {
            String c;
            AccidentalVal acc;
            if (tpcIsValid(tpc)) {
                tpc2name(tpc, noteSpelling, noteCase, c, acc);
            } else if (_function.size() > 0) {
                c = _function.at(_function.size() - 1);
            }
            TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
            String lookup = u"note" + c;
            ChordSymbol cs = chordList->symbol(lookup);
            if (!cs.isValid()) {
                cs = chordList->symbol(c);
            }
            if (cs.isValid()) {
                ts->m_font = fontList[cs.fontIdx];
                ts->setText(cs.value);
            } else {
                ts->setText(c);
            }
            textList.push_back(ts);
            x += ts->width();
        } else if (a.type == RenderAction::RenderActionType::ACCIDENTAL) {
            String c;
            String acc;
            String context = u"accidental";
            if (tpcIsValid(tpc)) {
                tpc2name(tpc, noteSpelling, noteCase, c, acc);
            } else if (_function.size() > 1) {
                acc = _function.at(0);
            }
            // German spelling - use special symbol for accidental in TPC_B_B
            // to allow it to be rendered as either Bb or B
            if (tpc == Tpc::TPC_B_B && noteSpelling == NoteSpellingType::GERMAN) {
                context = u"german_B";
            }
            if (acc != "") {
                TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                String lookup = context + acc;
                ChordSymbol cs = chordList->symbol(lookup);
                if (!cs.isValid()) {
                    cs = chordList->symbol(acc);
                }
                if (cs.isValid()) {
                    ts->m_font = fontList[cs.fontIdx];
                    ts->setText(cs.value);
                } else {
                    ts->setText(acc);
                }
                textList.push_back(ts);
                x += ts->width();
            }
        } else {
            LOGD("unknown render action %d", static_cast<int>(a.type));
        }
    }
}

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::render()
{
    int capo = score()->styleI(Sid::capoPosition);

    ChordList* chordList = score()->chordList();

    fontList.clear();
    for (const ChordFont& cf : chordList->fonts) {
        mu::draw::Font ff(font());
        ff.setPointSizeF(ff.pointSizeF() * cf.mag);
        if (!(cf.family.isEmpty() || cf.family == "default")) {
            ff.setFamily(cf.family, draw::Font::Type::Harmony);
        }
        fontList.push_back(ff);
    }
    if (fontList.empty()) {
        fontList.push_back(font());
    }

    for (const TextSegment* s : textList) {
        delete s;
    }
    textList.clear();
    double x = 0.0, y = 0.0;

    determineRootBaseSpelling();

    if (_leftParen) {
        render(u"( ", x, y);
    }

    if (_rootTpc != Tpc::TPC_INVALID) {
        // render root
        render(chordList->renderListRoot, x, y, _rootTpc, _rootSpelling, _rootRenderCase);
        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }
    } else if (_harmonyType == HarmonyType::NASHVILLE) {
        // render function
        render(chordList->renderListFunction, x, y, _rootTpc, _rootSpelling, _rootRenderCase);
        double adjust = chordList->nominalAdjust();
        y += adjust * magS() * spatium() * .2;
        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }
    } else {
        render(_textName, x, y);
    }

    // render bass
    if (_baseTpc != Tpc::TPC_INVALID) {
        render(chordList->renderListBase, x, y, _baseTpc, _baseSpelling, _baseRenderCase);
    }

    if (_rootTpc != Tpc::TPC_INVALID && capo > 0 && capo < 12) {
        int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
        int capoRootTpc = _rootTpc + tpcOffset[capo];
        int capoBassTpc = _baseTpc;

        if (capoBassTpc != Tpc::TPC_INVALID) {
            capoBassTpc += tpcOffset[capo];
        }

        /*
         * For guitarists, avoid x and bb in Root or Bass,
         * and also avoid E#, B#, Cb and Fb in Root.
         */
        if (capoRootTpc < 8 || (capoBassTpc != Tpc::TPC_INVALID && capoBassTpc < 6)) {
            capoRootTpc += 12;
            if (capoBassTpc != Tpc::TPC_INVALID) {
                capoBassTpc += 12;
            }
        } else if (capoRootTpc > 24 || (capoBassTpc != Tpc::TPC_INVALID && capoBassTpc > 26)) {
            capoRootTpc -= 12;
            if (capoBassTpc != Tpc::TPC_INVALID) {
                capoBassTpc -= 12;
            }
        }

        render(u"(", x, y);
        render(chordList->renderListRoot, x, y, capoRootTpc, _rootSpelling, _rootRenderCase);

        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }

        if (capoBassTpc != Tpc::TPC_INVALID) {
            render(chordList->renderListBase, x, y, capoBassTpc, _baseSpelling, _baseRenderCase);
        }
        render(u")", x, y);
    }

    if (_rightParen) {
        render(u" )", x, y);
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Harmony::spatiumChanged(double oldValue, double newValue)
{
    TextBase::spatiumChanged(oldValue, newValue);
    render();
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Harmony::localSpatiumChanged(double oldValue, double newValue)
{
    TextBase::localSpatiumChanged(oldValue, newValue);
    render();
}

//---------------------------------------------------------
//   extensionName
//---------------------------------------------------------

const String& Harmony::extensionName() const
{
    return _textName;
}

//---------------------------------------------------------
//   xmlKind
//---------------------------------------------------------

String Harmony::xmlKind() const
{
    const ChordDescription* cd = descr();
    return cd ? cd->xmlKind : String();
}

//---------------------------------------------------------
//   musicXmlText
//---------------------------------------------------------

String Harmony::musicXmlText() const
{
    const ChordDescription* cd = descr();
    return cd ? cd->xmlText : String();
}

//---------------------------------------------------------
//   xmlSymbols
//---------------------------------------------------------

String Harmony::xmlSymbols() const
{
    const ChordDescription* cd = descr();
    return cd ? cd->xmlSymbols : String();
}

//---------------------------------------------------------
//   xmlParens
//---------------------------------------------------------

String Harmony::xmlParens() const
{
    const ChordDescription* cd = descr();
    return cd ? cd->xmlParens : String();
}

//---------------------------------------------------------
//   xmlDegrees
//---------------------------------------------------------

StringList Harmony::xmlDegrees() const
{
    const ChordDescription* cd = descr();
    return cd ? cd->xmlDegrees : StringList();
}

//---------------------------------------------------------
//   degree
//---------------------------------------------------------

HDegree Harmony::degree(int i) const
{
    return mu::value(_degreeList, i);
}

//---------------------------------------------------------
//   addDegree
//---------------------------------------------------------

void Harmony::addDegree(const HDegree& d)
{
    _degreeList.push_back(d);
}

//---------------------------------------------------------
//   numberOfDegrees
//---------------------------------------------------------

size_t Harmony::numberOfDegrees() const
{
    return _degreeList.size();
}

//---------------------------------------------------------
//   clearDegrees
//---------------------------------------------------------

void Harmony::clearDegrees()
{
    _degreeList.clear();
}

//---------------------------------------------------------
//   degreeList
//---------------------------------------------------------

const std::vector<HDegree>& Harmony::degreeList() const
{
    return _degreeList;
}

//---------------------------------------------------------
//   parsedForm
//---------------------------------------------------------

const ParsedChord* Harmony::parsedForm()
{
    if (!_parsedForm) {
        ChordList* cl = score()->chordList();
        _parsedForm = new ParsedChord();
        _parsedForm->parse(_textName, cl, false);
    }
    return _parsedForm;
}

//---------------------------------------------------------
//   setHarmonyType
//---------------------------------------------------------

void Harmony::setHarmonyType(HarmonyType val)
{
    _harmonyType = val;
    setPlacement(propertyDefault(Pid::PLACEMENT).value<PlacementV>());
    switch (_harmonyType) {
    case HarmonyType::STANDARD:
        initTextStyleType(TextStyleType::HARMONY_A);
        break;
    case HarmonyType::ROMAN:
        initTextStyleType(TextStyleType::HARMONY_ROMAN);
        break;
    case HarmonyType::NASHVILLE:
        initTextStyleType(TextStyleType::HARMONY_NASHVILLE);
        break;
    }
    // TODO: convert text
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

TranslatableString Harmony::typeUserName() const
{
    switch (_harmonyType) {
    case HarmonyType::ROMAN:
        return TranslatableString("engraving", "Roman numeral");
    case HarmonyType::NASHVILLE:
        return TranslatableString("engraving", "Nashville number");
    case HarmonyType::STANDARD:
        break;
    }
    return EngravingItem::typeUserName();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Harmony::accessibleInfo() const
{
    return String(u"%1: %2").arg(translatedTypeUserName(), harmonyName());
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Harmony::screenReaderInfo() const
{
    return String(u"%1 %2").arg(translatedTypeUserName(), generateScreenReaderInfo());
}

//---------------------------------------------------------
//   generateScreenReaderInfo
//---------------------------------------------------------

String Harmony::generateScreenReaderInfo() const
{
    String rez;
    switch (_harmonyType) {
    case HarmonyType::ROMAN: {
        String aux = _textName;
        bool hasUpper = aux.contains(u'I') || aux.contains(u'V');
        bool hasLower = aux.contains(u'i') || aux.contains(u'v');
        if (hasLower && !hasUpper) {
            rez = String(u"%1 %2").arg(rez, mtrc("engraving", "lower case"));
        }
        aux = aux.toLower();
        static std::vector<std::pair<String, String> > rnaReplacements {
            { u"vii", u"7" },
            { u"vi", u"6" },
            { u"iv", u"4" },
            { u"v", u"5" },
            { u"iii", u"3" },
            { u"ii", u"2" },
            { u"i", u"1" },
        };
        static std::vector<std::pair<String, String> > symbolReplacements {
            { u"b", u"♭" },
            { u"h", u"♮" },
            { u"#", u"♯" },
            { u"bb", u"𝄫" },
            { u"##", u"𝄪" },
            // TODO: use SMuFL glyphs and translate
            //{ "o", ""},
            //{ "0", ""},
            //{ "\+", ""},
            //{ "\^", ""},
        };
        for (auto const& r : rnaReplacements) {
            aux.replace(r.first, r.second);
        }
        for (auto const& r : symbolReplacements) {
            // only replace when not preceded by backslash
            String s = u"(?<!\\\\)" + r.first;
            std::regex re(s.toStdString());
            aux.replace(re, r.second);
        }
        // construct string one  character at a time
        for (size_t i = 0; i < aux.size(); ++i) {
            rez = String(u"%1 %2").arg(rez).arg(aux.at(i));
        }
    }
        return rez;
    case HarmonyType::NASHVILLE:
        if (!_function.isEmpty()) {
            rez = String(u"%1 %2").arg(rez, _function);
        }
        break;
    case HarmonyType::STANDARD:
    default:
        if (_rootTpc != Tpc::TPC_INVALID) {
            rez = String(u"%1 %2").arg(rez, tpc2name(_rootTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
        }
    }

    if (const_cast<Harmony*>(this)->parsedForm() && !hTextName().isEmpty()) {
        String aux = const_cast<Harmony*>(this)->parsedForm()->handle();
        aux = aux.replace(u"#", u"♯").replace(u"<", u"");
        String extension;

        for (String s : aux.split(u'>', mu::SkipEmptyParts)) {
            if (!s.contains(u"blues")) {
                s.replace(u"b", u"♭");
            }
            extension += s + u' ';
        }
        rez = String(u"%1 %2").arg(rez, extension);
    } else {
        rez = String(u"%1 %2").arg(rez, hTextName());
    }

    if (_baseTpc != Tpc::TPC_INVALID) {
        rez = String(u"%1 / %2").arg(rez, tpc2name(_baseTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
    }

    return rez;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Harmony::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    if (e->isFretDiagram()) {
        return true;
    } else if (e->isSymbol() || e->isFSymbol()) {
        // symbols can be added in edit mode
        if (data.getData(this)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Harmony::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    if (e->isFretDiagram()) {
        FretDiagram* fd = toFretDiagram(e);
        fd->setParent(explicitParent());
        fd->setTrack(track());
        score()->undoAddElement(fd);
    } else if (e->isSymbol() || e->isFSymbol()) {
        TextBase::drop(data);
        layout1();
        e = 0;          // cannot select
    } else {
        LOGW("Harmony: cannot drop <%s>\n", e->typeName());
        delete e;
        e = 0;
    }
    return e;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Harmony::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::PLAY:
        return PropertyValue(_play);
        break;
    case Pid::HARMONY_TYPE:
        return PropertyValue(int(_harmonyType));
        break;
    case Pid::HARMONY_VOICE_LITERAL:
        return _realizedHarmony.literal();
        break;
    case Pid::HARMONY_VOICING:
        return int(_realizedHarmony.voicing());
        break;
    case Pid::HARMONY_DURATION:
        return int(_realizedHarmony.duration());
        break;
    default:
        return TextBase::getProperty(pid);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Harmony::setProperty(Pid pid, const PropertyValue& v)
{
    switch (pid) {
    case Pid::PLAY:
        _play = v.toBool();
        break;
    case Pid::HARMONY_TYPE:
        setHarmonyType(HarmonyType(v.toInt()));
        break;
    case Pid::HARMONY_VOICE_LITERAL:
        _realizedHarmony.setLiteral(v.toBool());
        break;
    case Pid::HARMONY_VOICING:
        _realizedHarmony.setVoicing(Voicing(v.toInt()));
        break;
    case Pid::HARMONY_DURATION:
        _realizedHarmony.setDuration(HDuration(v.toInt()));
        break;
    default:
        if (TextBase::setProperty(pid, v)) {
            if (pid == Pid::TEXT) {
                setHarmony(v.value<String>());
            }
            render();
            break;
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Harmony::propertyDefault(Pid id) const
{
    PropertyValue v;
    switch (id) {
    case Pid::HARMONY_TYPE:
        v = int(HarmonyType::STANDARD);
        break;
    case Pid::TEXT_STYLE: {
        switch (_harmonyType) {
        case HarmonyType::STANDARD:
            v = TextStyleType::HARMONY_A;
            break;
        case HarmonyType::ROMAN:
            v = TextStyleType::HARMONY_ROMAN;
            break;
        case HarmonyType::NASHVILLE:
            v = TextStyleType::HARMONY_NASHVILLE;
            break;
        }
    }
    break;
    case Pid::PLAY:
        v = true;
        break;
    case Pid::OFFSET:
        if (explicitParent() && explicitParent()->isFretDiagram()) {
            v = PropertyValue::fromValue(PointF(0.0, 0.0));
            break;
        }
    // fall-through
    default:
        v = TextBase::propertyDefault(id);
        break;
    }
    return v;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Harmony::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        if (explicitParent() && explicitParent()->isFretDiagram()) {
            return Sid::NOSTYLE;
        } else if (textStyleType() == TextStyleType::HARMONY_A) {
            return placeAbove() ? Sid::chordSymbolAPosAbove : Sid::chordSymbolAPosBelow;
        } else if (textStyleType() == TextStyleType::HARMONY_B) {
            return placeAbove() ? Sid::chordSymbolBPosAbove : Sid::chordSymbolBPosBelow;
        } else if (textStyleType() == TextStyleType::HARMONY_ROMAN) {
            return placeAbove() ? Sid::romanNumeralPosAbove : Sid::romanNumeralPosBelow;
        } else if (textStyleType() == TextStyleType::HARMONY_NASHVILLE) {
            return placeAbove() ? Sid::nashvilleNumberPosAbove : Sid::nashvilleNumberPosBelow;
        }
    }
    if (pid == Pid::PLACEMENT) {
        switch (_harmonyType) {
        case HarmonyType::STANDARD:
            return Sid::harmonyPlacement;
        case HarmonyType::ROMAN:
            return Sid::romanNumeralPlacement;
        case HarmonyType::NASHVILLE:
            return Sid::nashvilleNumberPlacement;
        }
    }
    return TextBase::getPropertyStyle(pid);
}

KerningType Harmony::doComputeKerningType(const EngravingItem* nextItem) const
{
    if (nextItem->isHarmony()) {
        return KerningType::NON_KERNING;
    }
    return KerningType::KERNING;
}
}
