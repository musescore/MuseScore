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

#include <array>

#include "importmusicxmlpass1.h"
#include "../shared/musicxmlsupport.h"
#include "../shared/musicxmltypes.h"
#include "musicxmltupletstate.h"

namespace mu::engraving {
class Accidental;
class Beam;
class ChordRest;
class EngravingItem;
class FiguredBass;
class FiguredBassItem;
class Fraction;
class FretDiagram;
class Glissando;
class GradualTempoChange;
class Hairpin;
class Harmony;
class Instrument;
class Jump;
class Lyrics;
class Marker;
class Measure;
class Note;
class Pedal;
class Score;
class SLine;
class Slur;
class Spanner;
class Staff;
class Sticking;
class StringData;
class Text;
class Tie;
class Trill;
class Tuplet;
class Volta;
enum class TupletBracketType : unsigned char;
enum class TupletNumberType : unsigned char;
}

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

using GraceChordList = std::vector<engraving::Chord*>;
using FiguredBassList = std::vector<engraving::FiguredBass*>;
using Tuplets = std::map<muse::String, engraving::Tuplet*>;
using Beams = std::map<muse::String, engraving::Beam*>;

//---------------------------------------------------------
//   MusicXmlSlash
//---------------------------------------------------------

enum class MusicXmlSlash : unsigned char {
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
    MusicXmlStartStop type;
    engraving::DirectionV direction;
    engraving::TupletBracketType bracket;
    engraving::TupletNumberType shownumber;
};

//---------------------------------------------------------
//   SlurDesc
//---------------------------------------------------------

/**
 The description of Slurs being handled
 */

class SlurDesc
{
public:
    enum class State : unsigned char {
        NONE, START, STOP
    };
    SlurDesc()
        : m_slur(0), m_state(State::NONE) {}
    engraving::Slur* slur() const { return m_slur; }
    void start(engraving::Slur* slur) { m_slur = slur; m_state = State::START; }
    void stop(engraving::Slur* slur) { m_slur = slur; m_state = State::STOP; }
    bool isStart() const { return m_state == State::START; }
    bool isStop() const { return m_state == State::STOP; }
private:
    engraving::Slur* m_slur = nullptr;
    State m_state;
};
typedef std::map<engraving::SLine*, std::pair<int, int> > MusicXmlSpannerMap;

//---------------------------------------------------------
//   MusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlSpannerDesc {
    engraving::SLine* sp = nullptr;
    engraving::ElementType tp = engraving::ElementType::INVALID;
    int nr = 0;

    MusicXmlSpannerDesc(engraving::SLine* sp, engraving::ElementType tp, int nr)
        : sp(sp), tp(tp), nr(nr) {}
    MusicXmlSpannerDesc(engraving::ElementType tp, int nr)
        : sp(0), tp(tp), nr(nr) {}
};

//---------------------------------------------------------
//   MusicXmlExtendedSpannerDesc
//---------------------------------------------------------

struct MusicXmlExtendedSpannerDesc {
    engraving::SLine* sp = nullptr;
    engraving::Fraction tick2 { 0, 0 };
    engraving::track_idx_t track2 = 0;
    bool isStarted = false;
    bool isStopped = false;
    MusicXmlExtendedSpannerDesc() {}
    muse::String toString() const;
};

//---------------------------------------------------------
//   HarmonyDesc
//---------------------------------------------------------

/**
 The description of a chord symbol with or without a fret diagram
 */

struct HarmonyDesc
{
    engraving::track_idx_t m_track;
    bool fretDiagramVisible() const;
    engraving::Harmony* m_harmony;
    engraving::FretDiagram* m_fretDiagram;

    HarmonyDesc(engraving::track_idx_t m_track, engraving::Harmony* m_harmony, engraving::FretDiagram* m_fretDiagram)
        : m_track(m_track), m_harmony(m_harmony),
        m_fretDiagram(m_fretDiagram) {}

    HarmonyDesc()
        : m_track(muse::nidx), m_harmony(nullptr), m_fretDiagram(nullptr) {}
};
using HarmonyMap = std::multimap<int, HarmonyDesc>;

