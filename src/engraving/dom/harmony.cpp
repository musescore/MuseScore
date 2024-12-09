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

#include "harmony.h"

#include "containers.h"
#include "translation.h"
#include "types/translatablestring.h"

#include "draw/fontmetrics.h"
#include "draw/types/brush.h"
#include "draw/types/pen.h"

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
using namespace muse::draw;
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

    if (m_leftParen) {
        s = u"(";
    }

    if (m_rootTpc != Tpc::TPC_INVALID) {
        r = tpc2name(m_rootTpc, m_rootSpelling, m_rootCase);
    } else if (m_harmonyType != HarmonyType::STANDARD) {
        r = m_function;
    }

    if (!m_textName.empty()) {
        e = m_textName;
        if (m_harmonyType != HarmonyType::ROMAN) {
            e.remove(u'=');
        }
    } else if (!m_degreeList.empty()) {
        hc.add(m_degreeList);
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
            r = hc.name(m_rootTpc);
            e = u"";
        }
    }

    if (m_baseTpc != Tpc::TPC_INVALID) {
        b = u"/" + tpc2name(m_baseTpc, m_baseSpelling, m_baseCase);
    }

    s += r + e + b;

    if (m_rightParen) {
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
    return tpc2name(m_rootTpc, m_rootSpelling, m_rootCase);
}

//---------------------------------------------------------
//   baseName
//---------------------------------------------------------

String Harmony::baseName()
{
    determineRootBaseSpelling();
    if (m_baseTpc == Tpc::TPC_INVALID) {
        return rootName();
    }
    return tpc2name(m_baseTpc, m_baseSpelling, m_baseCase);
}

