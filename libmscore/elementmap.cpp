//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "elementmap.h"
#include "tupletmap.h"
#include "slurmap.h"
#include "tiemap.h"
#include "spannermap.h"
#include "slur.h"
#include "chordrest.h"

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Tuplet* TupletMap::findNew(Tuplet* o)
      {
      foreach(const Tuplet2& t2, map) {
            if (t2.o == o)
                  return t2.n;
            }
      return 0;
      }

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Slur* SlurMap::findNew(Slur* o)
      {
      foreach(const Slur2& s2, map) {
            if (s2.o == o)
                  return s2.n;
            }
      return 0;
      }

//---------------------------------------------------------
//   check
//---------------------------------------------------------

void SlurMap::check()
      {
      foreach(const Slur2& s2, map) {
            Slur* slur = s2.n;
            if (slur->endElement() == 0) {
                  qDebug("slur end element missing %p new %p", s2.o, s2.n);
                  static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                  delete slur;
                  }
            }
      }