//---------------------------------------------------------
//   MusicXmlLyricsExtend
//---------------------------------------------------------

class MusicXmlLyricsExtend
{
public:
    MusicXmlLyricsExtend() {}
    void init();
    void addLyric(engraving::Lyrics* const lyric);
    void setExtend(const int no, const engraving::track_idx_t track, const engraving::Fraction& tick,
                   const engraving::Lyrics* prevAddedLyrics);

private:
    std::set<engraving::Lyrics*> m_lyrics;
};

struct GraceNoteLyrics {
    engraving::Lyrics* lyric = nullptr;
    bool extend = false;
    int no = 0;

    GraceNoteLyrics(engraving::Lyrics* lyric, bool extend, int no)
        : lyric(lyric), extend(extend), no(no) {}
};

struct InferredPercInstr {
    int pitch;
    engraving::track_idx_t track;
    engraving::String name;
    engraving::Fraction tick;

    InferredPercInstr(int pitch, engraving::track_idx_t track, engraving::String name, engraving::Fraction tick)
        : pitch(pitch), track(track), name(name), tick(tick) {}

    InferredPercInstr()
        : pitch(-1), track(muse::nidx), name(u""), tick(engraving::Fraction(0, -1)) {}
};
typedef std::vector<InferredPercInstr> InferredPercList;

typedef std::map<engraving::String, std::pair<engraving::String, engraving::DurationType> > MetronomeTextMap;

//---------------------------------------------------------
//   MusicXmlParserLyric
//---------------------------------------------------------

class MusicXmlParserLyric
{
public:
    MusicXmlParserLyric(const LyricNumberHandler lyricNumberHandler, muse::XmlStreamReader& e, engraving::Score* score,
                        MusicXmlLogger* logger, bool isVoiceStaff);
    std::set<engraving::Lyrics*> extendedLyrics() const { return m_extendedLyrics; }
    std::map<int, engraving::Lyrics*> numberedLyrics() const { return m_numberedLyrics; }
    std::vector<engraving::Sticking*> inferredStickings() const { return m_inferredStickings; }
    void parse();
private:
    void skipLogCurrElem();
    void readElision(muse::String& formattedText);
    const LyricNumberHandler m_lyricNumberHandler;
    muse::XmlStreamReader& m_e;
    const engraving::Score* m_score = nullptr;            // the score
    MusicXmlLogger* m_logger = nullptr;            // Error logger
    std::map<int, engraving::Lyrics*> m_numberedLyrics;   // lyrics with valid number
    std::set<engraving::Lyrics*> m_extendedLyrics;        // lyrics with the extend flag set
    std::vector<engraving::Sticking*> m_inferredStickings;   // stickings with valid number
    double m_defaultY = 0.0;
    double m_relativeY = 0.0;
    muse::String m_placement;
    muse::String placement() const;
    double totalY() const { return m_defaultY + m_relativeY; }
    bool hasTotalY() const { return !muse::RealIsNull(m_defaultY) || !muse::RealIsNull(m_relativeY); }
    bool isLikelySticking(const muse::String& text, const engraving::LyricsSyllabic syllabic, const bool hasExtend);
    bool m_isVoiceStaff = true;
};

//---------------------------------------------------------
//   Notation
//      Most with text base attributes (font-*, placement)
//---------------------------------------------------------

class Notation
{
public:
    Notation(const muse::String& name, const muse::String& parent = {},
             const engraving::SymId& symId = engraving::SymId::noSym) { m_name = name; m_parent = parent; m_symId = symId; }
    void addAttribute(const muse::String& name, const muse::String& value);
    muse::String attribute(const muse::String& name) const;
    const std::map<muse::String, muse::String>& attributes() const { return m_attributes; }
    muse::String name() const { return m_name; }
    muse::String parent() const { return m_parent; }
    void setSymId(const engraving::SymId& symId) { m_symId = symId; }
    engraving::SymId symId() const { return m_symId; }
    void setSubType(const muse::String& subType) { m_subType = subType; }
    muse::String subType() const { return m_subType; }
    muse::String print() const;
    void setText(const muse::String& text) { m_text = text; }
    muse::String text() const { return m_text; }
    static Notation notationWithAttributes(const muse::String& name, const std::vector<muse::XmlStreamReader::Attribute>& attributes,
                                           const muse::String& parent = {}, const engraving::SymId& symId = engraving::SymId::noSym);
private:
    muse::String m_name;
    muse::String m_parent;
    engraving::SymId m_symId = engraving::SymId::noSym;
    muse::String m_subType;
    muse::String m_text;
    std::map<muse::String, muse::String> m_attributes;
};

