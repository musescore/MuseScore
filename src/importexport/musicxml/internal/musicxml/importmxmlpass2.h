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

#include "libmscore/score.h"
#include "libmscore/tuplet.h"
#include "importxmlfirstpass.h"
#include "importmxmlpass1.h"
#include "musicxml.h" // a.o. for Slur
#include "musicxmlsupport.h"

namespace Ms {
//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

using GraceChordList = QList<Chord*>;
using FiguredBassList = QVector<FiguredBass*>;
//      typedef QList<Chord*> GraceChordList;
//      typedef QVector<FiguredBass*> FiguredBassList;
using Tuplets = std::map<QString, Tuplet*>;

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
    Placement placement;
    TupletBracketType bracket;
    TupletNumberType shownumber;
};

//---------------------------------------------------------
//   MusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlSpannerDesc {
    SLine* _sp;
    ElementType _tp;
    int _nr;
    MusicXmlSpannerDesc(SLine* sp, ElementType tp, int nr)
        : _sp(sp), _tp(tp), _nr(nr) {}
    MusicXmlSpannerDesc(ElementType tp, int nr)
        : _sp(0), _tp(tp), _nr(nr) {}
};

//---------------------------------------------------------
//   NewMusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlExtendedSpannerDesc {
    SLine* _sp { nullptr };
    Fraction _tick2 { 0, 0 };
    int _track2 {};
    bool _isStarted { false };
    bool _isStopped { false };
    MusicXmlExtendedSpannerDesc() {}
    QString toString() const;
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
    void setExtend(const int no, const int track, const Fraction& tick);

private:
    QSet<Lyrics*> _lyrics;
};

//---------------------------------------------------------
//   MusicXMLParserLyric
//---------------------------------------------------------

class MusicXMLParserLyric
{
public:
    MusicXMLParserLyric(const LyricNumberHandler lyricNumberHandler, QXmlStreamReader& e, Score* score, MxmlLogger* logger);
    QSet<Lyrics*> extendedLyrics() const { return _extendedLyrics; }
    QMap<int, Lyrics*> numberedLyrics() const { return _numberedLyrics; }
    void parse();
private:
    void skipLogCurrElem();
    const LyricNumberHandler _lyricNumberHandler;
    QXmlStreamReader& _e;
    Score* const _score;                        // the score
    MxmlLogger* _logger;                        ///< Error logger
    QMap<int, Lyrics*> _numberedLyrics;   // lyrics with valid number
    QSet<Lyrics*> _extendedLyrics;        // lyrics with the extend flag set
};

//---------------------------------------------------------
//   Notation
//      Most with text base attributes (font-*, placement)
//---------------------------------------------------------

class Notation
{
public:
    Notation(const QString& name, const QString& parent = "",
             const SymId& symId = SymId::noSym) { _name = name; _parent = parent; _symId = symId; }
    void addAttribute(const QStringRef name, const QStringRef value);
    QString attribute(const QString& name) const;
    QString name() const { return _name; }
    QString parent() const { return _parent; }
    void setSymId(const SymId& symId) { _symId = symId; }
    SymId symId() const { return _symId; }
    void setSubType(const QString& subType) { _subType = subType; }
    QString subType() const { return _subType; }
    QString print() const;
    void setText(const QString& text) { _text = text; }
    QString text() const { return _text; }
    static Notation notationWithAttributes(const QString& name, const QXmlStreamAttributes attributes, const QString& parent = "",
                                           const SymId& symId = SymId::noSym);
private:
    QString _name;
    QString _parent;
    SymId _symId { SymId::noSym };
    QString _subType;
    QString _text;
    std::map<QString, QString> _attributes;
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
    MusicXMLParserNotations(QXmlStreamReader& e, Score* score, MxmlLogger* logger);
    void parse();
    void addToScore(ChordRest* const cr, Note* const note, const int tick, SlurStack& slurs, Glissando* glissandi[MAX_NUMBER_LEVEL][2],
                    MusicXmlSpannerMap& spanners, TrillStack& trills, Tie*& tie);
    MusicXmlTupletDesc tupletDesc() const { return _tupletDesc; }
    QString tremoloType() const { return _tremoloType; }
    int tremoloNr() const { return _tremoloNr; }
    bool mustStopGraceAFter() const { return _slurStop || _wavyLineStop; }
private:
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
    QXmlStreamReader& _e;
    Score* const _score;                        // the score
    MxmlLogger* _logger;                              // the error logger
    MusicXmlTupletDesc _tupletDesc;
    QString _dynamicsPlacement;
    QStringList _dynamicsList;
    std::vector<Notation> _notations;
    SymId _breath { SymId::noSym };
    QString _tremoloType;
    int _tremoloNr { 0 };
    QString _wavyLineType;
    int _wavyLineNo { 0 };
    QString _arpeggioType;
    bool _slurStop { false };
    bool _wavyLineStop { false };
};

//---------------------------------------------------------
//   MusicXMLParserPass2
//---------------------------------------------------------

class MusicXMLParserPass2
{
public:
    MusicXMLParserPass2(Score* score, MusicXMLParserPass1& pass1, MxmlLogger* logger);
    Score::FileError parse(QIODevice* device);

