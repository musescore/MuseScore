//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: capella.h 3833 2011-01-04 13:55:40Z wschweer $
//
//  Copyright (C) 2009-2013 Werner Schweer and others
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

#ifndef __CAPELLA_H__
#define __CAPELLA_H__

#include "globals.h"
#include "libmscore/xml.h"

namespace Ms {

enum TIMESTEP { D1, D2, D4, D8, D16, D32, D64, D128, D256, D_BREVE };

#if 0
static const char* timeNames[] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64",
      "1/128", "1/256", "breve" };
#endif

class Capella;
enum class ClefType : signed char;

enum CapellaNoteObjectType {
      T_REST, T_CHORD, T_CLEF, T_KEY, T_METER, T_EXPL_BARLINE, T_IMPL_BARLINE,
      T_PAGE_BKGR
      };

enum BEAM_MODE { AUTO_BEAM, FORCE_BEAM, SPLIT_BEAM };

//---------------------------------------------------------
//   CapellaObj
//---------------------------------------------------------

class CapellaObj {
   protected:
      Capella* cap;

   public:
      CapellaObj(Capella* c) { cap = c; }
      };

//---------------------------------------------------------
//   NoteObj
//---------------------------------------------------------

class NoteObj {
      CapellaNoteObjectType _type;

   public:
      NoteObj(CapellaNoteObjectType t) { _type = t; }
      CapellaNoteObjectType type() const  { return _type; }
      };

enum FORM {
      FORM_G, FORM_C, FORM_F, FORM_PERCUSSION,
      FORM_NULL, CLEF_UNCHANGED
      };

enum CLEF_LINE {
      LINE_5, LINE_4, LINE_3, LINE_2, LINE_1
      };

enum OCT  {
      OCT_ALTA, OCT_NULL, OCT_BASSA
      };

//---------------------------------------------------------
//   CapClef
//---------------------------------------------------------

class CapClef : public NoteObj, public CapellaObj {
      FORM form;

   public:
      CapClef(Capella* c) : NoteObj(T_CLEF), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      const char* name() {
            static const char* formName[] = { "G", "C", "F", "=", " ", "*" };
            return formName[form];
            }
      ClefType clef() const;

      CLEF_LINE line;
      OCT  oct;
      static ClefType clefType(FORM, CLEF_LINE, OCT);
      };

//---------------------------------------------------------
//   CapKey
//---------------------------------------------------------

class CapKey : public NoteObj, public CapellaObj {

   public:
      CapKey(Capella* c) : NoteObj(T_KEY), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      int signature;    // -7 - +7
      };

//---------------------------------------------------------
//   CapMeter
//---------------------------------------------------------

class CapMeter : public NoteObj, public CapellaObj {
   public:
      unsigned char numerator;
      int log2Denom;
      bool allaBreve;

      CapMeter(Capella* c) : NoteObj(T_METER), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      };

//---------------------------------------------------------
//   CapExplicitBarline
//---------------------------------------------------------

class CapExplicitBarline : public NoteObj, public CapellaObj {
      int _type;
      int _barMode;      // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen

   public:
      CapExplicitBarline(Capella* c) : NoteObj(T_EXPL_BARLINE), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      int type() const    { return _type; }
      int barMode() const { return _barMode; }

      enum { BAR_SINGLE, BAR_DOUBLE, BAR_END,
             BAR_REPEND, BAR_REPSTART, BAR_REPENDSTART,
             BAR_DASHED};
      };

//---------------------------------------------------------
//   CapVoice
//---------------------------------------------------------

struct CapVoice {
      uchar y0Lyrics;
      uchar dyLyrics;
      QFont lyricsFont;
      unsigned char stemDir;
      QList<NoteObj*> objects;
      int voiceNo;
      };

//---------------------------------------------------------
//   CapStaff
//---------------------------------------------------------

struct CapStaff {
      uchar numerator;      // default time signature
      int log2Denom;
      bool allaBreve;

      uchar iLayout;
      int topDistX;
      int btmDistX;
      QColor color;
      QList<CapVoice*> voices;
      };

//---------------------------------------------------------
//   struct CapStaffLayout
//---------------------------------------------------------

struct CapStaffLayout {
      uchar barlineMode;
      uchar noteLines;
      bool bSmall;
      int topDist;
      int btmDist;
      int groupDist;
      uchar barlineFrom;
      uchar barlineTo;

      FORM form;
      CLEF_LINE line;
      OCT oct;                // clef

      // Schlagzeuginformation
      bool bPercussion;             // use drum channel
      bool bSoundMapIn;
      bool bSoundMapOut;
      char soundMapIn[128];         // Tabelle für MIDI-Töne iMin...iMin+n-1
      char soundMapOut[128];        // Tabelle für MIDI-Töne iMin...iMin+n-1

