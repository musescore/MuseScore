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

#include "libmscore/mscore.h"
#include "musicxmlsupport.h"

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
      CreditWords(double a, double b, QString c, QString d, QString e, QString f) {
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
//   MusicXmlCreator
//---------------------------------------------------------

/**
 The MusicXML "creator" meta-data element.
*/

class MusicXmlCreator {
      QString _type;
      QString _text;

   public:
      MusicXmlCreator(QString& tp, QString& txt) { _type = tp; _text = txt; }
      QString crType() const                     { return _type; }
      QString crText() const                     { return _text; }
      };

//---------------------------------------------------------
//   VoiceDesc
//---------------------------------------------------------

/**
 The description of a single voice in a MusicXML part.
*/

class VoiceDesc {
   public:
      VoiceDesc();
      void incrChordRests(int s);
      int numberChordRests() const;
      int numberChordRests(int s) const { return (s >= 0 && s < MAX_STAVES) ? _chordRests[s] : 0; }
      int preferredStaff() const;       ///< Determine preferred staff for this voice
      void setStaff(int s)              { if (s >= 0) _staff = s; }
      int staff() const                 { return _staff; }
      void setVoice(int v)              { if (v >= 0) _voice = v; }
      int voice() const                 { return _voice; }
      void setVoice(int s, int v)       { if (s >= 0 && s < MAX_STAVES) _voices[s] = v; }
      int voice(int s) const            { return (s >= 0 && s < MAX_STAVES) ? _voices[s] : -1; }
      void setOverlap(bool b)           { _overlaps = b; }
      bool overlaps() const             { return _overlaps; }
      void setStaffAlloc(int s, int i)  { if (s >= 0 && s < MAX_STAVES) _staffAlloc[s] = i; }
      int staffAlloc(int s) const       { return (s >= 0 && s < MAX_STAVES) ? _staffAlloc[s] : -1; }
      QString toString() const;
   private:
      int _chordRests[MAX_STAVES];      ///< The number of chordrests on each MusicXML staff
      int _staff;                       ///< The MuseScore staff allocated
      int _voice;                       ///< The MuseScore voice allocated
      bool _overlaps;                   ///< This voice contains active notes in multiple staves at the same time
      int _staffAlloc[MAX_STAVES];      ///< For overlapping voices: voice is allocated on these staves (note: -2=unalloc -1=undef 1=alloc)
      int _voices[MAX_STAVES];          ///< For every voice allocated on the staff, the voice number
      };

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
      Element* el() const         { return _el; }
      const Measure* meas() const { return _meas; }
      };

typedef QList<JumpMarkerDesc> JumpMarkerDescList;

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

      Slur* slur[MAX_NUMBER_LEVEL];

      TextLine* bracket[MAX_BRACKETS];
      TextLine* dashes[MAX_DASHES];

      Tie* tie;
      int voice;
      int move;
      Volta* lastVolta;

      QDomDocument* doc;
      int tick;                                 ///< Current position in MusicXML time
      int maxtick;                              ///< Maxtick of a measure, used to calculate measure len
      int prevtick;                             ///< Previous notes tick (used to insert Jumps)
      int lastMeasureLen;
      int multiMeasureRestCount;                ///< Remaining measures in a multi measure rest
      bool startMultiMeasureRest;               ///< Multi measure rest started in this measure

      int lastLen;                              ///< Needed for chords
      int maxLyrics;

      int divisions;
      QVector<Tuplet*> tuplets;                 ///< Current tuplet for each track in the current part

      QString composer;
      QString poet;
      QString translator;
      CreditWordsList credits;
      JumpMarkerDescList jumpsMarkers;

//      std::vector<MusicXmlWedge> wedgeList;
      std::vector<MusicXmlPartGroup*> partGroupList;
      QMap<Spanner*, QPair<int, int> > spanners;

      Ottava* ottava;                            ///< Current ottava
      Trill* trill;                              ///< Current trill
      Pedal* pedal;                              ///< Current pedal
      Harmony* harmony;                          ///< Current harmony
      Hairpin* hairpin;                          ///< Current hairpin (obsoletes wedgelist)
      Chord* tremStart;                          ///< Starting chord for current tremolo
      BeamMode beamMode;                         ///< Current beam mode

      int pageWidth;                             ///< Page width read from defaults
      int pageHeight;                            ///< Page height read from defaults

      QMap<QString, MusicXMLDrumset> drumsets;   ///< Drumset for each part

      //-----------------------------

//      void addWedge(int no, int startPos, qreal rx, qreal ry, bool above, bool hasYoffset, qreal yoffset, int subType);
//      void genWedge(int no, int endPos, Measure*, int staff);
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
      void xmlNote(Measure*, int stave, const QString& partId, QDomElement node);
      void xmlHarmony(QDomElement node, int tick, Measure* m, int staff);
      int xmlClef(QDomElement, int staffIdx, Measure*);
      void initVoiceMapperAndMapVoices(QDomElement e);

   public:
      MusicXml(QDomDocument* d);
      void import(Score*);
      };

//---------------------------------------------------------
//   XmlChorExtension
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

#endif