//---------------------------------------------------------
//   forward references and defines
//---------------------------------------------------------

struct DelayedArpeggio
{
    muse::String m_arpeggioType = u"";
    int m_arpeggioNo = 0;

    DelayedArpeggio(muse::String arpType, int no)
        : m_arpeggioType(arpType), m_arpeggioNo(no) {}

    DelayedArpeggio()
        : m_arpeggioType(muse::String(u"")), m_arpeggioNo(0) {}

    void clear() { m_arpeggioType = u""; m_arpeggioNo = 0; }
};

class MusicXmlLogger;
class MusicXmlDelayedDirectionElement;
class MusicXmlInferredFingering;
class MusicXmlParserPass2;

using DelayedDirectionsList = std::vector<MusicXmlDelayedDirectionElement*>;
using InferredFingeringsList = std::vector<MusicXmlInferredFingering*>;
using SlurStack = std::array<SlurDesc, MAX_NUMBER_LEVEL>;
using TrillStack = std::array<engraving::Trill*, MAX_NUMBER_LEVEL>;
using BracketsStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using OttavasStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using HairpinsStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using InferredHairpinsStack = std::vector<engraving::Hairpin*>;
using InferredTempoLineStack = std::vector<engraving::GradualTempoChange*>;
using SpannerStack = std::array<MusicXmlExtendedSpannerDesc, MAX_NUMBER_LEVEL>;
using SpannerSet = std::set<engraving::Spanner*>;
using DelayedArpMap = std::map<int, DelayedArpeggio>;
using SegnoStack = std::map<int, engraving::Marker*>;
using SystemElements = std::multimap<int, engraving::EngravingItem*>;

// Ties are identified by the pitch and track of their first note
typedef std::pair<int, engraving::track_idx_t> TieLocation;
using MusicXmlTieMap = std::map<TieLocation, engraving::Tie*>;

//---------------------------------------------------------
//   MusicXmlParserNotations
//---------------------------------------------------------

class MusicXmlParserNotations
{
public:
    MusicXmlParserNotations(muse::XmlStreamReader& e, engraving::Score* score, MusicXmlLogger* logger, MusicXmlParserPass2& pass2);
    void parse();
    void addToScore(engraving::ChordRest* const cr, engraving::Note* const note, const int tick, SlurStack& slurs,
                    engraving::Glissando* glissandi[MAX_NUMBER_LEVEL][2], MusicXmlSpannerMap& spanners, TrillStack& trills,
                    MusicXmlTieMap& ties, std::vector<engraving::Note*>& unstartedTieNotes, std::vector<engraving::Note*>& unendedTieNotes,
                    ArpeggioMap& arpMap, DelayedArpMap& delayedArps);
    muse::String errors() const { return m_errors; }
    MusicXmlTupletDesc tupletDesc() const { return m_tupletDesc; }
    bool hasTremolo() const { return m_hasTremolo; }
    muse::String tremoloType() const { return m_tremoloType; }
    muse::String tremoloSmufl() const { return m_tremoloSmufl; }
    int tremoloNr() const { return m_tremoloNr; }
    bool mustStopGraceAFter() const { return m_slurStop || m_wavyLineStop; }
private:
    void addError(const muse::String& error);      // Add an error to be shown in the GUI
    void addNotation(const Notation& notation, engraving::ChordRest* const cr, engraving::Note* const note);
    void addTechnical(const Notation& notation, engraving::Note* note);
    void arpeggio();
    void harmonic();
    void harmonMute();
    void hole();
    void articulations();
    void dynamics();
    void fermata();
    void glissandoSlide();
    void mordentNormalOrInverted();
    void ornaments();
    void slur();
    void skipLogCurrElem();
    void technical();
    void otherTechnical();
    void tied();
    void tuplet();
    void otherNotation();
    muse::XmlStreamReader& m_e;
    MusicXmlParserPass2& m_pass2;
    engraving::Score* m_score = nullptr;                         // the score
    MusicXmlLogger* m_logger = nullptr;                              // the error logger
    muse::String m_errors;                    // errors to present to the user
    MusicXmlTupletDesc m_tupletDesc;
    muse::String m_dynamicsPlacement;
    engraving::StringList m_dynamicsList;
    std::vector<Notation> m_notations;
    bool m_hasTremolo = false;
    muse::String m_tremoloType;
    int m_tremoloNr = 0;
    muse::String m_tremoloSmufl;
    muse::String m_wavyLineType;
    int m_wavyLineNo = 0;
    muse::String m_arpeggioType;
    int m_arpeggioNo = 0;
    bool m_slurStop = false;
    bool m_slurStart = false;
    bool m_wavyLineStop = false;
};

