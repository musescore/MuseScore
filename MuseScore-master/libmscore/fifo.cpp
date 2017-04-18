//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "fifo.h"

namespace Ms {

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FifoBase::clear()
      {
      ridx    = 0;
      widx    = 0;
      counter = 0;
      }

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void FifoBase::push()
      {
      widx = (widx + 1) % maxCount;
//      q_atomic_increment(&counter);
      ++counter;
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void FifoBase::pop()
      {
      ridx = (ridx + 1) % maxCount;
      // q_atomic_decrement(&counter);
      --counter;
      }

}

