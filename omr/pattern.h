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

#ifndef __PATTERN_H__
#define __PATTERN_H__

namespace Ms {

class Sym;

//---------------------------------------------------------
//   Pattern
//    _n % sizeof(int)  is zero, patterns are 32bit padded
//---------------------------------------------------------

class Pattern {
   protected:
      QImage _image;

   public:
      Pattern();
      ~Pattern();
      Pattern(Sym* symbol, double spatium);
      Pattern(QImage*, int, int, int, int);

      double match(const Pattern*) const;
      void dump() const;
      const QImage* image() const { return &_image; }
      int w() const { return _image.width(); }
      int h() const { return _image.height(); }
      bool dot(int x, int y) const;
      };

}

#endif