//---------------------------------------------------------
//   MusicXmlParserPass2
//---------------------------------------------------------

class MusicXmlParserPass2
{
public:
    MusicXmlParserPass2(engraving::Score* score, MusicXmlParserPass1& pass1, MusicXmlLogger* logger);
    engraving::Err parse(const muse::ByteArray& data);
    muse::String errors() const { return m_errors; }

    // part specific data interface functions
    void addSpanner(const MusicXmlSpannerDesc& desc);
    MusicXmlExtendedSpannerDesc& getSpanner(const MusicXmlSpannerDesc& desc);
    void clearSpanner(const MusicXmlSpannerDesc& desc);
    void deleteHandledSpanner(engraving::SLine* const& spanner);
    int divs() { return m_divs; }

    engraving::SLine* delayedOttava() { return m_delayedOttava; }
    void setDelayedOttava(engraving::SLine* ottava) { m_delayedOttava = ottava; }

    void addInferredHairpin(engraving::Hairpin* hp) { m_inferredHairpins.push_back(hp); }
    const InferredHairpinsStack& getInferredHairpins() { return m_inferredHairpins; }

    void addInferredTempoLine(engraving::GradualTempoChange* hp) { m_inferredTempoLines.push_back(hp); }
    const InferredTempoLineStack& getInferredTempoLine() { return m_inferredTempoLines; }

    void addSystemElement(engraving::EngravingItem* el, const engraving::Fraction& tick) { m_sysElements.insert({ tick.ticks(), el }); }
    const SystemElements systemElements() const { return m_sysElements; }

    InferredPercInstr inferredPercInstr(const engraving::Fraction& tick, const engraving::track_idx_t trackIdx);
    void addInferredPercInstr(InferredPercInstr instr)
    {
        m_inferredPerc.push_back(instr);
    }

