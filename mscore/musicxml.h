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

class Instrument;
class Measure;
class Tuplet;
class Tie;
class Slur;
class Part;
class Score;
class Note;
class Ottava;
class Trill;
class Pedal;
class Volta;
class TextLine;
class Chord;
class Harmony;
class Hairpin;
class Spanner;
class Lyrics;
class ChordRest;
class Beam;
class FiguredBass;

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

const int MAX_LYRICS       = 8;
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
      const Measure* _meas;

public:
      JumpMarkerDesc(Element* el, const Measure* meas) : _el(el), _meas(meas) {}
      Element* el() const { return _el; }
      const Measure* meas() const { return _meas; }
      };

typedef QList<JumpMarkerDesc> JumpMarkerDescList;

struct GraceNoteInfo {
      NoteType type;
      int pitch;
      int tpc;
      int len;
};

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

/**
 The MusicXML importer.
*/

class MusicXml {
      Score* score;
      QMap<int, VoiceDesc> voicelist;
      QVector<int> measureLength;               ///< Length of each measure in ticks
      QVector<int> measureStart;                ///< Start tick of each measure
      Fraction fractionTSig;                    ///< Current timesig as fraction

      Slur* slur[MAX_NUMBER_LEVEL];

      TextLine* bracket[MAX_BRACKETS];
      TextLine* dashes[MAX_DASHES];

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

      int maxLyrics;

      int divisions;                            ///< Current MusicXML divisions
      QVector<Tuplet*> tuplets;                 ///< Current tuplet for each track in the current part

      CreditWordsList credits;
      JumpMarkerDescList jumpsMarkers;

      std::vector<MusicXmlPartGroup*> partGroupList;
      QMap<Spanner*, QPair<int, int> > spanners;

      Ottava* ottava;                            ///< Current ottava
      Trill* trill;                              ///< Current trill
      Pedal* pedal;                              ///< Current pedal
      Harmony* harmony;                          ///< Current harmony
      Hairpin* hairpin;                          ///< Current hairpin (obsoletes wedgelist)
      Chord* tremStart;                          ///< Starting chord for current tremolo
      FiguredBass* figBass;                      ///< Current figured bass element (to attach to next note)
      bool figBassExtend;                        ///< Current figured bass extend
      BeamMode beamMode;                         ///< Current beam mode

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
      Measure* xmlMeasure(Part*, QDomElement, int, int measureLen);
      void xmlAttributes(Measure*, int stave, QDomElement node);
      void xmlLyric(int trk, QDomElement e,
                    QMap<int, Lyrics*>& numbrdLyrics,
                    QMap<int, Lyrics*>& defyLyrics,
                    QList<Lyrics*>& unNumbrdLyrics);
      void xmlNotations(Note* note, ChordRest* cr, int trk, int ticks, QDomElement node);
      void xmlNote(Measure*, int stave, const QString& partId, Beam*& beam, int& currentVoice, QDomElement node, QList<GraceNoteInfo>&);
      void xmlHarmony(QDomElement node, int tick, Measure* m, int staff);
      int xmlClef(QDomElement, int staffIdx, Measure*);
      void readPageFormat(PageFormat* pf, QDomElement de, qreal conversion);

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
