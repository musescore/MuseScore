//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
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

class Omr;
class Score;
class Xml;
class Pattern;

#include "libmscore/durationtype.h"

//---------------------------------------------------------
//   HLine
//---------------------------------------------------------

struct HLine {
      int x1, x2, y;
      HLine() {}
      HLine(int a, int b, int c) : x1(a), x2(b), y(c) {}
      };

//---------------------------------------------------------
//   OmrNote
//---------------------------------------------------------

class OmrNote {
   public:
      int sym;
      int line;
      QRect r;
      double prob;      // probability
      };

class OmrPage;

//---------------------------------------------------------
//   OmrStaff
//---------------------------------------------------------

class OmrStaff : public QRect {
      QList<OmrNote*> _notes;

   public:
      OmrStaff() : QRect() {}
      OmrStaff(const QRect& r) : QRect(r) {}
      OmrStaff(int x, int y, int w, int h) : QRect(x, y, w, h) {}

      const QList<OmrNote*>& notes() const  { return _notes;   }
      QList<OmrNote*>& notes()              { return _notes;   }
      };

//---------------------------------------------------------
//   OmrSystem
//---------------------------------------------------------

class OmrSystem {
      OmrPage* _page;
      QList<OmrStaff> _staves;

      void searchNotes(QList<OmrNote*>*, Pattern*, int x1, int x2, int y, int line, int sym);

   public:
      OmrSystem(OmrPage* p) { _page = p;  }

      const QList<OmrStaff>& staves() const { return _staves; }
      QList<OmrStaff>& staves()             { return _staves; }
      int nstaves() const                   { return _staves.size(); }

      QList<QLine> barLines;

      void searchBarLines();
      void searchNotes(int sym);
      };

//---------------------------------------------------------
//   OmrPage
//---------------------------------------------------------

class OmrPage {
      Omr* _omr;
      QImage _image;
      double _spatium;

      int cropL, cropR;       // crop values in words (32 bit) units
      int cropT, cropB;       // crop values in pixel units

      QList<QRect> _slices;
      QList<OmrStaff> staves;
      QList<HLine> slines;

      QList<QLine>  lines;
      QList<OmrSystem> _systems;

      void crop();
      void slice();
      double skew(const QRect&);
      void deSkew();
      void getStaffLines();
      double xproject2(int y);
      int xproject(const uint* p, int wl);
      void radonTransform(ulong* projection, int w, int n, const QRect&);

   public:
      OmrPage(Omr* _parent);
      void setImage(const QImage& i)     { _image = i; }
      const QImage& image() const        { return _image; }
      QImage& image()                    { return _image; }
      void read(int);
      int width() const                  { return _image.width(); }
      int height() const                 { return _image.height(); }
      const uint* scanLine(int y) const  { return (const uint*)_image.scanLine(y); }
      const uint* bits() const           { return (const uint*)_image.bits(); }
      int wordsPerLine() const           { return (_image.bytesPerLine() + 3)/4; }

      const QList<QLine>& sl()           { return lines;    }
      const QList<HLine>& l()            { return slines;   }

      const QList<QRect>& slices() const { return _slices;  }
      double spatium() const             { return _spatium; }
      double staffDistance() const;
      double systemDistance() const;
      void readHeader(Score* score);

      const QList<OmrSystem>& systems() const { return _systems; }


      void write(Xml&) const;
      void read(QDomElement e);
      bool dot(int x, int y) const;
      };

#endif