    void addElemOffset(engraving::EngravingItem* el, engraving::track_idx_t track, const muse::String& placement,
                       engraving::Measure* measure, const engraving::Fraction& tick);

private:
    void addError(const muse::String& error);      // Add an error to be shown in the GUI
    void initPartState(const muse::String& partId);
    SpannerSet findIncompleteSpannersAtPartEnd();
    engraving::Err parse();
    void scorePartwise();
    void partList();
    void scorePart();
    void part();
    void measure(const muse::String& partId, const engraving::Fraction time);
    void measureLayout(engraving::Measure* measure);
    void setMeasureRepeats(const engraving::staff_idx_t scoreRelStaff, engraving::Measure* measure);
    void attributes(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick);
    void measureStyle(engraving::Measure* measure);
    void barline(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick);
    void key(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick);
    void clef(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick);
    void time(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick);
    void divisions();
    engraving::Note* note(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction sTime,
                          const engraving::Fraction prevTime, engraving::Fraction& missingPrev, engraving::Fraction& dura,
                          engraving::Fraction& missingCurr, muse::String& currentVoice, GraceChordList& gcl, size_t& gac, Beams& currBeams,
                          FiguredBassList& fbl, int& alt, MusicXmlTupletStates& tupletStates, Tuplets& tuplets, ArpeggioMap& arpMap,
                          DelayedArpMap& delayedArps);
    void notePrintSpacingNo(engraving::Fraction& dura);
    engraving::FiguredBassItem* figure(const int idx, const bool paren, engraving::FiguredBass* parent);
    engraving::FiguredBass* figuredBass();
    engraving::FretDiagram* frame();
    void harmony(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& sTime, HarmonyMap& harmonyMap);
    engraving::Accidental* accidental();
    void beam(std::map<int, muse::String>& beamTypes);
    void duration(engraving::Fraction& dura);
    void forward(engraving::Fraction& dura);
    void backup(engraving::Fraction& dura);
    void timeModification(engraving::Fraction& timeMod, engraving::TDuration& normalType);
    void stem(engraving::DirectionV& sd, bool& nost);
    void doEnding(const muse::String& partId, engraving::Measure* measure, const muse::String& number, const muse::String& type,
                  const engraving::Color color, const muse::String& text, const bool print);
    void staffDetails(const muse::String& partId, engraving::Measure* measure = nullptr);
    void staffTuning(engraving::StringData* t);
    void skipLogCurrElem();

    // multi-measure rest state handling
    void setMultiMeasureRestCount(int count);
    int getAndDecMultiMeasureRestCount();

    void xmlSetDrumsetPitch(engraving::Note* note, const engraving::Chord* chord, const engraving::Staff* staff, int step, int octave,
                            engraving::NoteHeadGroup headGroup, engraving::DirectionV& stemDir, engraving::Instrument* instrument);

    // generic pass 2 data

    muse::XmlStreamReader m_e;
    int m_divs = 0;                        // the current divisions value
    engraving::Score* m_score = nullptr;              // the score
    MusicXmlParserPass1& m_pass1;          // the pass1 results
    MusicXmlLogger* m_logger = nullptr;    // Error logger
    muse::String m_errors;                       // Errors to present to the user

    // part specific data (TODO: move to part-specific class)

    // Measure duration according to last timesig read
    // TODO: store timesigs read in pass 1, use those instead
    // or use score->sigmap() ?
    engraving::Fraction m_timeSigDura;

    SlurStack m_slurs { {} };
    TrillStack m_trills { {} };            // Current trills
    BracketsStack m_brackets;
    OttavasStack m_ottavas;                // Current ottavas
    HairpinsStack m_hairpins;              // Current hairpins
    InferredHairpinsStack m_inferredHairpins;
    InferredTempoLineStack m_inferredTempoLines;
    MusicXmlExtendedSpannerDesc m_dummyNewMusicXmlSpannerDesc;

    engraving::Glissando* m_glissandi[MAX_NUMBER_LEVEL][2] { {} };   // Current slides ([0]) / glissandi ([1])

    MusicXmlTieMap m_ties;
    std::vector<engraving::Note*> m_unstartedTieNotes;
    std::vector<engraving::Note*> m_unendedTieNotes;
    engraving::Volta* m_lastVolta = nullptr;
    bool m_hasDrumset = false;                     // drumset defined TODO: move to pass 1

    MusicXmlSpannerMap m_spanners;

    MusicXmlExtendedSpannerDesc m_pedal;           // Current pedal
    engraving::Pedal* m_pedalContinue = nullptr;              // Current pedal type="change" requiring fixup
    engraving::Harmony* m_harmony = nullptr;                  // Current harmony
    engraving::Chord* m_tremStart = nullptr;                  // Starting chord for current tremolo
    engraving::FiguredBass* m_figBass = nullptr;              // Current figured bass element (to attach to next note)
    engraving::SLine* m_delayedOttava = nullptr;              // Current delayed ottava
    SegnoStack m_segnos;                           // List of segno markings
    int m_multiMeasureRestCount = 0;
    int m_measureNumber = 0;                       // Current measure number as written in the score
    MusicXmlLyricsExtend m_extendedLyrics;         // Lyrics with "extend" requiring fixup
    std::vector<GraceNoteLyrics> m_graceNoteLyrics;   // Lyrics to be moved from grace note to main note

