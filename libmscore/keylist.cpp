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

KeySigEvent KeyList::key(const Fraction& tick) const
      {
      KeySigEvent ke;
      ke.setKey(Key::C);

      if (empty())
            return ke;
      auto i = upper_bound(TimePosition(tick));
      if (i == begin())
            return ke;
      return (--i)->second;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void KeyList::setKey(const Fraction& tick, KeySigEvent k)
      {
      auto i = find(TimePosition(tick));
      if (i == end())
            insert(std::pair<TimePosition, KeySigEvent>(TimePosition(tick), k));
      else
            i->second = k;
      }

//---------------------------------------------------------
//   nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return -1, if no such a key sig
//---------------------------------------------------------

Fraction KeyList::nextKeyTick(const Fraction& tick) const
      {
      if (empty())
            return Fraction(-1, 1);
//      auto i = upper_bound(tick+1);
      auto i = upper_bound(TimePosition(tick));
      return i == end() ? Fraction(-1,1) : i->first.tick();
      }

//---------------------------------------------------------
//   prevKey
//
//    returns the key before the current key for tick
//---------------------------------------------------------

KeySigEvent KeyList::prevKey(const Fraction& tick) const
      {
      KeySigEvent kc;
      kc.setKey(Key::C);

      if (empty())
            return kc;
      auto i = upper_bound(TimePosition(tick));
      if (i == begin())
            return kc;
      --i;
      if (i == begin())
            return kc;
      return (--i)->second;
      }

//---------------------------------------------------------
//   currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//---------------------------------------------------------

Fraction KeyList::currentKeyTick(const Fraction& tick) const
      {
      if (empty())
            return Fraction(0,1);
      auto i = upper_bound(TimePosition(tick));
      if (i == begin())
            return Fraction(0,1);
      --i;
      return i->first.tick();
      }

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(XmlReader& e, Score* cs)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "key") {
                  Key k;
                  int tick = e.intAttribute("tick", 0);
                  if (e.hasAttribute("custom"))
                        k = Key::C;      // ke.setCustomType(e.intAttribute("custom"));
                  else
                        k = Key(e.intAttribute("idx"));
                  KeySigEvent ke;
                  ke.setKey(k);
                  (*this)[TimePosition(Fraction::fromTicks(cs->fileDivision(tick)))] = ke;
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

}

