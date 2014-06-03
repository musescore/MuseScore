//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "keylist.h"
#include "xml.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

int KeyList::key(int tick) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(tick);
      if (i == begin())
            return 0;
      return (--i)->second;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeyList::setKey(int tick, int k)
      {
      if (key(tick) == k)
            return;
      if (tick > 0 && key(tick-1) == k)
            erase(tick);
      else  {
            auto i = find(tick);
            if (i == end())
                  insert(std::pair<int, int>(tick, k));
            else
                  i->second = k;
            }

      }

//---------------------------------------------------------
//   nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

int KeyList::nextKeyTick(int tick) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(tick+1);
      if (i == end())
            return 0;
      return i->first;
      }

//---------------------------------------------------------
//   prevKey
//
//    returns the key before the current key for tick
//---------------------------------------------------------

int KeyList::prevKey(int tick) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(tick);
      if (i == begin())
            return 0;
      --i;
      if (i == begin())
            return 0;
      return (--i)->second;
      }

//---------------------------------------------------------
//   currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//---------------------------------------------------------

int KeyList::currentKeyTick(int tick) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(tick);
      if (i == begin())
            return 0;
      --i;
      return i->first;
      }

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(XmlReader& e, Score* cs)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "key") {
                  int k;
                  int tick = e.intAttribute("tick", 0);
                  if (e.hasAttribute("custom"))
                        k = 0;      // ke.setCustomType(e.intAttribute("custom"));
                  else
                        k = e.intAttribute("idx");
                  (*this)[cs->fileDivision(tick)] = k;
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

}

