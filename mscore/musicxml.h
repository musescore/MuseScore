//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: musicxml.h 5595 2012-04-29 15:30:32Z lvinken $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __MUSICXML_H__
#define __MUSICXML_H__

/**
 \file
 Definition of class MusicXML
*/

#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "libmscore/pitchspelling.h"
#include "importxmlfirstpass.h"
#include "musicxmlsupport.h"

namespace Ms {

class Beam;
class Chord;
class ChordRest;
class FiguredBass;
class Glissando;
class Hairpin;
class Harmony;
class Instrument;
class Lyrics;
class Measure;
class Note;
class Ottava;
class Part;
class Pedal;
class Score;
class Slur;
class Spanner;
class TextLine;
class Tie;
class Trill;
class Tuplet;
class Volta;
enum class StaffTypes : char;

//---------------------------------------------------------
//   MusicXmlWedge
//---------------------------------------------------------

struct MusicXmlWedge {
      int number;
      int startTick;
      int subType;
      qreal rx;
      qreal ry;
      bool above;
      bool hasYoffset;
      qreal yoffset;
      };

//---------------------------------------------------------
//   MusicXmlPartGroup
//---------------------------------------------------------

struct MusicXmlPartGroup {
      int span;
      int start;
      BracketType type;
      bool barlineSpan;
      };

const int MAX_LYRICS       = 16;
const int MAX_PART_GROUPS  = 8;
const int MAX_NUMBER_LEVEL = 6; // maximum number of overlapping MusicXML objects
const int MAX_BRACKETS     = 9; // 8;
const int MAX_DASHES       = 8;

//---------------------------------------------------------
//   CreditWords
//    a single parsed MusicXML credit-words element
//---------------------------------------------------------

struct CreditWords {
      double defaultX;
      double defaultY;
      QString justify;
      QString hAlign;
      QString vAlign;
      QString words;
      CreditWords(double a, double b, QString c, QString d, QString e, QString f)
            {
            defaultX = a;
            defaultY = b;
            justify  = c;
            hAlign   = d;
            vAlign   = e;
            words    = f;
            }
      };

typedef  QList<CreditWords*> CreditWordsList;
typedef  CreditWordsList::iterator iCreditWords;
typedef  CreditWordsList::const_iterator ciCreditWords;

//---------------------------------------------------------
//   JumpMarkerDesc
//---------------------------------------------------------

/**
 The description of Jumps and Markers to be added later
*/

class JumpMarkerDesc {
      Element* _el;
      Measure* _meas;

public:
      JumpMarkerDesc(Element* el, Measure* meas) : _el(el), _meas(meas) {}
      Element* el() const { return _el; }
      Measure* meas() const { return _meas; }
      };

typedef QList<JumpMarkerDesc> JumpMarkerDescList;
      
//---------------------------------------------------------
//   SlurDesc
//---------------------------------------------------------

/**
 The description of Slurs being handled
 */

class SlurDesc {
public:
      enum class State : char { NONE, START, STOP };
      SlurDesc() : _slur(0), _state(State::NONE) {}
      Slur* slur() const { return _slur; }
      void start(Slur* slur) { _slur = slur; _state = State::START; }
      void stop(Slur* slur) { _slur = slur; _state = State::STOP; }
      bool isStart() const { return _state == State::START; }
      bool isStop() const { return _state == State::STOP; }
private:
      Slur* _slur;
      State _state;
};

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

typedef std::vector<MusicXmlPartGroup*> MusicXmlPartGroupList;
typedef QMap<SLine*, QPair<int, int> > MusicXmlSpannerMap;

/**
 The MusicXML importer.
*/

class MusicXml {
      Score* score;
      QMap<QString, VoiceDesc> voicelist;
      QVector<Fraction> measureLength;          ///< Length of each measure as Fraction
      QVector<int> measureStart;                ///< Start tick of each measure
      Fraction fractionTSig;                    ///< Current timesig as fraction

