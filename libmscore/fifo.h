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

#ifndef __FIFO_H__
#define __FIFO_H__

#include <atomic>

namespace Ms {

//---------------------------------------------------------
//   FifoBase
//    - works only for one reader/writer
//    - reader writes ridx
//    - writer writes widx
//    - reader decrements counter
//    - writer increments counter
//    - counter increment/decrement must be atomic
//---------------------------------------------------------

class FifoBase {

   protected:
      int ridx;                 // read index
      int widx;                 // write index
      std::atomic<int> counter; // objects in fifo
      int maxCount;

      void push();
      void pop();

   public:
      FifoBase()              { clear(); }
      virtual ~FifoBase()     {}
      void clear();
      int count() const       { return counter; }
      bool empty() const    { return counter == 0; }
      bool isFull() const     { return maxCount == counter; }
      };


}     // namespace Ms
#endif