    MusicXmlSlash m_measureStyleSlash = MusicXmlSlash::NONE;   // Are we inside a measure to be displayed as slashes?

    size_t m_nstaves = 0;                          // Number of staves in current part
    std::vector<int> m_measureRepeatNumMeasures;
    std::vector<int> m_measureRepeatCount;

    SystemElements m_sysElements;
    InferredPercList m_inferredPerc;
};

//---------------------------------------------------------
//   MusicXmlParserDirection
//---------------------------------------------------------

class MusicXmlParserDirection
{
public:
    MusicXmlParserDirection(muse::XmlStreamReader& e, engraving::Score* score, MusicXmlParserPass1& pass1, MusicXmlParserPass2& pass2,
                            MusicXmlLogger* logger);
    void direction(const muse::String& partId, engraving::Measure* measure, const engraving::Fraction& tick, MusicXmlSpannerMap& spanners,
                   DelayedDirectionsList& delayedDirections, InferredFingeringsList& inferredFingerings, HarmonyMap& harmonyMap,
                   bool& measureHasCoda, SegnoStack& segnos);

    double totalY() const { return m_defaultY + m_relativeY; }
    muse::String placement() const;

private:
    void directionType(std::vector<MusicXmlSpannerDesc>& starts, std::vector<MusicXmlSpannerDesc>& stops);
    void bracket(const muse::String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
                 std::vector<MusicXmlSpannerDesc>& stops);
    void octaveShift(const muse::String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
                     std::vector<MusicXmlSpannerDesc>& stops);
    void pedal(const muse::String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
               std::vector<MusicXmlSpannerDesc>& stops);
    void dashes(const muse::String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
                std::vector<MusicXmlSpannerDesc>& stops);
    void wedge(const muse::String& type, const int number, std::vector<MusicXmlSpannerDesc>& starts,
               std::vector<MusicXmlSpannerDesc>& stops);
    muse::String metronome(double& r);
    void sound();
    void play();
    void dynamics();
    void otherDirection();
    void handleRepeats(engraving::Measure* measure, const engraving::Fraction tick, bool& measureHasCoda, SegnoStack& segnos,
                       DelayedDirectionsList& delayedDirections);
    engraving::Marker* findMarker(const muse::String& repeat) const;
    engraving::Jump* findJump(const muse::String& repeat) const;
    void handleNmiCmi(engraving::Measure* measure, const engraving::Fraction& tick, DelayedDirectionsList& delayedDirections);
    void handleChordSym(const engraving::Fraction& tick, HarmonyMap& harmonyMap);
    void handleTempo(muse::String& wordsString);
    muse::String matchRepeat(const muse::String& plainWords) const;
    void skipLogCurrElem();
    bool isLikelyCredit(const engraving::Fraction& tick) const;
    void textToDynamic(muse::String& text);
    void textToCrescLine(muse::String& text);
    void addInferredHairpin(const engraving::Fraction& tick, const bool isVocalStaff);
    void addInferredTempoLine(const engraving::Fraction& tick);
    bool isLyricBracket() const;
    bool isLikelySubtitle(const engraving::Fraction& tick) const;
    bool isLikelyLegallyDownloaded(const engraving::Fraction& tick) const;
    bool isLikelyTempoText(const engraving::track_idx_t track) const;
    void handleFraction();
    bool isLikelyTempoLine(const engraving::track_idx_t track) const;
    engraving::Text* addTextToHeader(const engraving::TextStyleType textStyleType);
    void hideRedundantHeaderText(const engraving::Text* inferredText, const std::vector<muse::String> metaTags);
    bool isLikelyFingering(const muse::String& fingeringStr) const;
    bool isLikelySticking();
    bool isLikelyDynamicRange() const;
    engraving::PlayingTechniqueType getPlayingTechnique() const;
    void handleDrumInstrument(bool isPerc, engraving::Fraction tick) const;

    // void terminateInferredLine(const std::vector<TextLineBase*> lines, const Fraction& tick);

    bool hasTotalY() const { return m_hasRelativeY || m_hasDefaultY; }