      SlurDesc slur[MAX_NUMBER_LEVEL];
      TextLine* bracket[MAX_BRACKETS];
      TextLine* dashes[MAX_DASHES];
      Ottava* ottavas[MAX_NUMBER_LEVEL];        ///< Current ottavas
      Hairpin* hairpins[MAX_NUMBER_LEVEL];      ///< Current hairpins
      Trill* trills[MAX_NUMBER_LEVEL];          ///< Current trills
	  Glissando* glissandi[MAX_NUMBER_LEVEL][2];   ///< Current slides ([0]) / glissandi ([1])

      Tie* tie;
      Volta* lastVolta;

      QDomDocument* doc;
      MxmlReaderFirstPass const& pass1;
      int tick;                                 ///< Current position in MuseScore time
      int maxtick;                              ///< Maxtick of a measure, used to calculate measure len
      int prevtick;                             ///< Previous notes tick (used to insert additional notes to chord)
      int lastMeasureLen;
      int multiMeasureRestCount;                ///< Remaining measures in a multi measure rest
      bool startMultiMeasureRest;               ///< Multi measure rest started in this measure
      bool hasDrumset;                          ///< Part has defined a drumset

      int divisions;                            ///< Current MusicXML divisions
      QVector<Tuplet*> tuplets;                 ///< Current tuplet for each track in the current part

      CreditWordsList credits;
      JumpMarkerDescList jumpsMarkers;

      MusicXmlPartGroupList partGroupList;
      MusicXmlSpannerMap spanners;

      Pedal* pedal;                              ///< Current pedal
      Pedal* pedalContinue;                      ///< Current pedal type="change" requiring fixup
      Harmony* harmony;                          ///< Current harmony
      Chord* tremStart;                          ///< Starting chord for current tremolo
      FiguredBass* figBass;                      ///< Current figured bass element (to attach to next note)
      QVector<FiguredBass*> figBassList;         ///< List of figured bass elements under a single note
      Beam::Mode beamMode;                       ///< Current beam mode
//      QString glissandoText;                     ///< Glissando text at glissando start
//      QString glissandoColor;                    ///< Glissando color at glissando start

      int pageWidth;                             ///< Page width read from defaults
      int pageHeight;                            ///< Page height read from defaults

      QMap<QString, MusicXMLDrumset> drumsets;   ///< Drumset for each part

      //-----------------------------

      void doCredits();
      void direction(Measure* measure, int staff, QDomElement node);
      void scorePartwise(QDomElement);
      void xmlPartList(QDomElement);
      void xmlPart(QDomElement, QString id);
      void xmlScorePart(QDomElement node, QString id, int& parts);
      Measure* xmlMeasure(Part*, QDomElement, int, Fraction measureLen, KeySig*);
      void xmlAttributes(Measure*, int stave, QDomElement node, KeySig*);
      void xmlLyric(int trk, QDomElement e,
                    QMap<int, Lyrics*>& numbrdLyrics,
                    QMap<int, Lyrics*>& defyLyrics,
                    QList<Lyrics*>& unNumbrdLyrics);
      void xmlNotations(Note* note, ChordRest* cr, int trk, int tick, int ticks, QDomElement node);
      Note* xmlNote(Measure*, int stave, const QString& partId, Beam*& beam, QString& currentVoice, QDomElement node, QList<Chord*>& graceNotes, int& alt);
      void xmlHarmony(QDomElement node, int tick, Measure* m, int staff);
      StaffTypes xmlClef(QDomElement, int staffIdx, Measure*);
      void readPageFormat(PageFormat* pf, QDomElement de, qreal conversion);
      QList<QDomElement> findSlurElements(QDomElement);
      void addGraceNoteAfter(Chord*, Segment*);
      void initPartState();
public:
      MusicXml(QDomDocument* d, MxmlReaderFirstPass const& p1);
      void import(Score*);
      };

//---------------------------------------------------------
//   XmlChordExtension
//---------------------------------------------------------

struct XmlChordExtension {
      int idx;
      const char* xmlName;
      };

enum {
      NoSystem          = 0,
      TopSystem         = 1,
      NewSystem         = 2,
      NewPage           = 3
      };

extern const XmlChordExtension chordExtensions[];


} // namespace Ms
#endif
