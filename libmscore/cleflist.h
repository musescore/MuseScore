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

#ifndef __CLEFLIST_H__
#define __CLEFLIST_H__

#include "types.h"
#include "clef.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   ClefList
//    hide std::map interface
//---------------------------------------------------------

typedef std::map<TimePosition, ClefTypeList> ClefMap;

class ClefList : ClefMap {

   public:
      ClefList() {}
      ClefTypeList clef(const Fraction&) const;
      void setClef(const Fraction&, ClefTypeList);
      Fraction nextClefTick(const Fraction&) const;
      Fraction currentClefTick(const Fraction&) const;

      void erase(const Fraction& t)                    { ClefMap::erase(TimePosition(t));}
      void erase(const Fraction& b, const Fraction& e) { ClefMap::erase(lower_bound(b), lower_bound(e)); }
      void erase(iterator i)                           { ClefMap::erase(i); }
      void clear()                                     { ClefMap::clear(); }

      iterator lower_bound(const Fraction& t)          { return ClefMap::lower_bound(TimePosition(t)); }
      iterator begin()                                 { return ClefMap::begin(); }
      const_iterator begin() const                     { return ClefMap::begin(); }
      iterator end()                                   { return ClefMap::end(); }
      const_iterator end() const                       { return ClefMap::end(); }

      void insert(iterator a, iterator b)              { ClefMap::insert(a, b); }
      void insert(const Fraction& f, ClefTypeList c)   { ClefMap::insert(std::pair<TimePosition, ClefTypeList>(TimePosition(f), c)); }

      bool empty() const                               { return ClefMap::empty(); }
      };

}     // namespace Ms
#endif