    muse::XmlStreamReader& m_e;
    engraving::Score* m_score = nullptr;                    // the score
    MusicXmlParserPass1& m_pass1;                // the pass1 results
    MusicXmlParserPass2& m_pass2;                // the pass2 results
    MusicXmlLogger* m_logger = nullptr;          // Error logger

    engraving::Color m_color;
    engraving::Hairpin* m_inferredHairpinStart = nullptr;
    engraving::GradualTempoChange* m_inferredTempoLineStart = nullptr;
    muse::StringList m_dynamicsList;
    muse::String m_fontFamily;
    muse::String m_enclosure;
    muse::String m_wordsText;
    muse::String m_metroText;
    muse::String m_rehearsalText;
    muse::String m_dynaVelocity;
    muse::String m_sndCoda;
    muse::String m_sndDacapo;
    muse::String m_sndDalsegno;
    muse::String m_sndFine;
    muse::String m_sndSegno;
    muse::String m_sndToCoda;
    muse::String m_codaId;
    muse::String m_segnoId;
    muse::String m_placement;
    muse::String m_play;
    bool m_hasDefaultY = false;
    double m_defaultY = 0.0;
    bool m_hasRelativeY = false;
    double m_relativeY = 0.0;
    double m_relativeX = 0.0;
    double m_tpoMetro = 0.0;                   // tempo according to metronome
    double m_tpoSound = 0.0;                   // tempo according to sound
    bool m_visible = true;
    bool m_systemDirection = false;
    std::vector<engraving::EngravingItem*> m_elems;
    engraving::Fraction m_offset;
    engraving::track_idx_t m_track = muse::nidx;
};

//---------------------------------------------------------
//   MusicXmlDelayedDirectionElement
//---------------------------------------------------------
/**
 Helper class to allow Direction elements to be sorted by _totalY
 before being added to the score.
 */

class MusicXmlDelayedDirectionElement
{
public:
    MusicXmlDelayedDirectionElement(double totalY, engraving::EngravingItem* element, engraving::track_idx_t track,
                                    muse::String placement, engraving::Measure* measure, engraving::Fraction tick)
        : m_totalY(totalY),  m_element(element), m_track(track), m_placement(placement),
        m_measure(measure), m_tick(tick) {}
    double totalY() const { return m_totalY; }
    engraving::EngravingItem* element() const { return m_element; }
    engraving::track_idx_t track() const { return m_track; }
    const engraving::Fraction& tick() const { return m_tick; }
    const muse::String& placement() const { return m_placement; }
    engraving::Measure* measure() const { return m_measure; }

private:
    double m_totalY = 0.0;
    engraving::EngravingItem* m_element = nullptr;
    engraving::track_idx_t m_track = muse::nidx;
    muse::String m_placement;
    engraving::Measure* m_measure = nullptr;
    engraving::Fraction m_tick;
};

//---------------------------------------------------------
//   MusicXmlInferredFingering
//---------------------------------------------------------
/**
 Helper class to allow Direction elements to be reinterpreted as fingerings
 */

class MusicXmlInferredFingering
{
public:
    MusicXmlInferredFingering(double totalY, engraving::EngravingItem* element, muse::String& text, engraving::track_idx_t track,
                              muse::String placement, engraving::Measure* measure, engraving::Fraction tick);
    double totalY() const { return m_totalY; }
    engraving::Fraction tick() const { return m_tick; }
    engraving::track_idx_t track() const { return m_track; }
    std::vector<muse::String> fingerings() const { return m_fingerings; }
    bool findAndAddToNotes(engraving::Measure* measure);
    MusicXmlDelayedDirectionElement* toDelayedDirection();

private:
    double m_totalY;
    engraving::EngravingItem* m_element;
    muse::String m_text;
    std::vector<muse::String> m_fingerings;
    engraving::track_idx_t m_track;
    muse::String m_placement;
    engraving::Measure* m_measure;
    engraving::Fraction m_tick;

    void roundTick(engraving::Measure* measure);
    void addToNotes(std::vector<engraving::Note*>& notes) const;
};
} // namespace Ms
