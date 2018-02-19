//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

typedef QList<Chord*> GraceChordList;
typedef QVector<FiguredBass*> FiguredBassList;

//---------------------------------------------------------
//   MxmlStartStop
//---------------------------------------------------------

enum class MxmlStartStop : char {
      START, STOP, NONE
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
      Tuplet::BracketType bracket;
      Tuplet::NumberType shownumber;
      };

//---------------------------------------------------------
//   MusicXmlSpannerDesc
//---------------------------------------------------------

struct MusicXmlSpannerDesc {
      SLine* sp;
      ElementType tp;
      int nr;
      MusicXmlSpannerDesc(SLine* _sp, ElementType _tp, int _nr) : sp(_sp), tp(_tp), nr(_nr) {}
      MusicXmlSpannerDesc(ElementType _tp, int _nr) : sp(0), tp(_tp), nr(_nr) {}
      };

//---------------------------------------------------------
//   MusicXmlLyricsExtend
//---------------------------------------------------------

class MusicXmlLyricsExtend {
public:
      MusicXmlLyricsExtend() {}
      void init();
      void addLyric(Lyrics* const lyric);
      void setExtend(const int no, const int track, const int tick);

private:
      QSet<Lyrics*> _lyrics;
      };

//---------------------------------------------------------
//   MusicXMLParserPass2
//---------------------------------------------------------

class FretDiagram;
class FiguredBassItem;
class Glissando;
class Pedal;
class Trill;
class MxmlLogger;

using SlurStack = std::array<SlurDesc, MAX_NUMBER_LEVEL>;
using TrillStack = std::array<Trill*, MAX_NUMBER_LEVEL>;
using BracketsStack = std::array<SLine*, MAX_BRACKETS>;
using DashesStack = std::array<SLine*, MAX_DASHES>;
using OttavasStack = std::array<SLine*, MAX_NUMBER_LEVEL>;
using HairpinsStack = std::array<SLine*, MAX_NUMBER_LEVEL>;

class MusicXMLParserPass2 {
public:
      MusicXMLParserPass2(Score* score, MusicXMLParserPass1& pass1, MxmlLogger* logger);
      void initPartState(const QString& partId);
      Score::FileError parse(QIODevice* device);
      Score::FileError parse();
      void scorePartwise();
      void partList();
      void scorePart();
      void part();
      void measChordNote( /*, const MxmlPhase2Note note, ChordRest& currChord */);
      void measChordFlush( /*, ChordRest& currChord */);
      void measure(const QString& partId, const Fraction time);
      void attributes(const QString& partId, Measure* measure, const int tick);
      void measureStyle(Measure* measure);
      void print(Measure* measure);
      void barline(const QString& partId, Measure* measure);
      void key(const QString& partId, Measure* measure, const int tick);
      void clef(const QString& partId, Measure* measure, const int tick);
      void time(const QString& partId, Measure* measure, const int tick);
      void divisions();
      void transpose(const QString& partId);
      Note* note(const QString& partId, Measure* measure, const Fraction sTime, const Fraction prevTime,
                 Fraction& dura, QString& currentVoice, GraceChordList& gcl, int& gac,
                 Beam*& beam, FiguredBassList& fbl, int& alt);
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
      //void pitch(int& step, int& alter, int& oct, AccidentalType& accid);
      void lyric(QMap<int, Lyrics*>& numbrdLyrics, QMap<int, Lyrics*>& defyLyrics,
                 QList<Lyrics*>& unNumbrdLyrics, QSet<Lyrics*>& extLyrics);
      void slur(ChordRest* cr, const int tick, const int track, bool& lastGraceAFter);
      void tied(Note* note, const int track);
      void articulations(ChordRest* cr, SymId& breath, QString& chordLineType);
      void dynamics(QString& placement, QStringList& dynamics);
      void ornaments(ChordRest* cr, QString& wavyLineType, int& wavyLineNo, QString& tremoloType, int& tremoloNr, bool& lastGraceAFter);
      void technical(Note* note, ChordRest* cr);
      void glissando(Note* note, const int tick, const int ticks, const int track);
      void notations(Note* note, ChordRest* cr, const int tick, MusicXmlTupletDesc& tupletDesc, bool& lastGraceAFter);
      void stem(Direction& sd, bool& nost);
      void fermata(ChordRest* cr);
      void tuplet(MusicXmlTupletDesc& tupletDesc);
      void doEnding(const QString& partId, Measure* measure, const QString& number, const QString& type, const QString& text);
      void staffDetails(const QString& partId);
      void staffTuning(StringData* t);
      void skipLogCurrElem();

      // part specific data interface functions
      void addSpanner(const MusicXmlSpannerDesc& desc);
      SLine* getSpanner(const MusicXmlSpannerDesc& desc);
      void clearSpanner(const MusicXmlSpannerDesc& desc);

      // multi-measure rest state handling
      void setMultiMeasureRestCount(int count);
      int getAndDecMultiMeasureRestCount();

private:
      // generic pass 2 data

      QXmlStreamReader _e;
      int _divs;                          // the current divisions value
      Score* const _score;                // the score
      MusicXMLParserPass1& _pass1;        // the pass1 results
      MxmlLogger* _logger;                ///< Error logger

      // part specific data (TODO: move to part-specific class)

      // Measure duration according to last timesig read
      // TODO: store timesigs read in pass 1, use those instead
      // or use score->sigmap() ?
      Fraction _timeSigDura;

      QVector<Tuplet*> _tuplets;          ///< Current tuplet for each track in the current part
      QVector<bool> _tuplImpls;           ///< Current tuplet implicit flag for each track in the current part
      SlurStack _slurs {{}};
      TrillStack _trills {{}};            ///< Current trills
      BracketsStack _brackets {{}};
      DashesStack _dashes {{}};
      OttavasStack _ottavas {{}};         ///< Current ottavas
      HairpinsStack _hairpins {{}};       ///< Current hairpins

      Glissando* _glissandi[MAX_NUMBER_LEVEL][2];   ///< Current slides ([0]) / glissandi ([1])

      Tie* _tie;
      Volta* _lastVolta;
      bool _hasDrumset;                           ///< drumset defined TODO: move to pass 1

      MusicXmlSpannerMap _spanners;

      SLine* _pedal;                              ///< Current pedal
      Pedal* _pedalContinue;                      ///< Current pedal type="change" requiring fixup
      Harmony* _harmony;                          ///< Current harmony
      Chord* _tremStart;                          ///< Starting chord for current tremolo
      FiguredBass* _figBass;                      ///< Current figured bass element (to attach to next note)
      int _multiMeasureRestCount;
      MusicXmlLyricsExtend _extendedLyrics;       ///< Lyrics with "extend" requiring fixup
      };

//---------------------------------------------------------
//   MusicXMLParserDirection
//---------------------------------------------------------

class MusicXMLParserDirection {
public:
      MusicXMLParserDirection(QXmlStreamReader& e, Score* score, const MusicXMLParserPass1& pass1, MusicXMLParserPass2& pass2, MxmlLogger* logger);
      void direction(const QString& partId, Measure* measure, const int tick, MusicXmlSpannerMap& spanners);

private:
      QXmlStreamReader& _e;
      Score* const _score;                      // the score
      const MusicXMLParserPass1& _pass1;        // the pass1 results
      MusicXMLParserPass2& _pass2;              // the pass2 results
      MxmlLogger* _logger;                      ///< Error logger

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
      double _tpoMetro;                 // tempo according to metronome
      double _tpoSound;                 // tempo according to sound
      QList<Element*> _elems;

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
