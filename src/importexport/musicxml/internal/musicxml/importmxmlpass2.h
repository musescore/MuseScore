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

#ifndef __IMPORTMXMLPASS2_H__
#define __IMPORTMXMLPASS2_H__

#include <array>

#include "importmxmlpass1.h"
#include "importxmlfirstpass.h"
#include "musicxml.h" // a.o. for Slur

#include "engraving/dom/types.h"

namespace mu::engraving {
class StringData;
class Tuplet;

//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

using GraceChordList = std::vector<Chord*>;
using FiguredBassList = std::vector<FiguredBass*>;
using Tuplets = std::map<String, Tuplet*>;
using Beams = std::map<String, Beam*>;

//---------------------------------------------------------
//   MxmlStartStop
//---------------------------------------------------------
/*
enum class MxmlStartStop : char {
      START, STOP, NONE
      };
 */

//---------------------------------------------------------
//   MusicXmlSlash
//---------------------------------------------------------

enum class MusicXmlSlash : char {
    NONE, RHYTHM, SLASH
};

//---------------------------------------------------------
//   MusicXmlTupletDesc
//---------------------------------------------------------

/**
 Describe the information extracted from
 a single note/notations/tuplet element.
 */

struct MusicXmlTupletDesc {
    MusicXmlTupletDesc();
    MxmlStartStop type;
    DirectionV direction;
    TupletBracketType bracket;
    TupletNumberType shownumber;
};

//---------------------------------------------------------
//   MusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlSpannerDesc {
    SLine* sp = nullptr;
    ElementType tp = ElementType::INVALID;
    int nr = 0;

    MusicXmlSpannerDesc(SLine* sp, ElementType tp, int nr)
        : sp(sp), tp(tp), nr(nr) {}
    MusicXmlSpannerDesc(ElementType tp, int nr)
        : sp(0), tp(tp), nr(nr) {}
};

//---------------------------------------------------------
//   NewMusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlExtendedSpannerDesc {
    SLine* sp = nullptr;
    Fraction tick2 { 0, 0 };
    track_idx_t track2 = 0;
    bool isStarted = false;
    bool isStopped = false;
    MusicXmlExtendedSpannerDesc() {}
    String toString() const;
};

//---------------------------------------------------------
//   MusicXmlLyricsExtend
//---------------------------------------------------------

class MusicXmlLyricsExtend
{
public:
    MusicXmlLyricsExtend() {}
    void init();
    void addLyric(Lyrics* const lyric);
    void setExtend(const int no, const track_idx_t track, const Fraction& tick);

private:
    std::set<Lyrics*> m_lyrics;
};

//---------------------------------------------------------
//   MusicXMLParserLyric
//---------------------------------------------------------

class MusicXMLParserLyric
{
public:
    MusicXMLParserLyric(const LyricNumberHandler lyricNumberHandler, XmlStreamReader& e, Score* score, MxmlLogger* logger);
    std::set<Lyrics*> extendedLyrics() const { return m_extendedLyrics; }
    std::map<int, Lyrics*> numberedLyrics() const { return m_numberedLyrics; }
    void parse();
private:
    void skipLogCurrElem();
    const LyricNumberHandler m_lyricNumberHandler;
    XmlStreamReader& m_e;
    const Score* m_score = nullptr;            // the score
    MxmlLogger* m_logger = nullptr;            // Error logger
    std::map<int, Lyrics*> m_numberedLyrics;   // lyrics with valid number
    std::set<Lyrics*> m_extendedLyrics;        // lyrics with the extend flag set
};

//---------------------------------------------------------
//   Notation
//      Most with text base attributes (font-*, placement)
//---------------------------------------------------------

