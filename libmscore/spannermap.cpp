//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "spannermap.h"
#include "spanner.h"

namespace Ms {

//---------------------------------------------------------
//   SpannerMap
//---------------------------------------------------------

SpannerMap::SpannerMap()
      : std::multimap<int, Spanner*>()
      {
      dirty = true;
      }

//---------------------------------------------------------
//   update
//   updates the internal lookup tree, not the map itself
//---------------------------------------------------------

void SpannerMap::update() const
      {
      std::vector< ::Interval<Spanner*> > intervals;
      for (auto i : *this)
            intervals.push_back(Interval<Spanner*>(i.second->tick(), i.second->tick2(), i.second));
      tree = IntervalTree<Spanner*>(intervals);
      dirty = false;
      }

//---------------------------------------------------------
//   findContained
//---------------------------------------------------------

const std::vector<Interval<Spanner*>>& SpannerMap::findContained(int start, int stop)
      {
      if (dirty)
            update();
      results.clear();
      tree.findContained(start, stop, results);
      return results;
      }

//---------------------------------------------------------
//   findOverlapping
//---------------------------------------------------------

const std::vector<Interval<Spanner*>>& SpannerMap::findOverlapping(int start, int stop)
      {
      if (dirty)
            update();
      results.clear();
      tree.findOverlapping(start, stop, results);
      return results;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void SpannerMap::addSpanner(Spanner* s)
      {
#ifndef NDEBUG
      // check if spanner already in list
      for (auto i = begin(); i != end(); ++i) {
            if (i->second == s) {
                  qFatal("SpannerMap::addSpanner: %s already in list %p", s->name(), s);
                  }
            }
#endif
      insert(std::pair<int,Spanner*>(s->tick(), s));
      dirty = true;
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

bool SpannerMap::removeSpanner(Spanner* s)
      {
      for (auto i = begin(); i != end(); ++i) {
            if (i->second == s) {
                  erase(i);
                  dirty = true;
                  return true;
                  }
            }
      qFatal("Score::removeSpanner: %s (%p) not found", s->name(), s);
      return false;
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void SpannerMap::dump() const
      {
      qDebug("SpannerMap::dump");
      for (auto i = begin(); i != end(); ++i)
            qDebug("   %5d: %s %p", i->first, i->second->name(), i->second);
      }

#endif

}     // namespace Ms