    // part specific data interface functions
    void addSpanner(const MusicXmlSpannerDesc& desc);
    MusicXmlExtendedSpannerDesc& getSpanner(const MusicXmlSpannerDesc& desc);
    void clearSpanner(const MusicXmlSpannerDesc& desc);

private:
    void initPartState(const QString& partId);
    SpannerSet findIncompleteSpannersAtPartEnd();
    Score::FileError parse();
    void scorePartwise();
    void partList();
    void scorePart();
    void part();
    void measChordNote(/*, const MxmlPhase2Note note, ChordRest& currChord */);
    void measChordFlush(/*, ChordRest& currChord */);
    void measure(const QString& partId, const Fraction time);
    void setMeasureRepeats(const int scoreRelStaff, Measure* measure);
    void attributes(const QString& partId, Measure* measure, const Fraction& tick);
    void measureStyle(Measure* measure);
    void barline(const QString& partId, Measure* measure, const Fraction& tick);
    void key(const QString& partId, Measure* measure, const Fraction& tick);
    void clef(const QString& partId, Measure* measure, const Fraction& tick);
    void time(const QString& partId, Measure* measure, const Fraction& tick);
    void divisions();
    void transpose(const QString& partId, const Fraction& tick);
    Note* note(const QString& partId, Measure* measure, const Fraction sTime, const Fraction prevTime, Fraction& missingPrev,
               Fraction& dura, Fraction& missingCurr, QString& currentVoice, GraceChordList& gcl, int& gac, Beam*& beam,
               FiguredBassList& fbl, int& alt, MxmlTupletStates& tupletStates, Tuplets& tuplets);
    void notePrintSpacingNo(Fraction& dura);
    FiguredBassItem* figure(const int idx, const bool paren);
    FiguredBass* figuredBass();
    FretDiagram* frame();
    void harmony(const QString& partId, Measure* measure, const Fraction sTime);
    Accidental* accidental();
    void beam(Beam::Mode& beamMode);
    void duration(Fraction& dura);
    void forward(Fraction& dura);
    void backup(Fraction& dura);
    void timeModification(Fraction& timeMod, TDuration& normalType);
    void stem(Direction& sd, bool& nost);
    void doEnding(const QString& partId, Measure* measure, const QString& number, const QString& type, const QString& text);
    void staffDetails(const QString& partId);
    void staffTuning(StringData* t);
    void skipLogCurrElem();

    // multi-measure rest state handling
    void setMultiMeasureRestCount(int count);
    int getAndDecMultiMeasureRestCount();

    // generic pass 2 data

    QXmlStreamReader _e;
    int _divs;                            // the current divisions value
    Score* const _score;                  // the score
    MusicXMLParserPass1& _pass1;          // the pass1 results
    MxmlLogger* _logger;                  ///< Error logger

    // part specific data (TODO: move to part-specific class)

    // Measure duration according to last timesig read
    // TODO: store timesigs read in pass 1, use those instead
    // or use score->sigmap() ?
    Fraction _timeSigDura;

    SlurStack _slurs { {} };
    TrillStack _trills { {} };            ///< Current trills
    BracketsStack _brackets;
    OttavasStack _ottavas;                ///< Current ottavas
    HairpinsStack _hairpins;              ///< Current hairpins
    MusicXmlExtendedSpannerDesc _dummyNewMusicXmlSpannerDesc;

    Glissando* _glissandi[MAX_NUMBER_LEVEL][2];     ///< Current slides ([0]) / glissandi ([1])

    Tie* _tie;
    Volta* _lastVolta;
    bool _hasDrumset;                             ///< drumset defined TODO: move to pass 1

    MusicXmlSpannerMap _spanners;

    MusicXmlExtendedSpannerDesc _pedal;           ///< Current pedal
    Pedal* _pedalContinue;                        ///< Current pedal type="change" requiring fixup
    Harmony* _harmony;                            ///< Current harmony
    Chord* _tremStart;                            ///< Starting chord for current tremolo
    FiguredBass* _figBass;                        ///< Current figured bass element (to attach to next note)
    int _multiMeasureRestCount;
    MusicXmlLyricsExtend _extendedLyrics;         ///< Lyrics with "extend" requiring fixup

    MusicXmlSlash _measureStyleSlash;             ///< Are we inside a measure to be displayed as slashes?

    int _nstaves;                                 ///< Number of staves in current part
    std::vector<int> _measureRepeatNumMeasures;
    std::vector<int> _measureRepeatCount;
};

//---------------------------------------------------------
//   MusicXMLParserDirection
//---------------------------------------------------------

class MusicXMLParserDirection
{
public:
    MusicXMLParserDirection(QXmlStreamReader& e, Score* score, const MusicXMLParserPass1& pass1, MusicXMLParserPass2& pass2,
                            MxmlLogger* logger);
    void direction(const QString& partId, Measure* measure, const Fraction& tick, const int divisions, MusicXmlSpannerMap& spanners);

private:
    QXmlStreamReader& _e;
    Score* const _score;                        // the score
    const MusicXMLParserPass1& _pass1;          // the pass1 results
    MusicXMLParserPass2& _pass2;                // the pass2 results
    MxmlLogger* _logger;                        ///< Error logger

    QStringList _dynamicsList;
    QString _enclosure;
    QString _wordsText;
    QString _metroText;
    QString _rehearsalText;
    QString _dynaVelocity;
    QString _tempo;
    QString _sndCapo;
    QString _sndCoda;
    QString _sndDacapo;
    QString _sndDalsegno;
    QString _sndSegno;
    QString _sndFine;
    bool _hasDefaultY;
    qreal _defaultY;
    bool _coda;
    bool _segno;
    double _tpoMetro;                   // tempo according to metronome
    double _tpoSound;                   // tempo according to sound
    QList<Element*> _elems;
    Fraction _offset;

    void directionType(QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    void bracket(const QString& type, const int number, QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    void octaveShift(const QString& type, const int number, QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    void pedal(const QString& type, const int number, QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    void dashes(const QString& type, const int number, QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    void wedge(const QString& type, const int number, QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops);
    QString metronome(double& r);
    void sound();
    void dynamics();
    void handleRepeats(Measure* measure, const int track);
    void skipLogCurrElem();
};
} // namespace Ms
#endif
