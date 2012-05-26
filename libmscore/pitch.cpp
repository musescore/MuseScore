//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "pitch.h"

//---------------------------------------------------------
//   PitchList
//    return pitch offset at tick position (ottava)
//---------------------------------------------------------

int PitchList::pitchOffset(int tick) const
      {
      if (empty())
            return 0;
      PitchList::const_iterator i = upperBound(tick);
      if (i == constBegin())
            return 0;
      --i;
      return i.value();
      }
