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
#include "libmscore/clef.h"
#include "libmscore/keysig.h"
#include "libmscore/hairpin.h"
#include "libmscore/ottava.h"
#include "libmscore/drumset.h"

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

/* How the fermatas are represented in Guitar Pro is two integers, the
 * first is an index value and the second is the time division that
 * index value refers to, and they are givin with respect to a
 * measure. Time division 0 means a minim, 1 is a crotchet, 2 is a
 * quaver and so on, with the index (counting from 0) refering to how
 * many time divisions occur before the fermata. These numbers are
 * separated in GP6 with a '/' character. For example, a note
 * occurring on the third beat of a measure in a 4/4 bar would be
 * represented as 2/1.
 */
struct GPFermata {
      int index;
      int timeDivision;
};

struct GPLyrics {
      QStringList lyrics;
      int fromBeat;
      int beatCounter;
      int lyricTrack;
};

struct GpBar {
      Fraction timesig;
      bool freeTime;
      int keysig;
      QString marker;
      BarLineType barLine;
      Repeat repeatFlags;
      int repeats;
      GPVolta volta;
      QString direction;
      QString directionStyle;

      GpBar();
      };

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

class GuitarPro {
   protected:

      // note effect bit masks
      static const uchar EFFECT_BEND = 0x1;
      static const uchar EFFECT_STACATTO = 0x1;
      static const uchar EFFECT_HAMMER = 0x2;
      static const uchar EFFECT_PALM_MUTE = 0x2;
      static const uchar EFFECT_TREMOLO = 0x4;
      static const uchar EFFECT_LET_RING = 0x8;
      static const uchar EFFECT_SLIDE_OLD = 0x4;
      static const uchar EFFECT_SLIDE = 0x8;
      static const uchar EFFECT_GRACE = 0x10;
      static const uchar EFFECT_ARTIFICIAL_HARMONIC = 0x10;
      static const uchar EFFECT_TRILL = 0x20;
      static const uchar EFFECT_GHOST = 0x01;

      // arpeggio direction masks
      static const uchar ARPEGGIO_UP = 0xa;
      static const uchar ARPEGGIO_DOWN = 0x2;

      // note bit masks
      static const uchar NOTE_GHOST = 0x04;
      static const uchar NOTE_DEAD = 0x20;
      static const uchar NOTE_DYNAMIC = 0x10;
      static const uchar NOTE_FRET = 0x20;
      static const uchar NOTE_FINGERING = 0x80;
      static const uchar NOTE_MARCATO = 0x02;
      static const uchar NOTE_SFORZATO = 0x40;
      static const uchar NOTE_SLUR = 0x8;
      static const uchar NOTE_APPOGIATURA = 0x02;

      // beat bit masks
      static const uchar BEAT_FADE = 0x10;
      static const uchar BEAT_EFFECT = 0x20;
      static const uchar BEAT_TREMOLO = 0x04;
      static const uchar BEAT_ARPEGGIO = 0x40;
      static const uchar BEAT_STROKE_DIR = 0x02;
      static const uchar BEAT_DOTTED = 0x01;
      static const uchar BEAT_PAUSE = 0x40;
      static const uchar BEAT_TUPLET = 0x20;
      static const uchar BEAT_LYRICS = 0x4;
      static const uchar BEAT_EFFECTS = 0x8;
      static const uchar BEAT_MIX_CHANGE = 0x10;
      static const uchar BEAT_CHORD = 0x2;

      // score bit masks
      static const uchar SCORE_TIMESIG_NUMERATOR = 0x1;
      static const uchar SCORE_TIMESIG_DENOMINATOR = 0x2;
      static const uchar SCORE_REPEAT_START = 0x4;
      static const uchar SCORE_REPEAT_END = 0x8;
      static const uchar SCORE_MARKER = 0x20;
      static const uchar SCORE_VOLTA = 0x10;
      static const uchar SCORE_KEYSIG = 0x40;
      static const uchar SCORE_DOUBLE_BAR = 0x80;

      // slide kinds
      static const int SHIFT_SLIDE = 1;
      static const int LEGATO_SLIDE = 2;
      static const int SLIDE_OUT_DOWN = 4;
      static const int SLIDE_OUT_UP = 8;
      static const int SLIDE_IN_ABOVE = 16;
      static const int SLIDE_IN_BELOW = 32;

      static const int MAX_PITCH = 127;
      static const char* errmsg[];
      int version;
      int key;

      QMap<int, QList<GPFermata>*> fermatas;
      Ottava** ottava;
      Hairpin** hairpins;
      Score* score;
      QFile* f;
      int curPos;
      int previousTempo;
      int previousDynamic;
      int ottavaFound;
      QString ottavaValue;
      int tempo;
      QMap<int,int> slides;

      GPLyrics gpLyrics;
      int slide;
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
      virtual bool readMixChange(Measure* measure);
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
      void createOttava(bool hasOttava, int track, ChordRest* cr, QString value);
      void createSlide(int slide, ChordRest* cr, int staffIdx);
      void createCrecDim(int staffIdx, int track, int tick, bool crec);
      void addTextToNote(QString string, TextStyle textStyle, Note* note);
      void addPalmMute(Note* note);
      void addLetRing(Note* note);
      void addTap(Note* note);
      void addSlap(Note* note);
      void addPop(Note* note);

   public:
      void initGuitarProDrumset();
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

      void readInfo();
      bool readNote(int string, int staffIdx, Note* note);
      virtual int readBeatEffects(int track, Segment* segment);
      virtual bool readMixChange(Measure* measure);
      int convertGP4SlideNum(int slide);

   public:
      GuitarPro4(Score* s, int v) : GuitarPro(s, v) {}
      virtual void read(QFile*);
      };

//---------------------------------------------------------
//   GuitarPro5
//---------------------------------------------------------

class GuitarPro5 : public GuitarPro {

      void readInfo();
      void readPageSetup();
      virtual int readBeatEffects(int track, Segment* segment);
      bool readNote(int string, Note* note);
      virtual bool readMixChange(Measure* measure);
      void readMeasure(Measure* measure, int staffIdx, Tuplet*[], bool mixChange);
      void readArtificialHarmonic();
      void readTracks();
      void readMeasures(int startingTempo);
      int readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets, bool mixChange);
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
      int readBeats(QString beats, GPPartInfo* partInfo, Measure* measure, int startTick, int staffIdx, int voiceNum, Tuplet* tuplets[], int measureCounter);
      void readBars(QDomNode* barList, Measure* measure, ClefType oldClefId[], GPPartInfo* partInfo, int measureCounter);
      void readTracks(QDomNode* tracks);
      void readMasterBars(GPPartInfo* partInfo);
      Fraction rhythmToDuration(QString value);
      Fraction fermataToFraction(int numerator, int denominator);
      QDomNode getNode(QString id, QDomNode nodes);
      void unhandledNode(QString nodeName);
      void makeTie(Note* note);
      int* previousDynamic;
      void addTremoloBar(Segment* segment, int track, int whammyOrigin, int whammyMiddle, int whammyEnd);

   protected:
      void readNote(int string, Note* note);
      virtual int readBeatEffects(int track, Segment*);

   public:
      GuitarPro6(Score* s) : GuitarPro(s, 6) {}
      virtual void read(QFile*);
      };

} // namespace Ms
#endif