class Notation
{
public:
    Notation(const String& name, const String& parent = {},
             const SymId& symId = SymId::noSym) { m_name = name; m_parent = parent; m_symId = symId; }
    void addAttribute(const String& name, const String& value);
    String attribute(const String& name) const;
    const std::map<String, String>& attributes() const { return m_attributes; }
    String name() const { return m_name; }
    String parent() const { return m_parent; }
    void setSymId(const SymId& symId) { m_symId = symId; }
    SymId symId() const { return m_symId; }
    void setSubType(const String& subType) { m_subType = subType; }
    String subType() const { return m_subType; }
    String print() const;
    void setText(const String& text) { m_text = text; }
    String text() const { return m_text; }
    static Notation notationWithAttributes(const String& name, const std::vector<XmlStreamReader::Attribute>& attributes,
                                           const String& parent = {}, const SymId& symId = SymId::noSym);
private:
    String m_name;
    String m_parent;
    SymId m_symId = SymId::noSym;
    String m_subType;
    String m_text;
    std::map<String, String> m_attributes;
};

//---------------------------------------------------------
//   forward references and defines
//---------------------------------------------------------

class FretDiagram;
class FiguredBassItem;
class Glissando;
class Pedal;
class Trill;
class MxmlLogger;
class MusicXMLDelayedDirectionElement;

using DelayedDirectionsList = std::vector<MusicXMLDelayedDirectionElement*>;
using SlurStack = std::array<SlurDesc, MAX_NUMBER_LEVEL>;
using TrillStack = std::array<Trill*, MAX_NUMBER_LEVEL>;
using BracketsStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using OttavasStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using HairpinsStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using SpannerStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using SpannerSet = std::set<Spanner*>;

//---------------------------------------------------------
//   MusicXMLParserNotations
//---------------------------------------------------------

class MusicXMLParserNotations
{
public:
    MusicXMLParserNotations(XmlStreamReader& e, Score* score, MxmlLogger* logger);
    void parse();
    void addToScore(ChordRest* const cr, Note* const note, const int tick, SlurStack& slurs, Glissando* glissandi[MAX_NUMBER_LEVEL][2],
                    MusicXmlSpannerMap& spanners, TrillStack& trills, Tie*& tie);
    String errors() const { return m_errors; }
    MusicXmlTupletDesc tupletDesc() const { return m_tupletDesc; }
    String tremoloType() const { return m_tremoloType; }
    int tremoloNr() const { return m_tremoloNr; }
    bool mustStopGraceAFter() const { return m_slurStop || m_wavyLineStop; }
private:
    void addError(const String& error);      // Add an error to be shown in the GUI
    void addNotation(const Notation& notation, ChordRest* const cr, Note* const note);
    void addTechnical(const Notation& notation, Note* note);
    void harmonic();
    void articulations();
    void dynamics();
    void fermata();
    void glissandoSlide();
    void mordentNormalOrInverted();
    void ornaments();
    void slur();
    void skipLogCurrElem();
    void technical();
    void tied();
    void tuplet();
    XmlStreamReader& m_e;
    const Score* m_score = nullptr;                         // the score
    MxmlLogger* m_logger = nullptr;                              // the error logger
    String m_errors;                    // errors to present to the user
    MusicXmlTupletDesc m_tupletDesc;
    String m_dynamicsPlacement;
    StringList m_dynamicsList;
    std::vector<Notation> m_notations;
    SymId m_breath { SymId::noSym };
    String m_tremoloType;
    int m_tremoloNr = 0;
    String m_wavyLineType;
    int m_wavyLineNo = 0;
    String m_arpeggioType;
    bool m_slurStop = false;
    bool m_slurStart = false;
    bool m_wavyLineStop = false;
};

//---------------------------------------------------------
//   MusicXMLParserPass2
//---------------------------------------------------------

class MusicXMLParserPass2
{
public:
    MusicXMLParserPass2(Score* score, MusicXMLParserPass1& pass1, MxmlLogger* logger);
    Err parse(const ByteArray& data);
    String errors() const { return m_errors; }

