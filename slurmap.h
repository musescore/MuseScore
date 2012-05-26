//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SLURMAP_H__
#define __SLURMAP_H__

class Slur;

//---------------------------------------------------------
//   Slur2
//---------------------------------------------------------

struct Slur2 {
      Slur* o;
      Slur* n;
      Slur2(Slur* _o, Slur* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   SlurMap
//---------------------------------------------------------

class SlurMap {
      QList<Slur2> map;

   public:
      SlurMap() {}
      Slur* findNew(Slur* o);
      void add(Slur* _o, Slur* _n) { map.append(Slur2(_o, _n)); }
      void check();
      };

#endif

