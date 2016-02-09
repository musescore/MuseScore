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

<<<<<<< HEAD
<<<<<<< HEAD
#include "libmscore/score.h"

namespace Ms {

enum class SymId;
=======
namespace Ms {

>>>>>>> 0e4c6b6... add pattern source files
=======
#include "libmscore/score.h"

namespace Ms {

enum class SymId;
>>>>>>> 27d1d6c... debug pattern match
class Sym;

//---------------------------------------------------------
//   Pattern
//    _n % sizeof(int)  is zero, patterns are 32bit padded
//---------------------------------------------------------

class Pattern {
   protected:
      QImage _image;
<<<<<<< HEAD
<<<<<<< HEAD
      SymId _id;
      QPoint _base;
    Score *_score;
    float **model;
    int rows;
    int cols;
=======
      Sym* _sym = 0;
      int _id;
      QPoint _base;
>>>>>>> 0e4c6b6... add pattern source files
=======
      SymId _id;
      QPoint _base;
    Score *_score;
<<<<<<< HEAD
>>>>>>> 27d1d6c... debug pattern match
=======
    float **model;
    int rows;
    int cols;
>>>>>>> 9d10dae... add note detector to suppress barline false positives: still under test

   public:
      Pattern();
      ~Pattern();
<<<<<<< HEAD
<<<<<<< HEAD
      Pattern(Score *s, SymId id, double spatium);
      Pattern(Score *s, QString name);
=======
      Pattern(int id, Sym* symbol, double spatium);
>>>>>>> 0e4c6b6... add pattern source files
=======
      Pattern(Score *s, SymId id, double spatium);
<<<<<<< HEAD
>>>>>>> 27d1d6c... debug pattern match
=======
      Pattern(Score *s, QString name);
>>>>>>> 9d10dae... add note detector to suppress barline false positives: still under test
      Pattern(QImage*, int, int, int, int);

      double match(const Pattern*) const;
      double match(const QImage* img, int col, int row) const;
<<<<<<< HEAD
<<<<<<< HEAD
      double match(const QImage* img, int col, int row, double bg_parm) const;

      void dump() const;
      const QImage* image() const { return &_image; }
    int w() const       { return cols; /*_image.width();*/ }
    int h() const       { return rows; /*_image.height();*/ }
      bool dot(int x, int y) const;
      SymId id() const      { return _id; }
      void setId(SymId val) { _id = val; }
=======
=======
      double match(const QImage* img, int col, int row, double bg_parm) const;
>>>>>>> 9d10dae... add note detector to suppress barline false positives: still under test

      void dump() const;
      const QImage* image() const { return &_image; }
    int w() const       { return cols; /*_image.width();*/ }
    int h() const       { return rows; /*_image.height();*/ }
      bool dot(int x, int y) const;
<<<<<<< HEAD
      int id() const      { return _id; }
      void setId(int val) { _id = val; }
>>>>>>> 0e4c6b6... add pattern source files
=======
      SymId id() const      { return _id; }
      void setId(SymId val) { _id = val; }
>>>>>>> 27d1d6c... debug pattern match
      const QPoint& base() const { return _base; }
      void setBase(const QPoint& v) { _base = v; }
      };
}

#endif