      int sound, volume, transp;

      QString descr;
      QString name;
      QString abbrev;
      QString intermediateName;
      QString intermediateAbbrev;
      };

//---------------------------------------------------------
//   CapSystem
//---------------------------------------------------------

struct CapSystem {
      int nAddBarCount;
      bool bBarCountReset;
      unsigned char explLeftIndent;      // < 0 --> Einrückung gemäß Stimmenbezeichnungen
                                         // >=  --> explizite Einrückung
      unsigned char beamMode;
      unsigned tempo;
      QColor color;                 // fuer Systemklammern
      bool bJustified;              // Randausgleich (Blocksatz)
      bool bPageBreak;              // nach dem System neue Seite anfangen
      int instrNotation;            // 0 = keine Instrumentenbezeichnung
                                    // 1 = abgekürzt, 2 = vollständig
      QList<CapStaff*> staves;
      };

//---------------------------------------------------------
//   BasicDrawObj
//---------------------------------------------------------

enum { CAP_GROUP, CAP_TRANSPOSABLE, CAP_METAFILE, CAP_SIMPLE_TEXT, CAP_TEXT, CAP_RECT_ELLIPSE,
      CAP_LINE, CAP_POLYGON, CAP_WAVY_LINE, CAP_SLUR, CAP_NOTE_LINES, CAP_WEDGE, CAP_VOLTA,
      CAP_BRACKET, CAP_GUITAR, CAP_TRILL
      };

class BasicDrawObj : public CapellaObj {
   public:
      unsigned char modeX, modeY, distY, flags;
      int nRefNote;
      int nNotes;
      bool background;
      int pageRange;
      int type;

      BasicDrawObj(int t, Capella* c)
         : CapellaObj(c), modeX(0), modeY(0), distY(0), flags(0),
           nRefNote(0), nNotes(0), background(0), pageRange(0), type(t) {}
      void read();
      void readCapx(XmlReader& e);
      };

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

class BasicRectObj : public BasicDrawObj {
   public:
      BasicRectObj(int t, Capella* c) : BasicDrawObj(t, c) {}
      void read();

      QPointF relPos;
      int width;
      int yxRatio;
      int height;
      };

//---------------------------------------------------------
//   GroupObj
//---------------------------------------------------------

class GroupObj : public BasicDrawObj {
   public:
      GroupObj(Capella* c) : BasicDrawObj(CAP_GROUP, c) {}
      void read();

      QPointF relPos;
      QList<BasicDrawObj*> objects;
      };

//---------------------------------------------------------
//   TransposableObj
//---------------------------------------------------------

class TransposableObj : public BasicDrawObj {
   public:
      TransposableObj(Capella* c) : BasicDrawObj(CAP_TRANSPOSABLE, c) {}
      void read();

      QPointF relPos;
      char b;
      QList<BasicDrawObj*> variants;
      };

//---------------------------------------------------------
//   MetafileObj
//---------------------------------------------------------

class MetafileObj : public BasicRectObj {
   public:
      MetafileObj(Capella* c) : BasicRectObj(CAP_METAFILE, c) {}
      void read();
      };

//---------------------------------------------------------
//   LineObj
//---------------------------------------------------------

class LineObj : public BasicDrawObj {

   public:
      LineObj(Capella* c) : BasicDrawObj(CAP_LINE, c) {}
      LineObj(int t, Capella* c) : BasicDrawObj(t, c) {}
      void read();

      QPointF pt1, pt2;
      QColor color;
      char lineWidth;
      };

//---------------------------------------------------------
//   RectEllipseObj
//---------------------------------------------------------

class RectEllipseObj : public LineObj {    // special
   public:
      RectEllipseObj(Capella* c) : LineObj(CAP_RECT_ELLIPSE, c) {}
      void read();

      int radius;
      bool bFilled;
      QColor clrFill;
      };

//---------------------------------------------------------
//   PolygonObj
//---------------------------------------------------------

class PolygonObj : public BasicDrawObj {
   public:
      PolygonObj(Capella* c) : BasicDrawObj(CAP_POLYGON, c) {}
      void read();

      bool bFilled;
      unsigned lineWidth;
      QColor clrFill;
      QColor clrLine;
      };

//---------------------------------------------------------
//   WavyLineObj
//---------------------------------------------------------

class WavyLineObj : public LineObj {
   public:
      WavyLineObj(Capella* c) : LineObj(CAP_WAVY_LINE, c) {}
      void read();

      unsigned waveLen;
      bool adapt;
      };

//---------------------------------------------------------
//   NotelinesObj
//---------------------------------------------------------

class NotelinesObj : public BasicDrawObj {
   public:
      NotelinesObj(Capella* c) : BasicDrawObj(CAP_NOTE_LINES, c) {}
      void read();

