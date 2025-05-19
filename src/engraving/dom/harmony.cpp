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
#include "line.h"
#include "linkedobjects.h"
#include "measure.h"
#include "mscore.h"
#include "part.h"
#include "pitchspelling.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "textedit.h"
#include "utils.h"

#include "log.h"
#include "undo.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   descr
//    look up id in chord list
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* HarmonyInfo::descr() const
{
    if (!chordList()) {
        return nullptr;
    }
    return chordList()->description(id());
}

//---------------------------------------------------------
//   descr
//    look up name in chord list
//    optionally look up by parsed chord as fallback
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* HarmonyInfo::descr(const String& name, const ParsedChord* pc) const
{
    const ChordDescription* match = nullptr;
    if (!chordList()) {
        return nullptr;
    }
    for (const auto& p : *chordList()) {
        const ChordDescription& cd = p.second;
        for (const String& s : cd.names) {
            if (s == name) {
                return &cd;
            }
            if (!pc) {
                continue;
            }
            for (const ParsedChord& sParsed : cd.parsedChords) {
                if (sParsed == *pc) {
                    match = &cd;
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

const ChordDescription* HarmonyInfo::getDescription()
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

const ChordDescription* HarmonyInfo::getDescription(const String& name, const ParsedChord* pc)
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
//   generateDescription
//    generate new chord description from _textName
//    add to chord list using private id
//---------------------------------------------------------

const ChordDescription* HarmonyInfo::generateDescription()
{
    ChordDescription cd(m_textName);
    cd.complete(getParsedChord(), chordList());
    // remove parsed chord from description
    // so we will only match it literally in the future
    cd.parsedChords.clear();
    chordList()->insert({ cd.id, cd });
    return &chordList()->at(cd.id);
}

ParsedChord* HarmonyInfo::getParsedChord()
{
    if (!m_parsedChord) {
        m_parsedChord = new ParsedChord();
        m_parsedChord->parse(m_textName, chordList(), false);
    }
    return m_parsedChord;
}

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

String Harmony::harmonyName() const
{
    String name;

    for (size_t i = 0; i < m_chords.size(); i++) {
        const HarmonyInfo* info = m_chords.at(i);
        HChord hc = info->descr() ? info->descr()->chord : HChord();
        String s, r, e, b;

        if (i != 0) {
            name += u"|";
        }

        if (m_leftParen) {
            s = u"(";
        }

        if (m_harmonyType == HarmonyType::STANDARD && tpcIsValid(info->rootTpc())) {
            NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();
            r = tpc2name(info->rootTpc(), spelling, m_rootCase);
        } else if (m_harmonyType == HarmonyType::NASHVILLE && tpcIsValid(info->rootTpc())) {
            const Staff* st = staff();
            Key key = st ? st->key(tick()) : Key::INVALID;
            r = tpc2Function(info->rootTpc(), key);
        }

        if (!info->textName().empty()) {
            e = info->textName();
            if (m_harmonyType != HarmonyType::ROMAN) {
                e.remove(u'=');
            }
        } else if (!m_degreeList.empty()) {
            hc.add(m_degreeList);
            // try to find the chord in chordList
            const ChordDescription* newExtension = nullptr;
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
                r = hc.name(info->rootTpc());
                e = u"";
            }
        }

        if (tpcIsValid(info->bassTpc())) {
            NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();
            b = u"/" + tpc2name(info->bassTpc(), spelling, m_bassCase);
        }

        s += r + e + b;

        if (m_rightParen) {
            s += u")";
        }

        name += s;
    }
    return name;
}

bool Harmony::isRealizable() const
{
    for (const HarmonyInfo* info : m_chords) {
        if (!tpcIsValid(info->rootTpc())) {
            return false;
        }
    }
    return true;
}

int Harmony::bassTpc() const
{
    if (m_chords.empty()) {
        return Tpc::TPC_INVALID;
    }
    return m_chords.front()->bassTpc();
}

int Harmony::rootTpc() const
{
    if (m_chords.empty()) {
        return Tpc::TPC_INVALID;
    }
    return m_chords.front()->rootTpc();
}

bool Harmony::isInFretBox() const
{
    EngravingObject* parent = explicitParent();
    if (!parent) {
        return false;
    }

    if (parent->isFretDiagram()) {
        return toFretDiagram(parent)->isInFretBox();
    }

    return parent->isFBox();
}

//---------------------------------------------------------
//   chordSymbolStyle
//---------------------------------------------------------

const ElementStyle chordSymbolStyle {
    { Sid::harmonyPlacement, Pid::PLACEMENT },
    { Sid::minHarmonyDistance, Pid::MIN_DISTANCE },
    { Sid::harmonyVoiceLiteral, Pid::HARMONY_VOICE_LITERAL },
    { Sid::harmonyVoicing, Pid::HARMONY_VOICING },
    { Sid::harmonyDuration, Pid::HARMONY_DURATION },
    { Sid::verticallyAlignChordSymbols, Pid::VERTICAL_ALIGN }
};

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Segment* parent)
    : TextBase(ElementType::HARMONY, parent, TextStyleType::HARMONY_A, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_rootCase   = NoteCaseType::CAPITAL;
    m_bassCase   = NoteCaseType::CAPITAL;
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
    m_rootCase   = h.m_rootCase;
    m_bassCase   = h.m_bassCase;
    m_leftParen  = h.m_leftParen;
    m_rightParen = h.m_rightParen;
    m_degreeList = h.m_degreeList;
    m_harmonyType = h.m_harmonyType;
    m_play       = h.m_play;
    m_realizedHarmony = h.m_realizedHarmony;
    m_realizedHarmony.setHarmony(this);
    for (const HarmonyInfo* hi : h.m_chords) {
        HarmonyInfo* newInfo = new HarmonyInfo(*hi);
        m_chords.push_back(newInfo);
    }

    for (const TextSegment* s : h.m_textList) {
        TextSegment* ns = new TextSegment(s->text, s->m_font, s->x, s->y, s->offset, s->hAlign);
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
    for (const HarmonyInfo* info : m_chords) {
        delete info;
    }
}

int Harmony::id() const
{
    if (m_chords.empty()) {
        return -1;
    }
    return m_chords.front()->id();
}

void Harmony::afterRead()
{
    // TODO: now that we can render arbitrary chords,
    // we could try to construct a full representation from a degree list.
    // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
    // or constructed in the Chord Symbol Properties dialog.

    for (HarmonyInfo* info : m_chords) {
        if (tpcIsValid(info->rootTpc())) {
            if (info->id() > 0) {
                // positive id will happen only for scores that were created with explicit chord lists
                // lookup id in chord list and generate new description if necessary
                info->getDescription();
            } else {
                // default case: look up by name
                // description will be found for any chord already read in this score
                // and we will generate a new one if necessary
                info->getDescription(info->textName());
            }
        } else if (info->textName().empty()) {
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
    }

    // render chord from description (or _textName)
    render();
    setPlainText(harmonyName());
}

//---------------------------------------------------------
//   parseHarmony
//    determine root and bass tpc & case
//    compare body of chordname against chord list
//    return true if chord is recognized
//---------------------------------------------------------

const std::vector<const ChordDescription*> Harmony::parseHarmony(const String& ss, bool syntaxOnly)
{
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

    muse::DeleteAll(m_chords);
    m_chords.clear();

    std::vector<const ChordDescription*> descriptions;
    if (s.isEmpty()) {
        return descriptions;
    }

    StringList chords = ss.split('|');

    for (const String& subChord : chords) {
        if (subChord.empty()) {
            continue;
        }
        HarmonyInfo* info = new HarmonyInfo(score());
        const ChordDescription* cd = parseSingleHarmony(subChord, info, syntaxOnly);
        descriptions.push_back(cd);
        m_chords.push_back(info);
    }

    return descriptions;
}

const ChordDescription* Harmony::parseSingleHarmony(const String& ss, HarmonyInfo* info, bool syntaxOnly)
{
    String s = ss.simplified();

    if (m_harmonyType == HarmonyType::ROMAN) {
        info->setTextName(ss);
        info->setRootTpc(Tpc::TPC_INVALID);
        info->setBassTpc(Tpc::TPC_INVALID);
        return nullptr;
    }

    bool useLiteral = false;
    if (ss.endsWith(' ')) {
        useLiteral = true;
    }

    // pre-process for lower case minor chords
    bool preferMinor;
    if (style().styleB(Sid::lowerCaseMinorChords) && s.at(0).isLower()) {
        preferMinor = true;
    } else {
        preferMinor = false;
    }

    size_t idx = 0;
    const Staff* st = staff();
    Key key = st ? st->key(tick()) : Key::INVALID;
    NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    int r = Tpc::TPC_INVALID;
    if (m_harmonyType == HarmonyType::STANDARD) {
        r = convertNote(s, spelling, m_rootCase, idx);
    } else if (m_harmonyType == HarmonyType::NASHVILLE) {
        r = function2Tpc(s, key, idx);
    }
    if (!tpcIsValid(r)) {
        if (s.at(0) == '/') {
            idx = 0;
        } else {
            LOGD("failed <%s>", muPrintable(ss));
            info->setTextName(s);
            return 0;
        }
    }
    info->setRootTpc(r);
    info->setBassTpc(Tpc::TPC_INVALID);
    size_t slash = s.lastIndexOf(u'/');
    if (slash != muse::nidx) {
        String bs = s.mid(slash + 1).simplified();
        s = s.mid(idx, slash - idx).simplified();
        size_t idx2 = 0;
        if (m_harmonyType == HarmonyType::STANDARD) {
            info->setBassTpc(convertNote(bs, spelling, m_bassCase, idx2));
        } else if (m_harmonyType == HarmonyType::NASHVILLE) {
            info->setBassTpc(function2Tpc(bs, key, idx2));
        }

        if (idx2 != bs.size()) {
            info->setBassTpc(Tpc::TPC_INVALID);
        }
        if (!tpcIsValid(info->bassTpc())) {
            // if what follows after slash is not (just) a TPC
            // then reassemble chord and try to parse with the slash
            s = s + u"/" + bs;
        }
    } else {
        s = s.mid(idx);             // don't simplify; keep leading space before extension if present
    }

    const ChordList* cl = score()->chordList();
    const ChordDescription* cd = 0;
    if (useLiteral) {
        cd = info->descr(s);
    } else {
        ParsedChord* pc = new ParsedChord();
        pc->parse(s, cl, syntaxOnly, preferMinor);
        // parser prepends "=" to name of implied minor chords
        // use this here as well
        if (preferMinor) {
            s = pc->name();
        }
        // look up to see if we already have a descriptor (chord has been used before)
        cd = info->descr(s, pc);
        info->setParsedChord(pc);
    }
    if (cd) {
        // descriptor found; use its information
        info->setId(cd->id);
        if (!cd->names.empty()) {
            info->setTextName(cd->names.front());
        }
    } else {
        // no descriptor yet; just set textname
        // we will generate descriptor later if necessary (when we are done editing this chord)
        info->setTextName(s);
    }
    return cd;
}

NoteCaseType Harmony::rootRenderCase(HarmonyInfo* info) const
{
    // case
    // always use case as typed if automatic capitalization is off
    NoteCaseType noteCase = m_rootCase;
    if (!style().styleB(Sid::automaticCapitalization)) {
        return noteCase;
    }

    // set default
    if (style().styleB(Sid::allCapsNoteNames)) {
        noteCase = NoteCaseType::UPPER;
    } else {
        noteCase = NoteCaseType::CAPITAL;
    }

    // override for minor chords
    if (style().styleB(Sid::lowerCaseMinorChords)) {
        const ChordDescription* cd = info->descr();
        String quality;
        if (cd) {
            // use chord description if possible
            // this is the usual case
            quality = cd->quality();
        } else if (info->getParsedChord()) {
            // this happens on load of new chord list
            // for chord symbols that were added/edited since the score was loaded
            // or read aloud with screenreader
            // parsed form is usable even if out of date with respect to chord list
            quality = info->getParsedChord()->quality();
        } else {
            // this happens on load of new chord list
            // for chord symbols that have not been edited since the score was loaded
            // we need to parse this chord for now to determine quality
            // but don't keep the parsed form around as we're not ready for it yet
            quality = info->getParsedChord()->quality();
            delete info->parsedChord();
            info->setParsedChord(nullptr);
        }
        if (quality == "minor" || quality == "diminished" || quality == "half-diminished") {
            noteCase = NoteCaseType::LOWER;
        }
    }

    return noteCase;
}

NoteCaseType Harmony::bassRenderCase() const
{
    // case
    // always use case as typed if automatic capitalization is off
    NoteCaseType noteCase = m_bassCase;
    if (!style().styleB(Sid::automaticCapitalization)) {
        return noteCase;
    }

    // set default
    if (style().styleB(Sid::allCapsNoteNames)) {
        noteCase = NoteCaseType::UPPER;
    } else {
        noteCase = NoteCaseType::CAPITAL;
    }

    // override for bass note
    if (style().styleB(Sid::lowerCaseBassNotes)) {
        noteCase = NoteCaseType::LOWER;
    }

    return noteCase;
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
    String str = xmlText();

    std::vector<const ChordDescription*> descriptions = parseHarmony(str, true);
    bool descriptionsValid = true;
    for (const ChordDescription* cd : descriptions) {
        if (!cd) {
            descriptionsValid = false;
        }
    }

    bool tpcsValid = true;
    for (const HarmonyInfo* info : m_chords) {
        tpcsValid &= tpcIsValid(info->rootTpc());
    }

    m_isMisspelled = !str.isEmpty()
                     && !descriptionsValid
                     && !tpcsValid
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

    setHarmony(s);
    setPlainText(harmonyName());

    // disable spell check
    m_isMisspelled = false;

    TextBase::endEditTextual(ed);

    TextEditData* ted = dynamic_cast<TextEditData*>(ed.getData(this).get());
    bool textChanged = ted != nullptr && ted->oldXmlText != harmonyName();

    if (textChanged) {
        Segment* parentSegment = toSegment(findAncestor(ElementType::SEGMENT));
        if (parentSegment) {
            EngravingItem* fretDiagramItem = parentSegment->findAnnotation(ElementType::FRET_DIAGRAM, track(), track());
            if (fretDiagramItem) {
                FretDiagram* fretDiagram = toFretDiagram(fretDiagramItem);

                UndoStack* undo = score()->undoStack();
                undo->reopen();
                score()->undo(new FretDataChange(fretDiagram, s));
                score()->endCmd();
            }
        }

        UndoStack* undo = score()->undoStack();
        undo->reopen();
        if (ted->oldXmlText.empty()) {
            score()->undoAddChordToFretBox(this);
        } else {
            score()->undoRenameChordInFretBox(this, ted->oldXmlText);
        }
        score()->endCmd();
    }

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
                Segment* segment = toSegment(findAncestor(ElementType::SEGMENT));
                Fraction tick = segment ? segment->tick() : Fraction(-1, 1);
                Interval interval = staffDest->transpose(tick);
                if (!interval.isZero()) {
                    if (!h->style().styleB(Sid::concertPitch)) {
                        interval.flip();
                    }
                    for (HarmonyInfo* info : h->m_chords) {
                        int rootTpc = transposeTpc(info->rootTpc(), interval, true);
                        int bassTpc = transposeTpc(info->bassTpc(), interval, true);
                        info->setRootTpc(rootTpc);
                        info->setBassTpc(bassTpc);
                        // score()->undoTransposeHarmony(h, rootTpc, bassTpc);
                        h->setPlainText(h->harmonyName());
                        h->setHarmony(h->plainText());
                        h->triggerLayout();
                    }
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
    m_realizedHarmony.setDirty(true);

    std::vector<const ChordDescription*> descriptions = parseHarmony(s);
    for (size_t i = 0; i < m_chords.size(); i++) {
        HarmonyInfo* info = m_chords.at(i);
        const ChordDescription* cd = i < descriptions.size() ? descriptions.at(i) : nullptr;

        if (!cd && info->getParsedChord()->parseable()) {
            // our first time encountering this chord
            // generate a descriptor and use it
            cd = info->generateDescription();
            info->setId(cd->id);
        }
        if (!cd) {
            // unparseable chord or roman numeral, render as plain text
            info->setRootTpc(Tpc::TPC_INVALID);
            info->setBassTpc(Tpc::TPC_INVALID);
            info->setId(-1);
        }
    }
    render();
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
//   findNext
///   find the next Harmony in the score
///
///   returns 0 if there is none
//---------------------------------------------------------

Harmony* Harmony::findNext() const
{
    const Segment* segment = toSegment(findAncestor(ElementType::SEGMENT));
    Segment* cur = segment ? segment->next1() : nullptr;
    while (cur) {
        Harmony* h = findInSeg(cur);
        if (h) {
            return h;
        }
        cur = cur->next1();
    }
    return nullptr;
}

//---------------------------------------------------------
//   findPrev
///   find the previous Harmony in the score
///
///   returns 0 if there is none
//---------------------------------------------------------

Harmony* Harmony::findPrev() const
{
    const Segment* segment = toSegment(findAncestor(ElementType::SEGMENT));
    Segment* cur = segment ? segment->next1() : nullptr;
    while (cur) {
        Harmony* h = findInSeg(cur);
        if (h) {
            return h;
        }
        cur = cur->prev1();
    }
    return nullptr;
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
    const Segment* seg = toSegment(findAncestor(ElementType::SEGMENT));
    if (!seg) {
        return Fraction(-1, 1);
    }

    Fraction duration = seg->ticks();

    const RepeatList& repeats = score()->repeatList();
    auto rsIt = repeats.findRepeatSegmentFromUTick(utick);
    if (rsIt == repeats.cend()) {
        return duration;
    }

    Segment* cur = seg->next();
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
            } else if (!nextHarmony) {
                // move to next RepeatSegment
                if (++rsIt != repeats.end()) {
                    currentMeasure = (*rsIt)->firstMeasure();
                    endMeasure     = (*rsIt)->lastMeasure();
                    cur = currentMeasure->first();
                }
            }
        }
    } while ((nextHarmony == nullptr) && (cur != nullptr));

    if (nextHarmony && rsIt != repeats.end()) {
        int tickOffset = (*rsIt)->utick - (*rsIt)->tick;
        int nextHarmonyUtick = nextHarmony->tick().ticks() + tickOffset;
        duration = Fraction::fromTicks(nextHarmonyUtick - utick);
    }

    return duration;
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
    const Staff* st = staff();
    IF_ASSERT_FAILED(st) {
        return m_realizedHarmony;
    }

    const Fraction tick = this->tick();
    const CapoParams& capo = st->capo(tick);

    int offset = 0;
    if (capo.active) {
        offset = capo.fretPosition;
    }

    Interval interval = st->part()->instrument(tick)->transpose();
    if (!style().styleB(Sid::concertPitch)) {
        offset += interval.chromatic;
    }

    HarmonyInfo* info = m_chords.empty() ? nullptr : m_chords.front();
    int root = info ? info->rootTpc() : Tpc::TPC_INVALID;
    int bass = info ? info->bassTpc() : Tpc::TPC_INVALID;

    m_realizedHarmony.update(root, bass, offset);

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

const ParsedChord* Harmony::parsedForm()const
{
    if (m_chords.empty()) {
        return nullptr;
    }
    return m_chords.front()->getParsedChord();
}

Color Harmony::curColor() const
{
    if (m_isMisspelled) {
        return configuration()->criticalColor();
    }

    return EngravingItem::curColor();
}

void Harmony::renderRomanNumeral()
{
    HarmonyRenderCtx ctx;
    if (m_chords.empty()) {
        return;
    }
    HarmonyInfo* info = m_chords.front();

    if (m_leftParen) {
        render(u"( ", ctx);
    }

    render(info->textName(), ctx);

    if (m_rightParen) {
        render(u" )", ctx);
    }
}

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

TextSegment::TextSegment(const String& s, const Font& f, double _x, double _y, bool align)
{
    m_font = f;
    x      = _x;
    y      = _y;
    hAlign = align;
    setText(s);
}

TextSegment::TextSegment(const String& s, const muse::draw::Font& f, double _x, double _y, PointF _offset, bool align)
{
    m_font = f;
    x      = _x;
    y      = _y;
    hAlign = align;
    offset = _offset;
    setText(s);
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
//   render
//---------------------------------------------------------

void Harmony::render(const String& s, HarmonyRenderCtx& ctx)
{
    if (s.isEmpty()) {
        return;
    }

    Font f = m_harmonyType != HarmonyType::ROMAN ? m_fontList.front() : font();
    TextSegment* ts = new TextSegment(s, f, ctx.x(), ctx.y(), ctx.hAlign);
    ctx.textList.push_back(ts);
    ctx.setX(ctx.x() + ts->width());
}

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void Harmony::render(const std::list<RenderAction>& renderList, HarmonyRenderCtx& ctx, int tpc, NoteSpellingType noteSpelling,
                     NoteCaseType noteCase, double noteMag)
{
    ChordList* chordList = score()->chordList();
    std::stack<PointF> stack;

    const Staff* st = staff();
    Key key = st ? st->key(tick()) : Key::INVALID;

// LOGD("===");
    for (const RenderAction& a : renderList) {
// a.print();
        if (a.type == RenderAction::RenderActionType::SET) {
            ChordSymbol cs = chordList->symbol(a.text);
            String text = cs.isValid() ? cs.value : a.text;
            muse::draw::Font font = cs.isValid() ? m_fontList[cs.fontIdx] : m_fontList.front();

            TextSegment* ts = new TextSegment(text, font, ctx.x(), ctx.y(), ctx.hAlign);
            if (m_harmonyType == HarmonyType::NASHVILLE) {
                double nmag = chordList->nominalMag();
                ts->m_font.setPointSizeF(ts->m_font.pointSizeF() * nmag);
            }
            ctx.textList.push_back(ts);
            ctx.setX(ctx.x() + ts->width());
        } else if (a.type == RenderAction::RenderActionType::MOVE) {
            FontMetrics fm = FontMetrics(font());
            ctx.setX(ctx.x() + a.movex * fm.height());
            ctx.setY(ctx.y() + a.movey * fm.height());
        } else if (a.type == RenderAction::RenderActionType::PUSH) {
            stack.push(ctx.pos);
        } else if (a.type == RenderAction::RenderActionType::POP) {
            if (!stack.empty()) {
                PointF pt = stack.top();
                stack.pop();
                ctx.pos = PointF(a.popx ? pt.x() : ctx.x(), a.popy ? pt.y() : ctx.y());
            } else {
                LOGD("RenderAction::RenderActionType::POP: stack empty");
            }
        } else if (a.type == RenderAction::RenderActionType::NOTE) {
            String c;
            AccidentalVal acc;
            if (m_harmonyType == HarmonyType::STANDARD && tpcIsValid(tpc)) {
                tpc2name(tpc, noteSpelling, noteCase, c, acc);
            } else if (m_harmonyType == HarmonyType::NASHVILLE && tpcIsValid(tpc)) {
                String accStr;
                tpc2Function(tpc, key, accStr, c);
            }
            String lookup = u"note" + c;
            ChordSymbol cs = chordList->symbol(lookup);
            if (!cs.isValid()) {
                cs = chordList->symbol(c);
            }
            String text = cs.isValid() ? cs.value : c;
            muse::draw::Font font = cs.isValid() ? m_fontList[cs.fontIdx] : m_fontList.front();
            font.setPointSizeF(font.pointSizeF() * noteMag);

            TextSegment* ts = new TextSegment(text, font, ctx.x(), ctx.y(), ctx.hAlign);
            ctx.textList.push_back(ts);
            ctx.setX(ctx.x() + ts->width());
        } else if (a.type == RenderAction::RenderActionType::ACCIDENTAL) {
            String c;
            String acc;
            String context = u"accidental";
            if (m_harmonyType == HarmonyType::STANDARD && tpcIsValid(tpc)) {
                tpc2name(tpc, noteSpelling, noteCase, c, acc);
            } else if (m_harmonyType == HarmonyType::NASHVILLE && tpcIsValid(tpc)) {
                tpc2Function(tpc, key, acc, c);
            }
            // German spelling - use special symbol for accidental in TPC_B_B
            // to allow it to be rendered as either Bb or B
            if (tpc == Tpc::TPC_B_B && noteSpelling == NoteSpellingType::GERMAN) {
                context = u"german_B";
            }
            if (!acc.empty()) {
                String lookup = context + acc;
                ChordSymbol cs = chordList->symbol(lookup);
                if (!cs.isValid()) {
                    cs = chordList->symbol(acc);
                }
                String text = cs.isValid() ? cs.value : c;
                muse::draw::Font font = cs.isValid() ? m_fontList[cs.fontIdx] : m_fontList.front();
                font.setPointSizeF(font.pointSizeF() * noteMag);

                TextSegment* ts = new TextSegment(text, font, ctx.x(), ctx.y(), ctx.hAlign);
                ctx.textList.push_back(ts);
                ctx.setX(ctx.x() + ts->width());
            }
        } else if (a.type == RenderAction::RenderActionType::STOPHALIGN) {
            ctx.hAlign = false;
        } else {
            LOGD("unknown render action %d", static_cast<int>(a.type));
        }
    }
}

void Harmony::renderSingleHarmony(HarmonyInfo* info, HarmonyRenderCtx& ctx)
{
    int capo = style().styleI(Sid::capoPosition);

    ChordList* chordList = info->chordList();

    NoteCaseType rootCase = rootRenderCase(info);
    NoteCaseType bassCase = bassRenderCase();

    if (m_leftParen) {
        render(u"( ", ctx);
    }

    NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    if (m_harmonyType == HarmonyType::STANDARD && tpcIsValid(info->rootTpc())) {
        // render root
        render(chordList->renderListRoot, ctx, info->rootTpc(), spelling, rootCase);
        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            render(cd->renderList, ctx, 0);
        }
    } else if (m_harmonyType == HarmonyType::NASHVILLE && tpcIsValid(info->rootTpc())) {
        // render function
        render(chordList->renderListFunction, ctx, info->rootTpc(), spelling, bassCase);
        double adjust = chordList->nominalAdjust();
        ctx.setY(ctx.y() + adjust * magS() * spatium() * .2);
        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            render(cd->renderList, ctx, 0);
        }
    } else {
        render(info->textName(), ctx);
    }

    // render bass
    if (tpcIsValid(info->bassTpc())) {
        std::list<RenderAction>& bassNoteChordList
            = style().styleB(Sid::chordBassNoteStagger) ? chordList->renderListBassOffset : chordList->renderListBass;
        render(bassNoteChordList, ctx, info->bassTpc(), spelling, bassCase, style().styleD(Sid::chordBassNoteScale));
    }

    if (tpcIsValid(info->rootTpc()) && capo > 0 && capo < 12) {
        int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
        int capoRootTpc = info->rootTpc() + tpcOffset[capo];
        int capoBassTpc = info->bassTpc();

        if (tpcIsValid(capoBassTpc)) {
            capoBassTpc += tpcOffset[capo];
        }

        /*
         * For guitarists, avoid x and bb in Root or Bass,
         * and also avoid E#, B#, Cb and Fb in Root.
         */
        if (capoRootTpc < 8 || (tpcIsValid(capoBassTpc) && capoBassTpc < 6)) {
            capoRootTpc += 12;
            if (tpcIsValid(capoBassTpc)) {
                capoBassTpc += 12;
            }
        } else if (capoRootTpc > 24 || (tpcIsValid(capoBassTpc) && capoBassTpc > 26)) {
            capoRootTpc -= 12;
            if (tpcIsValid(capoBassTpc)) {
                capoBassTpc -= 12;
            }
        }

        render(u"(", ctx);
        render(chordList->renderListRoot, ctx, capoRootTpc, spelling, rootCase);

        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            render(cd->renderList, ctx, 0);
        }

        if (tpcIsValid(capoBassTpc)) {
            std::list<RenderAction>& bassNoteChordList
                = style().styleB(Sid::chordBassNoteStagger) ? chordList->renderListBassOffset : chordList->renderListBass;
            render(bassNoteChordList, ctx, capoBassTpc, spelling, bassCase, style().styleD(Sid::chordBassNoteScale));
        }
        render(u")", ctx);
    }

    if (m_rightParen) {
        render(u" )", ctx);
    }
}

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::render()
{
    for (const TextSegment* s : m_textList) {
        delete s;
    }
    m_textList.clear();
    if (m_harmonyType == HarmonyType::ROMAN) {
        renderRomanNumeral();
        return;
    }

    // Render standard or Nashville chords

    ChordList* chordList = score()->chordList();

    m_fontList.clear();
    for (const ChordFont& cf : chordList->fonts) {
        Font ff(font());
        double mag = m_userMag.value_or(cf.mag);
        ff.setPointSizeF(ff.pointSizeF() * mag);
        if (!(cf.family.isEmpty() || cf.family == "default")) {
            ff.setFamily(cf.family, Font::Type::Harmony);
        }
        m_fontList.push_back(ff);
    }
    if (m_fontList.empty()) {
        m_fontList.push_back(font());
    }

    mutldata()->polychordDividerLines.reset();
    HarmonyRenderCtx ctx;

    // Map of text segments and their final width
    std::map<double, std::vector<TextSegment*> > chordTextSegments;

    for (size_t i = m_chords.size(); i > 0; i--) {
        HarmonyInfo* harmony = m_chords.at(i - 1);
        renderSingleHarmony(harmony, ctx);

        chordTextSegments.emplace(std::pair<double, std::vector<TextSegment*> > { ctx.x(), ctx.textList });
        m_textList.insert(m_textList.end(), ctx.textList.begin(), ctx.textList.end());
        ctx.textList.clear();

        if (m_chords.size() == 1 || i == 1) {
            break;
        }

        ctx.setX(0);
        for (const TextSegment* ts : m_textList) {
            double top = ts->pos().y() + ts->boundingRect().top();
            if (top < ctx.y()) {
                ctx.setY(top);
            }
        }

        double lineY = ctx.y() - style().styleS(Sid::polychordDividerSpacing).toMM(spatium());
        LineF line = LineF(PointF(0.0, lineY), PointF(0.0, lineY));
        mutldata()->polychordDividerLines.mut_value().push_back(line);

        ctx.setY(ctx.y() - style().styleS(Sid::polychordDividerSpacing).toMM(spatium()) * 2.0);
        ctx.setY(ctx.y() - style().styleS(Sid::polychordDividerThickness).toMM(spatium()));
    }

    // Align polychords

    AlignH align = AlignH(style().styleI(Sid::chordAlignmentToNotehead));

    if (align == AlignH::LEFT) {
        return;
    }

    double longestLine = 0.0;
    for (double width : muse::keys(chordTextSegments)) {
        if (width > longestLine) {
            longestLine = width;
        }
    }

    for (auto& textSegs : chordTextSegments) {
        double width = textSegs.first;
        std::vector<TextSegment*>& segs = textSegs.second;

        double diff = longestLine - width;

        if (muse::RealIsNull(diff)) {
            continue;
        }

        // For centre align adjust by .5* difference, for right align adjust by full difference
        if (align == AlignH::HCENTER) {
            diff *= 0.5;
        }

        for (TextSegment* seg : segs) {
            seg->x += diff;
        }
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
//   addDegree
//---------------------------------------------------------

void Harmony::addDegree(const HDegree& d)
{
    m_degreeList.push_back(d);
}

//---------------------------------------------------------
//   degreeList
//---------------------------------------------------------

const std::vector<HDegree>& Harmony::degreeList() const
{
    return m_degreeList;
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
    for (size_t i = 0; i < m_chords.size(); i++) {
        HarmonyInfo* info = m_chords.at(i);
        if (!tpcIsValid(info->rootTpc())) {
            continue;
        }
        if (i != 0) {
            rez += u" | ";
        }

        switch (m_harmonyType) {
        case HarmonyType::ROMAN: {
            if (m_chords.empty()) {
                return u"";
            }
            String aux = info->textName();
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
        case HarmonyType::NASHVILLE: {
            const Staff* st = staff();
            Key key = st ? st->key(tick()) : Key::INVALID;
            rez = String(u"%1 %2").arg(rez, tpc2Function(info->rootTpc(), key));
            break;
        }
        case HarmonyType::STANDARD:
        default:
            rez = String(u"%1 %2").arg(rez, tpc2name(info->rootTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
        }

        if (!info->textName().isEmpty()) {
            String aux = info->getParsedChord()->handle();
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
            rez = String(u"%1 %2").arg(rez, info->textName());
        }

        if (tpcIsValid(info->bassTpc())) {
            rez = String(u"%1 / %2").arg(rez, tpc2name(info->bassTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));
        }
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
    case Pid::VERTICAL_ALIGN:
        return true;
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

double Harmony::mag() const
{
    if (m_userMag.has_value()) {
        return m_userMag.value();
    }

    return EngravingItem::mag();
}

void Harmony::undoMoveSegment(Segment* newSeg, Fraction tickDiff)
{
    if (newSeg->isTimeTickType()) {
        Measure* measure = newSeg->measure();
        Segment* chordRestSegAtSameTick = measure->undoGetSegment(SegmentType::ChordRest, newSeg->tick());
        newSeg = chordRestSegAtSameTick;
    }

    TextBase::undoMoveSegment(newSeg, tickDiff);
}

HarmonyInfo::HarmonyInfo(const HarmonyInfo& h)
{
    m_id = h.m_id;
    m_bassTpc = h.m_bassTpc;
    m_rootTpc = h.m_rootTpc;
    m_textName = h.m_textName;
    m_score = h.m_score;
    m_parsedChord = h.m_parsedChord ? new ParsedChord(*h.m_parsedChord) : 0;
}

HarmonyInfo::~HarmonyInfo()
{
    if (m_parsedChord) {
        delete m_parsedChord;
    }
}
}
