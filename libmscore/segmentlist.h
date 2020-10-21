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

#ifndef __SEGMENTLIST_H__
#define __SEGMENTLIST_H__

#include "segment.h"

namespace Ms {

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
      Segment* first(SegmentType) const;
      Segment* first(ElementFlag) const;

      Segment* last() const                { return _last;        }
      Segment* last(ElementFlag) const;
      Segment* firstCRSegment() const;
      void remove(Segment*);
      void push_back(Segment*);
      void push_front(Segment*);
      void insert(Segment* e, Segment* el);  // insert e before el

      class iterator {
            Segment* p;
         public:
            iterator(Segment* s) { p = s; }
            iterator operator++() { iterator i(p); p = p->next(); return i; }
            bool operator !=(const iterator& i) const { return p != i.p; }
            Segment& operator*() { return *p; }
            };
      class const_iterator {
            const Segment* p;
         public:
            const_iterator(const Segment* s) { p = s; }
            const_iterator operator++() { const_iterator i(p); p = p->next(); return i; }
            bool operator !=(const const_iterator& i) const { return p != i.p; }
            const Segment& operator*() const { return *p; }
            };

      iterator begin()             { return _first; }
      iterator end()               { return 0; }
      const_iterator begin() const { return _first; }
      const_iterator end() const   { return 0; }
      };

// Segment* begin(SegmentList& l) { return l.first(); }
// Segment* end(SegmentList&) { return 0; }


}     // namespace Ms
#endif

