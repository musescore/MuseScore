//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __IMPORTGTP_H__
#define __IMPORTGTP_H__

#include "libmscore/mscore.h"
#include "libmscore/fraction.h"
#include "libmscore/fret.h"
#include "libmscore/chordrest.h"
#include "libmscore/slur.h"
#include "libmscore/clef.h"
#include "libmscore/keysig.h"
#include "libmscore/chordrest.h"

namespace Ms {

class Score;
class Chord;
class Note;
class Segment;
class Measure;
class Tuplet;

static const int GP_MAX_LYRIC_LINES = 5;
static const int GP_MAX_TRACK_NUMBER = 32;
static const int GP_MAX_STRING_NUMBER = 7;
static const int GP_DEFAULT_PERCUSSION_CHANNEL = 9;

static const int GP_INVALID_KEYSIG = 127;

static const int GP_VOLTA_BINARY = 1;
static const int GP_VOLTA_FLAGS = 2;

enum class Repeat : char;

struct GpTrack {
      int patch;
      uchar volume, pan, chorus, reverb, phase, tremolo;
      };

struct GPVolta {
      int voltaType;
      QList<int> voltaInfo;
};

struct GpBar {
      Fraction timesig;
      int keysig;
      QString marker;
      BarLineType barLine;
      Repeat repeatFlags;
      int repeats;
      GPVolta volta;

      GpBar();
      };

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

class GuitarPro {
   protected:
      static const char* errmsg[];
      int version;
      int key;

      Score* score;
      QFile* f;
      int curPos;
      int previousTempo;
      int previousDynamic;
      int tempo;
      QMap<int,int> slides;

      int voltaSequence;
      QTextCodec* _codec;
      Slur** slurs;

      void skip(qint64 len);
      void read(void* p, qint64 len);
      int readUChar();
      int readChar();
      QString readPascalString(int);
      QString readWordPascalString();
      QString readBytePascalString();
      int readInt();
      QString readDelphiString();
      void readVolta(GPVolta*, Measure*);
      virtual void readBend(Note*);
      virtual void readMixChange(Measure* measure);
      virtual int readBeatEffects(int track, Segment*) = 0;
      void readLyrics();
      void readChannels();
      void setTuplet(Tuplet* tuplet, int tuple);
      Fraction len2fraction(int len);
      void addDynamic(Note*, int d);
      void setTempo(int n, Measure* measure);
      void createMeasures();
      void applyBeatEffects(Chord*, int beatEffects);
      void readTremoloBar(int track, Segment*);
      void readChord(Segment* seg, int track, int numStrings, QString name, bool gpHeader);
      void restsForEmptyBeats(Segment* seg, Measure* measure, ChordRest* cr, Fraction& l, int track, int tick);
      void createSlur(bool hasSlur, int staffIdx, ChordRest* cr);
      void createSlide(int slide, ChordRest* cr, int staffIdx);

   public:
      QString title, subtitle, artist, album, composer;
      QString transcriber, instructions;
      QStringList comments;
      GpTrack channelDefaults[GP_MAX_TRACK_NUMBER * 2];
      int staves;
      int measures;
      QList<GpBar> bars;

      enum class GuitarProError : char { GP_NO_ERROR, GP_UNKNOWN_FORMAT,
         GP_EOF, GP_BAD_NUMBER_OF_STRINGS
            };

      GuitarPro(Score*, int v);
      virtual ~GuitarPro();
      virtual void read(QFile*) = 0;
      QString error(GuitarProError n) const { return QString(errmsg[int(n)]); }
      };

//---------------------------------------------------------
//   GuitarPro1
//---------------------------------------------------------

class GuitarPro1 : public GuitarPro {

   protected:
      void readNote(int string, Note* note);
      virtual int readBeatEffects(int track, Segment*);

   public:
      GuitarPro1(Score* s, int v) : GuitarPro(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro2
//---------------------------------------------------------

class GuitarPro2 : public GuitarPro1 {

   public:
      GuitarPro2(Score* s, int v) : GuitarPro1(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro3
//---------------------------------------------------------

class GuitarPro3 : public GuitarPro1 {

      virtual int readBeatEffects(int track, Segment* segment);
   public:
      GuitarPro3(Score* s, int v) : GuitarPro1(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro4
//---------------------------------------------------------

class GuitarPro4 : public GuitarPro {

      int slide;
      void readInfo();
      bool readNote(int string, Note* note);
      virtual int readBeatEffects(int track, Segment* segment);
      virtual void readMixChange(Measure* measure);
      int convertGP4SlideNum(int slide);

   public:
      GuitarPro4(Score* s, int v) : GuitarPro(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro5
//---------------------------------------------------------

class GuitarPro5 : public GuitarPro {

      int slide;
      void readInfo();
      void readPageSetup();
      virtual int readBeatEffects(int track, Segment* segment);
      bool readNote(int string, Note* note);
      virtual void readMixChange(Measure* measure);
      void readMeasure(Measure* measure, int staffIdx, Tuplet*[]);
      void readArtificialHarmonic();
      void readTracks();
      void readMeasures();
      int readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets);
      bool readNoteEffects(Note*);

   public:
      GuitarPro5(Score* s, int v) : GuitarPro(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro6
//---------------------------------------------------------

class GuitarPro6 : public GuitarPro {

   private:
      // an integer stored in the header indicating that the file is not compressed (BCFS).
      const int GPX_HEADER_UNCOMPRESSED = 1397113666;
      // an integer stored in the header indicating that the file is not compressed (BCFZ).
      const int GPX_HEADER_COMPRESSED = 1514554178;
      int position=0;
      QMap<int, int>* slides;
      QByteArray* buffer;
      // a constant storing the amount of bits per byte
      const int BITS_IN_BYTE = 8;
      // contains all the information about notes that will go in the parts
      struct GPPartInfo {
            QDomNode masterBars;
            QDomNode bars;
            QDomNode voices;
            QDomNode beats;
            QDomNode notes;
            QDomNode rhythms;
            };
      // a mapping from identifiers to fret diagrams
      QMap<int, FretDiagram*> fretDiagrams;
      void parseFile(char* filename, QByteArray* data);
      int readBit();
      QByteArray getBytes(QByteArray* buffer, int offset, int length);
      void readGPX(QByteArray* buffer);
      int readInteger(QByteArray* buffer, int offset);
      QByteArray readString(QByteArray* buffer, int offset, int length);
      int readBits(int bitsToRead);
      int readBitsReversed(int bitsToRead);
      void readGpif(QByteArray* data);
      void readScore(QDomNode* metadata);
      void readChord(QDomNode* diagram, int track);
      int findNumMeasures(GPPartInfo* partInfo);
      void readMasterTracks(QDomNode* masterTrack);
      void readDrumNote(Note* note, int element, int variation);
      int readBeats(QString beats, GPPartInfo* partInfo, Measure* measure, int tick, int staffIdx, int voiceNum, Tuplet* tuplets[]);
      void readBars(QDomNode* barList, Measure* measure, ClefType oldClefId[], GPPartInfo* partInfo, KeySig* t);
      void readTracks(QDomNode* tracks);
      void readMasterBars(GPPartInfo* partInfo);
      Fraction rhythmToDuration(QString value);
      QDomNode getNode(QString id, QDomNode nodes);
      void unhandledNode(QString nodeName);
      void makeTie(Note* note);

   protected:
      void readNote(int string, Note* note);
      virtual int readBeatEffects(int track, Segment*);

   public:
      GuitarPro6(Score* s) : GuitarPro(s, 6) {}
      virtual void read(QFile*);
      };

} // namespace Ms
#endif