    // part specific data interface functions
    void addSpanner(const MusicXmlSpannerDesc& desc);
    MusicXmlExtendedSpannerDesc& getSpanner(const MusicXmlSpannerDesc& desc);
    void clearSpanner(const MusicXmlSpannerDesc& desc);
    void deleteHandledSpanner(SLine* const& spanner);
    int divs() { return m_divs; }

private:
    void addError(const String& error);      // Add an error to be shown in the GUI
    void initPartState(const String& partId);
    SpannerSet findIncompleteSpannersAtPartEnd();
    Err parse();
    void scorePartwise();
    void partList();
    void scorePart();
    void part();
    void measChordNote(/*, const MxmlPhase2Note note, ChordRest& currChord */);
    void measChordFlush(/*, ChordRest& currChord */);
    void measure(const String& partId, const Fraction time);
    void setMeasureRepeats(const staff_idx_t scoreRelStaff, Measure* measure);
    void attributes(const String& partId, Measure* measure, const Fraction& tick);
    void measureStyle(Measure* measure);
    void barline(const String& partId, Measure* measure, const Fraction& tick);
    void key(const String& partId, Measure* measure, const Fraction& tick);
    void clef(const String& partId, Measure* measure, const Fraction& tick);
    void time(const String& partId, Measure* measure, const Fraction& tick);
    void divisions();
    Note* note(const String& partId, Measure* measure, const Fraction sTime, const Fraction prevTime, Fraction& missingPrev, Fraction& dura,
               Fraction& missingCurr, String& currentVoice, GraceChordList& gcl, size_t& gac, Beams& currBeams, FiguredBassList& fbl,
               int& alt, MxmlTupletStates& tupletStates, Tuplets& tuplets);
    void notePrintSpacingNo(Fraction& dura);
    FiguredBassItem* figure(const int idx, const bool paren, FiguredBass* parent);
    FiguredBass* figuredBass();
    FretDiagram* frame();
    void harmony(const String& partId, Measure* measure, const Fraction& sTime);
    Accidental* accidental();
    void beam(std::map<int, String>& beamTypes);
    void duration(Fraction& dura);
    void forward(Fraction& dura);
    void backup(Fraction& dura);
    void timeModification(Fraction& timeMod, TDuration& normalType);
    void stem(DirectionV& sd, bool& nost);
    void doEnding(const String& partId, Measure* measure, const String& number, const String& type, const Color color, const String& text,
                  const bool print);
    void staffDetails(const String& partId, Measure* measure = nullptr);
    void staffTuning(StringData* t);
    void skipLogCurrElem();

    // multi-measure rest state handling
    void setMultiMeasureRestCount(int count);
    int getAndDecMultiMeasureRestCount();

    // generic pass 2 data

    XmlStreamReader m_e;
    int m_divs = 0;                        // the current divisions value
    Score* m_score = nullptr;              // the score
    MusicXMLParserPass1& m_pass1;          // the pass1 results
    MxmlLogger* m_logger = nullptr;        // Error logger
    String m_errors;                       // Errors to present to the user

    // part specific data (TODO: move to part-specific class)

    // Measure duration according to last timesig read
    // TODO: store timesigs read in pass 1, use those instead
    // or use score->sigmap() ?
    Fraction m_timeSigDura;

    SlurStack m_slurs { {} };
    TrillStack m_trills { {} };            // Current trills
    BracketsStack m_brackets;
    OttavasStack m_ottavas;                // Current ottavas
    HairpinsStack m_hairpins;              // Current hairpins
    MusicXmlExtendedSpannerDesc m_dummyNewMusicXmlSpannerDesc;

    Glissando* m_glissandi[MAX_NUMBER_LEVEL][2];     // Current slides ([0]) / glissandi ([1])

    Tie* m_tie = nullptr;
    Volta* m_lastVolta = nullptr;
    bool m_hasDrumset;                             // drumset defined TODO: move to pass 1

    MusicXmlSpannerMap m_spanners;