bool Harmony::isRealizable() const
{
    return (m_rootTpc != Tpc::TPC_INVALID)
           || (m_harmonyType == HarmonyType::NASHVILLE);        // unable to fully check at for nashville at the moment
}

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
{
    if (m_degreeList.empty()) {
        return;
    }

    HChord hc = descr() ? descr()->chord : HChord();

    hc.add(m_degreeList);

// LOGD("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
// hc.print();
// _descr->chord.print();

    // try to find the chord in chordList
    const ChordList* cl = score()->chordList();
    for (const auto& p : *cl) {
        const ChordDescription& cd = p.second;
        if ((cd.chord == hc) && !cd.names.empty()) {
            LOGD("ResolveDegreeList: found in table as %s", muPrintable(cd.names.front()));
            m_id = cd.id;
            m_degreeList.clear();
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
    m_rootTpc    = Tpc::TPC_INVALID;
    m_baseTpc    = Tpc::TPC_INVALID;
    m_rootSpelling = NoteSpellingType::STANDARD;
    m_baseSpelling = NoteSpellingType::STANDARD;
    m_rootCase   = NoteCaseType::CAPITAL;
    m_baseCase   = NoteCaseType::CAPITAL;
    m_rootRenderCase = NoteCaseType::CAPITAL;
    m_baseRenderCase = NoteCaseType::CAPITAL;
    m_id         = -1;
    m_parsedForm = 0;
    m_harmonyType = HarmonyType::STANDARD;
    m_leftParen  = false;
    m_rightParen = false;
    m_play = true;
    m_realizedHarmony = RealizedHarmony(this);
    initElementStyle(&chordSymbolStyle);
}

Harmony::Harmony(const Harmony& h)
    : TextBase(h)
{
    m_rootTpc    = h.m_rootTpc;
    m_baseTpc    = h.m_baseTpc;
    m_rootSpelling = h.m_rootSpelling;
    m_baseSpelling = h.m_baseSpelling;
    m_rootCase   = h.m_rootCase;
    m_baseCase   = h.m_baseCase;
    m_rootRenderCase = h.m_rootRenderCase;
    m_baseRenderCase = h.m_baseRenderCase;
    m_id         = h.m_id;
    m_leftParen  = h.m_leftParen;
    m_rightParen = h.m_rightParen;
    m_degreeList = h.m_degreeList;
    m_parsedForm = h.m_parsedForm ? new ParsedChord(*h.m_parsedForm) : 0;
    m_harmonyType = h.m_harmonyType;
    m_textName   = h.m_textName;
    m_userName   = h.m_userName;
    m_function   = h.m_function;
    m_play       = h.m_play;
    m_realizedHarmony = h.m_realizedHarmony;
    m_realizedHarmony.setHarmony(this);
    for (const TextSegment* s : h.m_textList) {
        TextSegment* ns = new TextSegment();
        ns->set(s->text, s->m_font, s->x, s->y, s->offset);
        m_textList.push_back(ns);
    }
}

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
{
    for (const TextSegment* ts : m_textList) {
        delete ts;
    }
    if (m_parsedForm) {
        delete m_parsedForm;
    }
}

void Harmony::afterRead()
{
    // TODO: now that we can render arbitrary chords,
    // we could try to construct a full representation from a degree list.
    // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
    // or constructed in the Chord Symbol Properties dialog.

    if (m_rootTpc != Tpc::TPC_INVALID) {
        if (m_id > 0) {
            // positive id will happen only for scores that were created with explicit chord lists
            // lookup id in chord list and generate new description if necessary
            getDescription();
        } else {
            // default case: look up by name
            // description will be found for any chord already read in this score
            // and we will generate a new one if necessary
            getDescription(m_textName);
        }
    } else if (m_textName.empty()) {
        // unrecognized chords prior to 2.0 were stored as text with markup
        // we need to strip away the markup
        // this removes any user-applied formatting,
        // but we no longer support user-applied formatting for chord symbols anyhow
        // with any luck, the resulting text will be parseable now, so give it a shot
        createBlocks();
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
    if (style().styleB(Sid::useStandardNoteNames)) {
        rootSpelling = NoteSpellingType::STANDARD;
    } else if (style().styleB(Sid::useGermanNoteNames)) {
        rootSpelling = NoteSpellingType::GERMAN;
    } else if (style().styleB(Sid::useFullGermanNoteNames)) {
        rootSpelling = NoteSpellingType::GERMAN_PURE;
    } else if (style().styleB(Sid::useSolfeggioNoteNames)) {
        rootSpelling = NoteSpellingType::SOLFEGGIO;
    } else if (style().styleB(Sid::useFrenchNoteNames)) {
        rootSpelling = NoteSpellingType::FRENCH;
    }
    baseSpelling = rootSpelling;

    // case

    // always use case as typed if automatic capitalization is off
    if (!style().styleB(Sid::automaticCapitalization)) {
        rootCase = m_rootCase;
        baseCase = m_baseCase;
        return;
    }

    // set default
    if (style().styleB(Sid::allCapsNoteNames)) {
        rootCase = NoteCaseType::UPPER;
        baseCase = NoteCaseType::UPPER;
    } else {
        rootCase = NoteCaseType::CAPITAL;
        baseCase = NoteCaseType::CAPITAL;
    }

    // override for bass note
    if (style().styleB(Sid::lowerCaseBassNotes)) {
        baseCase = NoteCaseType::LOWER;
    }

    // override for minor chords
    if (style().styleB(Sid::lowerCaseMinorChords)) {
        const ChordDescription* cd = descr();
        String quality;
        if (cd) {
            // use chord description if possible
            // this is the usual case
            quality = cd->quality();
        } else if (m_parsedForm) {
            // this happens on load of new chord list
            // for chord symbols that were added/edited since the score was loaded
            // or read aloud with screenreader
            // parsed form is usable even if out of date with respect to chord list
            quality = m_parsedForm->quality();
        } else {
            // this happens on load of new chord list
            // for chord symbols that have not been edited since the score was loaded
            // we need to parse this chord for now to determine quality
            // but don't keep the parsed form around as we're not ready for it yet
            quality = parsedForm()->quality();
            delete m_parsedForm;
            m_parsedForm = 0;
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
    determineRootBaseSpelling(m_rootSpelling, m_rootRenderCase,
                              m_baseSpelling, m_baseRenderCase);
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
    if (s.empty()) {
        return Tpc::TPC_INVALID;
    }
    noteCase = s.at(0).isLower() ? NoteCaseType::LOWER : NoteCaseType::CAPITAL;
    int acci;
    switch (noteSpelling) {
    case NoteSpellingType::SOLFEGGIO:
    case NoteSpellingType::FRENCH:
        useSolfeggio = true;
        if (s.startsWith(u"sol", muse::CaseInsensitive)) {
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
    if (!acc.empty()) {
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
    m_id = -1;
    if (m_parsedForm) {
        delete m_parsedForm;
        m_parsedForm = 0;
    }
    m_textName.clear();
    bool useLiteral = false;
    if (ss.endsWith(' ')) {
        useLiteral = true;
    }

    if (m_harmonyType == HarmonyType::ROMAN) {
        m_userName = ss;
        m_textName = ss;
        *root = Tpc::TPC_INVALID;
        *base = Tpc::TPC_INVALID;
        return 0;
    }

    // pre-process for parentheses
    String s = ss.simplified();
    if ((m_leftParen = s.startsWith('('))) {
        s.remove(0, 1);
    }
    if ((m_rightParen = (s.endsWith(')') && s.count('(') < s.count(')')))) {
        s.remove(s.size() - 1, 1);
    }
    if (m_leftParen || m_rightParen) {
        s = s.simplified();         // in case of spaces inside parentheses
    }
    if (s.isEmpty()) {
        return 0;
    }

    // pre-process for lower case minor chords
    bool preferMinor;
    if (style().styleB(Sid::lowerCaseMinorChords) && s.at(0).isLower()) {
        preferMinor = true;
    } else {
        preferMinor = false;
    }

    if (m_harmonyType == HarmonyType::NASHVILLE) {
        int n = 0;
        if (s.at(0).isDigit()) {
            n = 1;
        } else if (s.at(1).isDigit()) {
            n = 2;
        }
        m_function = s.mid(0, n);
        s = s.mid(n);
        *root = Tpc::TPC_INVALID;
        *base = Tpc::TPC_INVALID;
    } else {
        determineRootBaseSpelling();
        size_t idx;
        int r = convertNote(s, m_rootSpelling, m_rootCase, idx);
        if (r == Tpc::TPC_INVALID) {
            if (s.at(0) == '/') {
                idx = 0;
            } else {
                LOGD("failed <%s>", muPrintable(ss));
                m_userName = s;
                m_textName = s;
                return 0;
            }
        }
        *root = r;
        *base = Tpc::TPC_INVALID;
        size_t slash = s.lastIndexOf(u'/');
        if (slash != muse::nidx) {
            String bs = s.mid(slash + 1).simplified();
            s = s.mid(idx, slash - idx).simplified();
            size_t idx2;
            *base = convertNote(bs, m_baseSpelling, m_baseCase, idx2);
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

    m_userName = s;
    const ChordList* cl = score()->chordList();
    const ChordDescription* cd = 0;
    if (useLiteral) {
        cd = descr(s);
    } else {
        m_parsedForm = new ParsedChord();
        m_parsedForm->parse(s, cl, syntaxOnly, preferMinor);
        // parser prepends "=" to name of implied minor chords
        // use this here as well
        if (preferMinor) {
            s = m_parsedForm->name();
        }
        // look up to see if we already have a descriptor (chord has been used before)
        cd = descr(s, m_parsedForm);
    }
    if (cd) {
        // descriptor found; use its information
        m_id = cd->id;
        if (!cd->names.empty()) {
            m_textName = cd->names.front();
        }
    } else {
        // no descriptor yet; just set textname
        // we will generate descriptor later if necessary (when we are done editing this chord)
        m_textName = s;
    }
    return cd;
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Harmony::startEditTextual(EditData& ed)
{
    if (!m_textList.empty()) {
        // convert chord symbol to plain text
        setPlainText(harmonyName());
        // clear rendering
        for (const TextSegment* t : m_textList) {
            delete t;
        }
        m_textList.clear();
    }

    // layout as text, without position reset
    renderer()->layoutText1(this, true);
    triggerLayout();

    TextBase::startEditTextual(ed);
}

bool Harmony::isTextualEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    if (ed.key == Key_Semicolon || ed.key == Key_Colon) {
        return false;
    }

    if ((ed.key == Key_Left || ed.key == Key_Right) && (ed.modifiers & ControlModifier)) {
        return false;
    }

    if (ed.key == Key_Return || ed.key == Key_Enter) {
        // This "edit" is actually handled in NotationInteraction::editElement
        return true;
    }

    return TextBase::isTextualEditAllowed(ed);
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Harmony::editTextual(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    bool rv = TextBase::editTextual(ed);

    // layout as text, without position reset
    renderer()->layoutText1(this, true);
    triggerLayout();

    // check spelling
    int root = TPC_INVALID;
    int base = TPC_INVALID;
    String str = xmlText();
    m_isMisspelled = !str.isEmpty()
                     && !parseHarmony(str, &root, &base, true)
                     && root == TPC_INVALID
                     && m_harmonyType == HarmonyType::STANDARD;
    if (m_isMisspelled) {
        LOGD("bad spell");
    }

    return rv;
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEditTextual(EditData& ed)
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
    m_realizedHarmony.setDirty(true);

    setHarmony(s);
    setPlainText(harmonyName());

    // disable spell check
    m_isMisspelled = false;

    TextBase::endEditTextual(ed);

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
            if (style().styleB(Sid::concertPitch) != h->style().styleB(Sid::concertPitch)) {
                Staff* staffDest = h->staff();
                Segment* segment = getParentSeg();
                Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
                Interval interval = staffDest->transpose(tick);
                if (!interval.isZero()) {
                    if (!h->style().styleB(Sid::concertPitch)) {
                        interval.flip();
                    }
                    int rootTpc = transposeTpc(m_rootTpc, interval, true);
                    int baseTpc = transposeTpc(m_baseTpc, interval, true);
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
}

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void Harmony::setHarmony(const String& s)
{
    int r, b;
    const ChordDescription* cd = parseHarmony(s, &r, &b);
    if (!cd && m_parsedForm && m_parsedForm->parseable()) {
        // our first time encountering this chord
        // generate a descriptor and use it
        cd = generateDescription();
        m_id = cd->id;
    }
    if (cd) {
        setRootTpc(r);
        setBaseTpc(b);
        render();
    } else {
        // unparseable chord, render as plain text
        for (const TextSegment* ts : m_textList) {
            delete ts;
        }
        m_textList.clear();
        setRootTpc(Tpc::TPC_INVALID);
        setBaseTpc(Tpc::TPC_INVALID);
        m_id = -1;
        render();
    }
}

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

double Harmony::baseLine() const
{
    return (m_textList.empty()) ? TextBase::baseLine() : 0.0;
}

//---------------------------------------------------------
//   text
//---------------------------------------------------------

String HDegree::text() const
{
    if (m_type == HDegreeType::UNDEF) {
        return String();
    }
    String degree;
    switch (m_type) {
    case HDegreeType::UNDEF: break;
    case HDegreeType::ADD:         degree = u"add";
        break;
    case HDegreeType::ALTER:       degree = u"alt";
        break;
    case HDegreeType::SUBTRACT:    degree = u"sub";
        break;
    }

    switch (m_alter) {
    case -1:          degree += u"b";
        break;
    case 1:           degree += u"#";
        break;
    default:          break;
    }
    String s = String::number(m_value);
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
            if (currentMeasure && currentMeasure->mmRest()) {
                duration += currentMeasure->mmRest()->ticks();
                currentMeasure = currentMeasure->mmRest()->nextMeasure();
            }
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
    m_textName = pc->fromXml(kind, kindText, symbols, parens, dl, score()->chordList());
    m_parsedForm = pc;
    const ChordDescription* cd = getDescription(m_textName, pc);
    return cd;
}

//---------------------------------------------------------
//   descr
//    look up id in chord list
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* Harmony::descr() const
{
    return score()->chordList()->description(m_id);
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
        m_textName = cd->names.front();
    } else if (!m_textName.empty()) {
        cd = generateDescription();
        m_id = cd->id;
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
        m_id = cd->id;
    } else {
        cd = generateDescription();
        m_id = cd->id;
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
    Fraction tick = this->tick();
    const Staff* st = staff();

    const CapoParams& capo = st->capo(tick);

    int offset = 0;
    if (capo.active) {
        offset = capo.fretPosition;
    }

    Interval interval = st->part()->instrument(tick)->transpose();
    if (!style().styleB(Sid::concertPitch)) {
        offset += interval.chromatic;
    }

    //Adjust for Nashville Notation, might be temporary
    // TODO: set dirty on add/remove of keysig
    if (m_harmonyType == HarmonyType::NASHVILLE && !m_realizedHarmony.valid()) {
        Key key = staff()->key(tick);

        //parse root
        int rootTpc = function2Tpc(m_function, key);

        //parse bass
        size_t slash = m_textName.lastIndexOf('/');
        int bassTpc;
        if (slash == muse::nidx) {
            bassTpc = Tpc::TPC_INVALID;
        } else {
            bassTpc = function2Tpc(m_textName.mid(slash + 1), key);
        }
        m_realizedHarmony.update(rootTpc, bassTpc, offset);
    } else {
        m_realizedHarmony.update(m_rootTpc, m_baseTpc, offset);
    }

    return m_realizedHarmony;
}

//---------------------------------------------------------
//   realizedHarmony
//    get realized harmony or create one for the current symbol
//    without updating the realized harmony
//---------------------------------------------------------

RealizedHarmony& Harmony::realizedHarmony()
{
    return m_realizedHarmony;
}

//---------------------------------------------------------
//   generateDescription
//    generate new chord description from _textName
//    add to chord list using private id
//---------------------------------------------------------

const ChordDescription* Harmony::generateDescription()
{
    ChordList* cl = score()->chordList();
    ChordDescription cd(m_textName);
    cd.complete(m_parsedForm, cl);
    // remove parsed chord from description
    // so we will only match it literally in the future
    cd.parsedChords.clear();
    cl->insert({ cd.id, cd });
    return &cl->at(cd.id);
}

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void Harmony::drawEditMode(Painter* p, EditData& ed, double currentViewScaling)
{
    TextBase::drawEditMode(p, ed, currentViewScaling);

    Color originalColor = color();
    if (m_isMisspelled) {
        setColor(configuration()->criticalColor());
        setSelected(false);
    }
    PointF pos(canvasPos());
    p->translate(pos);
    setIsDrawEditMode(true);
    renderer()->drawItem(this, p);
    setIsDrawEditMode(false);
    p->translate(-pos);
    if (m_isMisspelled) {
        setColor(originalColor);
        setSelected(true);
    }
}

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

TextSegment::TextSegment(const String& s, const Font& f, double x, double y)
{
    set(s, f, x, y, PointF());
    select = false;
}

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double TextSegment::width() const
{
    return FontMetrics::width(m_font, text);
}

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

RectF TextSegment::boundingRect() const
{
    return FontMetrics::boundingRect(m_font, text);
}

//---------------------------------------------------------
//   tightBoundingRect
//---------------------------------------------------------

RectF TextSegment::tightBoundingRect() const
{
    return FontMetrics::tightBoundingRect(m_font, text);
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextSegment::set(const String& s, const Font& f, double _x, double _y, PointF _offset)
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
        Font f = m_harmonyType != HarmonyType::ROMAN ? m_fontList[fontIdx] : font();
        TextSegment* ts = new TextSegment(s, f, x, y);
        m_textList.push_back(ts);
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
            TextSegment* ts = new TextSegment(m_fontList[fontIdx], x, y);
            ChordSymbol cs = chordList->symbol(a.text);
            if (cs.isValid()) {
                ts->m_font = m_fontList[cs.fontIdx];
                ts->setText(cs.value);
            } else {
                ts->setText(a.text);
            }
            if (m_harmonyType == HarmonyType::NASHVILLE) {
                double nmag = chordList->nominalMag();
                ts->m_font.setPointSizeF(ts->m_font.pointSizeF() * nmag);
            }
            m_textList.push_back(ts);
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
            } else if (m_function.size() > 0) {
                c = m_function.at(m_function.size() - 1);
            }
            TextSegment* ts = new TextSegment(m_fontList[fontIdx], x, y);
            String lookup = u"note" + c;
            ChordSymbol cs = chordList->symbol(lookup);
            if (!cs.isValid()) {
                cs = chordList->symbol(c);
            }
            if (cs.isValid()) {
                ts->m_font = m_fontList[cs.fontIdx];
                ts->setText(cs.value);
            } else {
                ts->setText(c);
            }
            m_textList.push_back(ts);
            x += ts->width();
        } else if (a.type == RenderAction::RenderActionType::ACCIDENTAL) {
            String c;
            String acc;
            String context = u"accidental";
            if (tpcIsValid(tpc)) {
                tpc2name(tpc, noteSpelling, noteCase, c, acc);
            } else if (m_function.size() > 1) {
                acc = m_function.at(0);
            }
            // German spelling - use special symbol for accidental in TPC_B_B
            // to allow it to be rendered as either Bb or B
            if (tpc == Tpc::TPC_B_B && noteSpelling == NoteSpellingType::GERMAN) {
                context = u"german_B";
            }
            if (!acc.empty()) {
                TextSegment* ts = new TextSegment(m_fontList[fontIdx], x, y);
                String lookup = context + acc;
                ChordSymbol cs = chordList->symbol(lookup);
                if (!cs.isValid()) {
                    cs = chordList->symbol(acc);
                }
                if (cs.isValid()) {
                    ts->m_font = m_fontList[cs.fontIdx];
                    ts->setText(cs.value);
                } else {
                    ts->setText(acc);
                }
                m_textList.push_back(ts);
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
    int capo = style().styleI(Sid::capoPosition);

    ChordList* chordList = score()->chordList();

    m_fontList.clear();
    for (const ChordFont& cf : chordList->fonts) {
        Font ff(font());
        ff.setPointSizeF(ff.pointSizeF() * cf.mag);
        if (!(cf.family.isEmpty() || cf.family == "default")) {
            ff.setFamily(cf.family, Font::Type::Harmony);
        }
        m_fontList.push_back(ff);
    }
    if (m_fontList.empty()) {
        m_fontList.push_back(font());
    }

    for (const TextSegment* s : m_textList) {
        delete s;
    }
    m_textList.clear();
    double x = 0.0, y = 0.0;

    determineRootBaseSpelling();

    if (m_leftParen) {
        render(u"( ", x, y);
    }

    if (m_rootTpc != Tpc::TPC_INVALID) {
        // render root
        render(chordList->renderListRoot, x, y, m_rootTpc, m_rootSpelling, m_rootRenderCase);
        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }
    } else if (m_harmonyType == HarmonyType::NASHVILLE) {
        // render function
        render(chordList->renderListFunction, x, y, m_rootTpc, m_rootSpelling, m_rootRenderCase);
        double adjust = chordList->nominalAdjust();
        y += adjust * magS() * spatium() * .2;
        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }
    } else {
        render(m_textName, x, y);
    }

    // render bass
    if (m_baseTpc != Tpc::TPC_INVALID) {
        render(chordList->renderListBase, x, y, m_baseTpc, m_baseSpelling, m_baseRenderCase);
    }

    if (m_rootTpc != Tpc::TPC_INVALID && capo > 0 && capo < 12) {
        int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
        int capoRootTpc = m_rootTpc + tpcOffset[capo];
        int capoBassTpc = m_baseTpc;

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
        render(chordList->renderListRoot, x, y, capoRootTpc, m_rootSpelling, m_rootRenderCase);

        // render extension
        const ChordDescription* cd = getDescription();
        if (cd) {
            render(cd->renderList, x, y, 0);
        }

        if (capoBassTpc != Tpc::TPC_INVALID) {
            render(chordList->renderListBase, x, y, capoBassTpc, m_baseSpelling, m_baseRenderCase);
        }
        render(u")", x, y);
    }

    if (m_rightParen) {
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
    return m_textName;
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
    return muse::value(m_degreeList, i);
}

//---------------------------------------------------------
//   addDegree
//---------------------------------------------------------

void Harmony::addDegree(const HDegree& d)
{
    m_degreeList.push_back(d);
}

//---------------------------------------------------------
//   numberOfDegrees
//---------------------------------------------------------

size_t Harmony::numberOfDegrees() const
{
    return m_degreeList.size();
}

//---------------------------------------------------------
//   clearDegrees
//---------------------------------------------------------

void Harmony::clearDegrees()
{
    m_degreeList.clear();
}

//---------------------------------------------------------
//   degreeList
//---------------------------------------------------------

const std::vector<HDegree>& Harmony::degreeList() const
{
    return m_degreeList;
}

//---------------------------------------------------------
//   parsedForm
//---------------------------------------------------------

const ParsedChord* Harmony::parsedForm() const
{
    if (!m_parsedForm) {
        ChordList* cl = score()->chordList();
        m_parsedForm = new ParsedChord();
        m_parsedForm->parse(m_textName, cl, false);
    }
    return m_parsedForm;
}

//---------------------------------------------------------
//   setHarmonyType
//---------------------------------------------------------

void Harmony::setHarmonyType(HarmonyType val)
{
    m_harmonyType = val;
    setPlacement(propertyDefault(Pid::PLACEMENT).value<PlacementV>());
    switch (m_harmonyType) {
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
    switch (m_harmonyType) {
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
    switch (m_harmonyType) {
    case HarmonyType::ROMAN: {
        String aux = m_textName;
        bool hasUpper = aux.contains(u'I') || aux.contains(u'V');
        bool hasLower = aux.contains(u'i') || aux.contains(u'v');
        if (hasLower && !hasUpper) {
            rez = String(u"%1 %2").arg(rez, muse::mtrc("engraving", "lower case"));
        }
        aux = aux.toLower();
        static const std::vector<std::pair<String, String> > rnaReplacements {
            { u"vii", u"7" },
            { u"vi", u"6" },
            { u"iv", u"4" },
            { u"v", u"5" },
            { u"iii", u"3" },
            { u"ii", u"2" },
            { u"i", u"1" },
        };
        static const std::vector<std::pair<String, String> > symbolReplacements {
            { u"bb", u"𝄫" },
            { u"##", u"𝄪" },
            { u"h", u"♮" },
            { u"\\♮", u"h" }, // \h should be h, so need to correct replacing in the previous step
            { u"#", u"♯" },
            { u"\\♯", u"#" }, // \# should be #, so need to correct replacing in the previous step
            { u"b", u"♭" },
            { u"\\♭", u"b" }, // \b should be b, so need to correct replacing in the previous step
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
            aux.replace(r.first, r.second);
        }
        // construct string one  character at a time
        for (size_t i = 0; i < aux.size(); ++i) {
            rez = String(u"%1 %2").arg(rez).arg(aux.at(i));
        }
    }
        return rez;
    case HarmonyType::NASHVILLE:
        if (!m_function.isEmpty()) {
            rez = String(u"%1 %2").arg(rez, m_function);
        }
        break;
    case HarmonyType::STANDARD:
    default:
        if (m_rootTpc != Tpc::TPC_INVALID) {
            rez = String(u"%1 %2").arg(rez, tpc2name(m_rootTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
        }
    }

    if (const_cast<Harmony*>(this)->parsedForm() && !hTextName().isEmpty()) {
        String aux = const_cast<Harmony*>(this)->parsedForm()->handle();
        aux = aux.replace(u"#", u"♯").replace(u"<", u"");
        String extension;

        for (String s : aux.split(u'>', muse::SkipEmptyParts)) {
            if (!s.contains(u"blues")) {
                s.replace(u"b", u"♭");
            }
            extension += s + u' ';
        }
        rez = String(u"%1 %2").arg(rez, extension);
    } else {
        rez = String(u"%1 %2").arg(rez, hTextName());
    }

    if (m_baseTpc != Tpc::TPC_INVALID) {
        rez = String(u"%1 / %2").arg(rez, tpc2name(m_baseTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
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
        renderer()->layoutText1(this);
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
        return PropertyValue(m_play);
        break;
    case Pid::HARMONY_TYPE:
        return PropertyValue(int(m_harmonyType));
        break;
    case Pid::HARMONY_VOICE_LITERAL:
        return m_realizedHarmony.literal();
        break;
    case Pid::HARMONY_VOICING:
        return int(m_realizedHarmony.voicing());
        break;
    case Pid::HARMONY_DURATION:
        return int(m_realizedHarmony.duration());
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
        m_play = v.toBool();
        break;
    case Pid::HARMONY_TYPE:
        setHarmonyType(HarmonyType(v.toInt()));
        break;
    case Pid::HARMONY_VOICE_LITERAL:
        m_realizedHarmony.setLiteral(v.toBool());
        break;
    case Pid::HARMONY_VOICING:
        m_realizedHarmony.setVoicing(Voicing(v.toInt()));
        break;
    case Pid::HARMONY_DURATION:
        m_realizedHarmony.setDuration(HDuration(v.toInt()));
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
        switch (m_harmonyType) {
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
        switch (m_harmonyType) {
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
}