      int x0, x1, y;
      QColor color;
      };

//---------------------------------------------------------
//   VoltaObj
//---------------------------------------------------------

class VoltaObj : public BasicDrawObj {
   public:
      VoltaObj(Capella* c)
         : BasicDrawObj(CAP_VOLTA, c), x0(0), x1(0), y(0),
           bLeft(false), bRight(false), bDotted(false),
           allNumbers(false), from(0), to(0) {}
      void read();
      void readCapx(XmlReader& e);

      int x0, x1, y;
      QColor color;

      bool bLeft;
      bool bRight;
      bool bDotted;
      bool allNumbers;

      int from, to;
      };

//---------------------------------------------------------
//   GuitarObj
//---------------------------------------------------------

class GuitarObj : public BasicDrawObj {
   public:
      GuitarObj(Capella* c) : BasicDrawObj(CAP_GUITAR, c) {}
      void read();

      QPointF relPos;
      QColor color;
      short flags;
      int strings;      // 8 Saiten in 8 Halbbytes
      };

//---------------------------------------------------------
//   TrillObj
//---------------------------------------------------------

class TrillObj : public BasicDrawObj {
   public:
      TrillObj(Capella* c) : BasicDrawObj(CAP_TRILL, c) {}
      void read();

      int x0, x1, y;
      QColor color;
      bool trillSign;
      };

//---------------------------------------------------------
//   SlurObj
//---------------------------------------------------------

class SlurObj : public BasicDrawObj {
      QPointF bezierPoint[4]; // note default constructor inits to (0, 0)
      QColor color;           // note default constructor inits to invalid

   public:
      SlurObj(Capella* c)
         : BasicDrawObj(CAP_SLUR, c), color(Qt::black), nEnd(0), nMid(0), nDotDist(0), nDotWidth(0) {}
      void read();
      void readCapx(XmlReader& e);
      unsigned char nEnd, nMid, nDotDist, nDotWidth;
      };

//---------------------------------------------------------
//   TextObj
//---------------------------------------------------------

class TextObj : public BasicRectObj {

   public:
      TextObj(Capella* c) : BasicRectObj(CAP_TEXT, c) {}
      ~TextObj() {}
      void read();

      QString text;
      };

//---------------------------------------------------------
//   SimpleTextObj
//---------------------------------------------------------

class SimpleTextObj : public BasicDrawObj {
      QString _text;
      QPointF relPos;
      unsigned char align;
      QFont _font;

   public:
      SimpleTextObj(Capella* c)
         : BasicDrawObj(CAP_SIMPLE_TEXT, c), relPos(0, 0), align(0) {}
      void read();
      void readCapx(XmlReader& e);
      QString text() const { return _text; }
      QFont font() const { return _font; }
      QPointF pos() const { return relPos; }
      };

//---------------------------------------------------------
//   BracketObj
//---------------------------------------------------------

class BracketObj : public LineObj {

   public:
      BracketObj(Capella* c) : LineObj(CAP_BRACKET, c) {}
      void read();

      char orientation, number;
      };

//---------------------------------------------------------
//   WedgeObj
//---------------------------------------------------------

class WedgeObj : public LineObj {

   public:
      WedgeObj(Capella* c) : LineObj(CAP_WEDGE, c) {}
      void read();

      int height;
      bool decresc;
      };

//---------------------------------------------------------
//   BasicDurationalObj
//---------------------------------------------------------

class BasicDurationalObj : public CapellaObj {
   public:
      int nDots;
      bool noDuration;
      bool postGrace;
      bool bSmall;
      bool notBlack;
      QColor color;
      TIMESTEP t;
      int horizontalShift;
      int count;              // tuplet
      bool tripartite;
      bool isProlonging;

   public:
      BasicDurationalObj(Capella* c) : CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e, unsigned int& fullm);
      void readCapxDisplay(XmlReader& e);
      void readCapxObjectArray(XmlReader& e);
      int ticks() const;
      bool invisible;
      QList<BasicDrawObj*> objects;
      };

//---------------------------------------------------------
//   Verse
//---------------------------------------------------------

struct Verse {
      bool leftAlign;
      bool extender;
      bool hyphen;
      int num;
      QString verseNumber;
      QString text;
      };

struct CNote {
      char pitch;
      int explAlteration;     // 1 force, 2 suppress
      int headType;
      int alteration;
      int silent;
      };

//---------------------------------------------------------
//   ChordObj
//---------------------------------------------------------

class ChordObj : public BasicDurationalObj, public NoteObj {
   public:
      enum StemDir { DOWN = -1, AUTO = 0, UP = 1, NONE = 3 };
      unsigned char beamMode;
      char notationStave;
      char dStemLength;
      unsigned char nTremoloBars;
      unsigned articulation;
      bool leftTie;
      bool rightTie;
      char beamShift;
      char beamSlope;