    MusicXmlExtendedSpannerDesc m_pedal;           // Current pedal
    Pedal* m_pedalContinue = nullptr;              // Current pedal type="change" requiring fixup
    Harmony* m_harmony = nullptr;                  // Current harmony
    Chord* m_tremStart = nullptr;                  // Starting chord for current tremolo
    FiguredBass* m_figBass = nullptr;              // Current figured bass element (to attach to next note)
    int m_multiMeasureRestCount = 0;
    int m_measureNumber = 0;                       // Current measure number as written in the score
    MusicXmlLyricsExtend m_extendedLyrics;         // Lyrics with "extend" requiring fixup

    MusicXmlSlash m_measureStyleSlash;             // Are we inside a measure to be displayed as slashes?

    size_t m_nstaves = 0;                          // Number of staves in current part
    std::vector<int> m_measureRepeatNumMeasures;
    std::vector<int> m_measureRepeatCount;
};

//---------------------------------------------------------
//   MusicXMLParserDirection
//---------------------------------------------------------

class MusicXMLParserDirection
{
public:
    MusicXMLParserDirection(XmlStreamReader& e, Score* score, MusicXMLParserPass1& pass1, MusicXMLParserPass2& pass2, MxmlLogger* logger);
    void direction(const String& partId, Measure* measure, const Fraction& tick, MusicXmlSpannerMap& spanners,
                   DelayedDirectionsList& delayedDirections);
    double totalY() const { return m_defaultY + m_relativeY; }

private:
    void directionType(std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    void bracket(const String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    void octaveShift(const String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
                     std::vector<MusicXmlSpannerDesc>& stops);
    void pedal(const String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    void dashes(const String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    void wedge(const String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    String metronome(double& r);
    void sound();
    void dynamics();
    void handleRepeats(Measure* measure, const track_idx_t track, const Fraction tick);
    void handleNmiCmi(Measure* measure, const track_idx_t track, const Fraction tick, DelayedDirectionsList& delayedDirections);
    String matchRepeat() const;
    void skipLogCurrElem();
    bool isLikelyCredit(const Fraction& tick) const;
    bool isLyricBracket() const;

    bool hasTotalY() { return m_hasRelativeY || m_hasDefaultY; }

    XmlStreamReader& m_e;
    Score* m_score = nullptr;                              // the score
    MusicXMLParserPass1& m_pass1;                // the pass1 results
    MusicXMLParserPass2& m_pass2;                // the pass2 results
    MxmlLogger* m_logger = nullptr;                        // Error logger

    StringList m_dynamicsList;
    String m_enclosure;
    String m_wordsText;
    String m_metroText;
    String m_rehearsalText;
    String m_dynaVelocity;
    String m_tempo;
    String m_sndCoda;
    String m_sndDacapo;
    String m_sndDalsegno;
    String m_sndFine;
    String m_sndSegno;
    String m_sndToCoda;
    bool m_hasDefaultY = false;
    double m_defaultY = 0.0;
    bool m_hasRelativeY = false;
    double m_relativeY = 0.0;
    double m_tpoMetro = 0.0;                   // tempo according to metronome
    double m_tpoSound = 0.0;                   // tempo according to sound
    std::vector<EngravingItem*> m_elems;
    Fraction m_offset;
};

//---------------------------------------------------------
//   MusicXMLDelayedDirectionElement
//---------------------------------------------------------
/**
 Helper class to allow Direction elements to be sorted by _totalY
 before being added to the score.
 */

class MusicXMLDelayedDirectionElement
{
public:
    MusicXMLDelayedDirectionElement(double totalY, EngravingItem* element, track_idx_t track,
                                    String placement, Measure* measure, Fraction tick)
        : m_totalY(totalY),  m_element(element), m_track(track), m_placement(placement),
        m_measure(measure), m_tick(tick) {}
    void addElem();
    double totalY() const { return m_totalY; }

private:
    double m_totalY = 0.0;
    EngravingItem* m_element = nullptr;
    track_idx_t m_track = 0;
    String m_placement;
    Measure* m_measure = nullptr;
    Fraction m_tick;
};
} // namespace Ms
#endif
