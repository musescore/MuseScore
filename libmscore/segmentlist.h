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

#ifndef __SEGMENTLIST_H__
#define __SEGMENTLIST_H__

#include "mscore.h"
class Segment;

//---------------------------------------------------------
//   SegmentList
//---------------------------------------------------------

class SegmentList {
      Segment* _first;        ///< First item of segment list
      Segment* _last;         ///< Last item of segment list
      int _size;              ///< Number of items in segment list

   public:
      SegmentList()                        { clear(); }
      void clear()                         { _first = _last = 0; _size = 0; }
#ifndef NDEBUG
      void check();
#else
      void check() {}
#endif
      SegmentList clone() const;
      int size() const                     { return _size;        }

      Segment* first() const               { return _first;       }
      Segment* first(SegmentTypes) const;

      Segment* last() const                { return _last;        }
      Segment* firstCRSegment() const;
      void remove(Segment*);
      void push_back(Segment*);
      void push_front(Segment*);
      void insert(Segment*);
      void insert(Segment* e, Segment* el);
      };

#endif

