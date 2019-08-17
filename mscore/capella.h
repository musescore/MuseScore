//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

enum class TIMESTEP : char { D1, D2, D4, D8, D16, D32, D64, D128, D256, D_BREVE };

#if 0
static const char* timeNames[] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64",
      "1/128", "1/256", "breve" };
#endif

class Capella;
enum class ClefType : signed char;

enum class CapellaNoteObjectType : char {
      REST, CHORD, CLEF, KEY, METER, EXPL_BARLINE, IMPL_BARLINE,
      PAGE_BKGR
      };

enum class BeamMode : unsigned char { AUTO, FORCE, SPLIT };

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

enum class Form : char {
      G, C, F, PERCUSSION,
      FORM_NULL, CLEF_UNCHANGED
      };

enum class ClefLine : char {
      L5, L4, L3, L2, L1
      };

enum class Oct : char {
      OCT_ALTA, OCT_NULL, OCT_BASSA
      };

//---------------------------------------------------------
//   CapClef
//---------------------------------------------------------

class CapClef : public NoteObj, public CapellaObj {
      Form form;

   public:
      CapClef(Capella* c) : NoteObj(CapellaNoteObjectType::CLEF), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      const char* name() {
            static const char* formName[] = { "G", "C", "F", "=", " ", "*" };
            return formName[int(form)];
            }
      ClefType clef() const;

      ClefLine line;
      Oct  oct;
      static ClefType clefType(Form, ClefLine, Oct);
      };

//---------------------------------------------------------
//   CapKey
//---------------------------------------------------------

class CapKey : public NoteObj, public CapellaObj {

   public:
      CapKey(Capella* c) : NoteObj(CapellaNoteObjectType::KEY), CapellaObj(c) {}
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

      CapMeter(Capella* c) : NoteObj(CapellaNoteObjectType::METER), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      };

//---------------------------------------------------------
//   CapExplicitBarline
//---------------------------------------------------------

class CapExplicitBarline : public NoteObj, public CapellaObj {
      BarLineType _type;
      int _barMode;      // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen

   public:
      CapExplicitBarline(Capella* c) : NoteObj(CapellaNoteObjectType::EXPL_BARLINE), CapellaObj(c) {}
      void read();
      void readCapx(XmlReader& e);
      BarLineType type() const { return _type; }
      int barMode() const      { return _barMode; }

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

      Form form;
      ClefLine line;
      Oct oct;                // clef

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
      BeamMode beamMode;
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

enum class CapellaType : unsigned char { GROUP, TRANSPOSABLE, METAFILE, SIMPLE_TEXT, TEXT, RECT_ELLIPSE,
      LINE, POLYGON, WAVY_LINE, SLUR, NOTE_LINES, WEDGE, VOLTA,
      BRACKET, GUITAR, TRILL
      };

class BasicDrawObj : public CapellaObj {
   public:
      unsigned char modeX, modeY, distY, flags;
      int nRefNote;
      int nNotes;
      bool background;
      int pageRange;
      CapellaType type;

      BasicDrawObj(CapellaType t, Capella* c)
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
      BasicRectObj(CapellaType t, Capella* c) : BasicDrawObj(t, c) {}
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
      GroupObj(Capella* c) : BasicDrawObj(CapellaType::GROUP, c) {}
      void read();

      QPointF relPos;
      QList<BasicDrawObj*> objects;
      };

//---------------------------------------------------------
//   TransposableObj
//---------------------------------------------------------

class TransposableObj : public BasicDrawObj {
   public:
      TransposableObj(Capella* c) : BasicDrawObj(CapellaType::TRANSPOSABLE, c) {}
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
      MetafileObj(Capella* c) : BasicRectObj(CapellaType::METAFILE, c) {}
      void read();
      };

//---------------------------------------------------------
//   LineObj
//---------------------------------------------------------

class LineObj : public BasicDrawObj {

   public:
      LineObj(Capella* c) : BasicDrawObj(CapellaType::LINE, c) {}
      LineObj(CapellaType t, Capella* c) : BasicDrawObj(t, c) {}
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
      RectEllipseObj(Capella* c) : LineObj(CapellaType::RECT_ELLIPSE, c) {}
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
      PolygonObj(Capella* c) : BasicDrawObj(CapellaType::POLYGON, c) {}
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
      WavyLineObj(Capella* c) : LineObj(CapellaType::WAVY_LINE, c) {}
      void read();

      unsigned waveLen;
      bool adapt;
      };

//---------------------------------------------------------
//   NotelinesObj
//---------------------------------------------------------

class NotelinesObj : public BasicDrawObj {
   public:
      NotelinesObj(Capella* c) : BasicDrawObj(CapellaType::NOTE_LINES, c) {}
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
         : BasicDrawObj(CapellaType::VOLTA, c), x0(0), x1(0), y(0),
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
      GuitarObj(Capella* c) : BasicDrawObj(CapellaType::GUITAR, c) {}
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
      TrillObj(Capella* c) : BasicDrawObj(CapellaType::TRILL, c), x0(0),
           x1(0), y(0), trillSign(true) {}
      void read();
      void readCapx(XmlReader& e);

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
         : BasicDrawObj(CapellaType::SLUR, c), color(Qt::black), nEnd(0), nMid(0), nDotDist(0), nDotWidth(0) {}
      void read();
      void readCapx(XmlReader& e);
      unsigned char nEnd, nMid, nDotDist, nDotWidth;
      };

//---------------------------------------------------------
//   TextObj
//---------------------------------------------------------

class TextObj : public BasicRectObj {

   public:
      TextObj(Capella* c) : BasicRectObj(CapellaType::TEXT, c) {}
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
         : BasicDrawObj(CapellaType::SIMPLE_TEXT, c), relPos(0, 0), align(0) {}
      void read();
      void readCapx(XmlReader& e);
      QString text() const { return _text; }
      QFont font() const { return _font; }
      QPointF pos() const { return relPos; }
      unsigned char textalign() const { return align; }
      };

//---------------------------------------------------------
//   BracketObj
//---------------------------------------------------------

class BracketObj : public LineObj {

   public:
      BracketObj(Capella* c) : LineObj(CapellaType::BRACKET, c) {}
      void read();

      char orientation, number;
      };

//---------------------------------------------------------
//   WedgeObj
//---------------------------------------------------------

class WedgeObj : public LineObj {

   public:
      WedgeObj(Capella* c) : LineObj(CapellaType::WEDGE, c), height(32),
           decresc(false) {}
      void read();
      void readCapx(XmlReader& e);

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
      Fraction ticks() const;
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
      signed char pitch;
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
      enum class StemDir : signed char { DOWN = -1, AUTO = 0, UP = 1, NONE = 3 };
      BeamMode beamMode;
      signed char notationStave;
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
      void readCapxArticulation(XmlReader& e);
      QList<Verse> verse;
      QList<CNote> notes;
      StemDir stemDir;
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
      enum class Error : char { CAP_NO_ERROR, BAD_SIG, CAP_EOF, BAD_VOICE_SIG,
            BAD_STAFF_SIG, BAD_SYSTEM_SIG
            };

      Capella();
      ~Capella();
      void read(QFile*);
      QString error(Error n) const { return QString(errmsg[int(n)]); }

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
      bool read(void* p, qint64 len);
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