   public:
      ChordObj(Capella*);
      void read();
      void readCapx(XmlReader& e);
      void readCapxLyrics(XmlReader& e);
      void readCapxNotes(XmlReader& e);
      void readCapxStem(XmlReader& e);
      QList<Verse> verse;
      QList<CNote> notes;
      char stemDir;           // -1 down, 0 auto, 1 up, 3 no stem
      };

//---------------------------------------------------------
//   RestObj
//---------------------------------------------------------

class RestObj : public BasicDurationalObj, public NoteObj {
      bool bVerticalCentered;
      int vertShift;

   public:
      RestObj(Capella*);
      void read();
      void readCapx(XmlReader& e);
      unsigned fullMeasures;  // >0, multi measure rest (counting measures)
      };

//---------------------------------------------------------
//   CapFont
//---------------------------------------------------------

struct CapFont {
      QString face;
      };

//---------------------------------------------------------
//   CapBracket
//---------------------------------------------------------

struct CapBracket {
      int from, to;
      bool curly;
      };

//---------------------------------------------------------
//   Capella
//---------------------------------------------------------

class Capella {
      static const char* errmsg[];
      int curPos;

      QFile* f;
      char* author;
      char* keywords;
      char* comment;

      unsigned char beamRelMin0;
      unsigned char beamRelMin1;
      unsigned char beamRelMax0;
      unsigned char beamRelMax1;
      unsigned nRel;                // presentation parameter
      unsigned nAbs;
      bool bUseRealSize;
      bool bAllowCompression;
      bool bPrintLandscape;

      bool bShowBarCount;           // Taktnumerierung zeigen
      unsigned char barNumberFrame; // 0=kein, 1=Rechteck, 2=Ellipse
      unsigned char nBarDistX;
      unsigned char nBarDistY;
      // LogFont       barNumFont;

      unsigned nFirstPage;          // Versatz fuer Seitenzaehlung

      unsigned leftPageMargins;     // Seitenraender
      unsigned topPageMargins;
      unsigned rightPageMargins;
      unsigned btmPageMargins;

      QList<QFont> fonts;
      QList<CapStaffLayout*> _staffLayouts;      // staff layout

      int interDist;
      unsigned char txtAlign;       // Stimmenbezeichnungen 0=links, 1=zentriert, 2=rechts
      unsigned char adjustVert;     // 0=nein, 1=außer letzte Seite, 3=alle Seiten
      bool redundantKeys;
      bool modernDoubleNote;
      bool bSystemSeparators;
      int nUnnamed;
      QFont namesFont;

      void readVoice(CapStaff*, int);
      void readStaff(CapSystem*);
      void readSystem();

   protected:
      void readStaveLayout(CapStaffLayout*, int);
      void readLayout();

   public:
      enum CapellaError { CAP_NO_ERROR, CAP_BAD_SIG, CAP_EOF, CAP_BAD_VOICE_SIG,
            CAP_BAD_STAFF_SIG, CAP_BAD_SYSTEM_SIG
            };

      Capella();
      ~Capella();
      void read(QFile*);
      QString error(CapellaError n) const { return QString(errmsg[int(n)]); }

      unsigned char readByte();
      char readChar();
      QColor readColor();
      int readInt();
      int readLong();
      short readWord();
      int readDWord();
      unsigned readUnsigned();
      char* readString();
      QString readQString();
      void readExtra();
      QList<BasicDrawObj*> readDrawObjectArray();
      void read(void* p, qint64 len);
      QFont readFont();
      QPointF readPoint();

      QList<CapSystem*> systems;
      QList<CapBracket> brackets;
      ChordObj* backgroundChord;
      CapStaffLayout* staffLayout(int idx)               { return _staffLayouts[idx]; }
      const QList<CapStaffLayout*>& staffLayouts() const { return _staffLayouts; }

      double smallLineDist;            // spatium unit in metric mm
      double normalLineDist;
      int topDist;
// capx support
   private:
      void readCapxVoice(XmlReader& e, CapStaff*, int);
      void readCapxStaff(XmlReader& e, CapSystem*, int);
      void readCapxSystem(XmlReader& e);
      void capxSystems(XmlReader& e);
      void readCapxStaveLayout(XmlReader& e, CapStaffLayout*, int);
      void capxLayoutStaves(XmlReader& e);
      void capxLayout(XmlReader& e);
      void initCapxLayout();
   public:
      void readCapx(XmlReader& e);
      QList<BasicDrawObj*> readCapxDrawObjectArray(XmlReader& e);
      };


} // namespace Ms
#endif

