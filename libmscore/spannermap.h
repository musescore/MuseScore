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

#ifndef __SPANNERMAP_H__
#define __SPANNERMAP_H__


#include "thirdparty/intervaltree/IntervalTree.h"

namespace Ms {

class Spanner;

//---------------------------------------------------------
//   SpannerMap
//---------------------------------------------------------

class SpannerMap : std::multimap<int, Spanner*> {
      mutable bool dirty;
      mutable IntervalTree<Spanner*> tree;
      std::vector< ::Interval<Spanner*> > results;

      void update() const;

   public:
      SpannerMap();
      const std::vector< ::Interval<Spanner*> >& findContained(int start, int stop);
      const std::vector< ::Interval<Spanner*> >& findOverlapping(int start, int stop);
      const std::multimap<int, Spanner*>& map() const { return *this; }
      std::multimap<int,Spanner*>::const_reverse_iterator crbegin() const { return std::multimap<int, Spanner*>::crbegin(); }
      std::multimap<int,Spanner*>::const_reverse_iterator crend() const   { return std::multimap<int, Spanner*>::crend(); }
      std::multimap<int,Spanner*>::const_iterator cbegin() const { return std::multimap<int, Spanner*>::cbegin(); }
      std::multimap<int,Spanner*>::const_iterator cend() const  { return std::multimap<int, Spanner*>::cend(); }
      void addSpanner(Spanner* s);
      bool removeSpanner(Spanner* s);
      };

}     // namespace Ms

#endif

