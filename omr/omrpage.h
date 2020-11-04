//=============================================================================
//  MusE Reader
//  Music Score Reader
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __OMRPAGE_H__
#define __OMRPAGE_H__

#include "libmscore/mscore.h"
#include "libmscore/durationtype.h"
#include "libmscore/fraction.h"
#include "libmscore/clef.h"
#include "libmscore/xml.h"
#include "libmscore/sym.h"

namespace Ms {

class Omr;
class Score;
class XmlWriter;
class XmlReader;
class Pattern;
class OmrPage;


//---------------------------------------------------------
//   HLine
//---------------------------------------------------------

struct HLine {
      int x1, x2, y;
      HLine() {}
      HLine(int a, int b, int c) : x1(a), x2(b), y(c) {}
      };

//---------------------------------------------------------
//   OmrPattern
//---------------------------------------------------------

class OmrPattern : public QRect {
   public:
    OmrPattern() : QRect(), sym(SymId::noSym), prob(0.0) {}
      SymId sym;
      double prob;
      };

//---------------------------------------------------------
//   OmrClef
//---------------------------------------------------------

class OmrClef : public OmrPattern {
   public:
      OmrClef() : OmrPattern() {}
      OmrClef(const OmrPattern& p) : OmrPattern(p) {}
      ClefType type = ClefType::G;//CLEF_G;
      };

//---------------------------------------------------------
//   OmrNote
//    object on staff line
//---------------------------------------------------------

class OmrNote : public OmrPattern {
   public:
      int line;
      };

//---------------------------------------------------------
//   OmrChord
//---------------------------------------------------------

class OmrChord {
   public:
      TDuration duration;
      QList<OmrNote*> notes;
      };

//---------------------------------------------------------
//   OmrTimesig
//---------------------------------------------------------

class OmrTimesig : public QRect {
   public:
      OmrTimesig() {}
      OmrTimesig(const QRect& r) : QRect(r) {}
      Fraction timesig;
      };

//---------------------------------------------------------
//   OmrKeySig
//---------------------------------------------------------

class OmrKeySig : public QRect {
   public:
      OmrKeySig() {}
      OmrKeySig(const QRect& r) : QRect(r) {}
      int type = 0;          // -7 -> +7
      };

//---------------------------------------------------------
//   OmrMeasure
//---------------------------------------------------------

class OmrMeasure {
      int _x1, _x2;
      QList<QList<OmrChord>> _chords;      // list of notes for every staff
      OmrTimesig* _timesig = 0;

   public:
      OmrMeasure() {}
      OmrMeasure(int x1, int x2) : _x1(x1), _x2(x2) {}
      QList<QList<OmrChord>>& chords()             { return _chords; }
      const QList<QList<OmrChord>>& chords() const { return _chords; }
      int x1() const { return _x1; }
      int x2() const { return _x2; }
      OmrTimesig* timesig() const     { return _timesig; }
      void setTimesig(OmrTimesig* ts) { _timesig = ts;}
      };

//---------------------------------------------------------
//   OmrStaff
//    rectangle of staff lines
//---------------------------------------------------------

class OmrStaff : public QRect {
      QList<OmrNote*> _notes;
      OmrClef _clef;
      OmrKeySig _keySig;

   public:
      OmrStaff() : QRect() {}
      OmrStaff(const QRect& r) : QRect(r) {}
      OmrStaff(int x, int y, int w, int h) : QRect(x, y, w, h) {}
      QList<OmrNote*>& notes()             { return _notes; }
      const QList<OmrNote*>& notes() const { return _notes; }
      OmrClef clef() const                 { return _clef; }
      void setClef(const OmrClef& c)       { _clef = c; }
      OmrKeySig keySig() const             { return _keySig; }
      void setKeySig(const OmrKeySig& s)   { _keySig = s; }
      };

//---------------------------------------------------------
//   OmrSystem
//---------------------------------------------------------

class OmrSystem {
      OmrPage* _page;
      QList<OmrStaff>  _staves;
      QList<OmrMeasure>_measures;

      void searchNotes(QList<OmrNote*>*, int x1, int x2, int y, int line);

   public:
      OmrSystem(OmrPage* p) { _page = p;  }

      const QList<OmrStaff>& staves() const { return _staves; }
      QList<OmrStaff>& staves()             { return _staves; }
      QList<OmrMeasure>& measures()         { return _measures; }
      const QList<OmrMeasure>& measures() const { return _measures; }

      QList<QLine> barLines;

      void searchSysBarLines();
      float searchBarLinesvar(int n_staff, int **note_labels);
      void searchNotes();
      void searchNotes(int *note_labels, int ran);
      };

//---------------------------------------------------------
//   OmrPage
//---------------------------------------------------------

class OmrPage {
      Omr* _omr;
      QImage _image;
      double _spatium;
      double _ratio;

      int cropL, cropR;       // crop values in words (32 bit) units
      int cropT, cropB;       // crop values in pixel units

      QList<QRect> _slices;
      QList<OmrStaff> staves;
      QList<HLine> slines;

      QList<QLine>  lines;
      QList<OmrSystem> _systems;

      void removeBorder();
      void crop();
      void slice();
      double skew(const QRect&);
      void deSkew();
      void getStaffLines();
      void getRatio();
      double xproject2(int y);
      int xproject(const uint* p, int wl);
      void radonTransform(ulong* projection, int w, int n, const QRect&);
      OmrTimesig* searchTimeSig(OmrSystem* system);
      OmrClef searchClef(OmrSystem* system, OmrStaff* staff);
      void searchKeySig(OmrSystem* system, OmrStaff* staff);
      OmrPattern searchPattern(const std::vector<Pattern*>& pl, int y, int x1, int x2);

   public:
      OmrPage(Omr* _parent);
      void setImage(const QImage& i)     { _image = i; }
      const QImage& image() const        { return _image; }
      QImage& image()                    { return _image; }
      void read();
      int width() const                  { return _image.width(); }
      int height() const                 { return _image.height(); }
      const uint* scanLine(int y) const  { return (const uint*)_image.scanLine(y); }
      const uint* bits() const           { return (const uint*)_image.bits(); }
      int wordsPerLine() const           { return (_image.bytesPerLine() + 3)/4; }

      const QList<QLine>& sl()           { return lines;    }
      const QList<HLine>& l()            { return slines;   }

      const QList<QRect>& slices() const { return _slices;  }
      double spatium() const             { return _spatium; }
      double ratio() const   {return _ratio;}
      double staffDistance() const;
      double systemDistance() const;
      void readHeader(Score* score);
      void readBarLines();
      float searchBarLines(int start_staff, int end_staff);
      void identifySystems();

      const QList<OmrSystem>& systems() const { return _systems; }
      //QList<OmrSystem>& systems() { return _systems; }
      OmrSystem* system(int idx)  { return &_systems[idx]; }


      void write(XmlWriter&) const;
      void read(XmlReader&);
      bool dot(int x, int y) const;
      bool isBlack(int x, int y) const;
      };

}

#endif


