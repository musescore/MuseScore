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
#include "tiemap.h"
#include "slur.h"
#include "chordrest.h"

namespace Ms {

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

}

