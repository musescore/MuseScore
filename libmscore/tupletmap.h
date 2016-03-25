//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TUPLETMAP_H__
#define __TUPLETMAP_H__

namespace Ms {

class Tuplet;

//---------------------------------------------------------
//   Tuplet2
//---------------------------------------------------------

struct Tuplet2 {
      Tuplet* o;
      Tuplet* n;
      Tuplet2(Tuplet* _o, Tuplet* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   TupletMap
//---------------------------------------------------------

class TupletMap {
      QList<Tuplet2> map;

   public:
      TupletMap() {}
      Tuplet* findNew(Tuplet* o);
      void add(Tuplet* _o, Tuplet* _n) { map.append(Tuplet2(_o, _n)); }
      };


}     // namespace Ms
#endif

