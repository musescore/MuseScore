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

#ifndef __KEYLIST_H__
#define __KEYLIST_H__

#include "types.h"
#include "key.h"
#include "timeposition.h"

namespace Ms {

class XmlReader;

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

typedef std::map<TimePosition, KeySigEvent> KeyMap;

class KeyList : public KeyMap {

   public:
      KeySigEvent key(const Fraction& tick) const;
      KeySigEvent prevKey(const Fraction& tick) const;
      void setKey(const Fraction& tick, KeySigEvent);
      Fraction nextKeyTick(const Fraction& tick) const;
      Fraction currentKeyTick(const Fraction& tick) const;

      void read(XmlReader&, Score*);

      iterator lower_bound(const Fraction& t)          { return KeyMap::lower_bound(TimePosition(t)); }
      iterator begin()                                 { return KeyMap::begin(); }
      const_iterator begin() const                     { return KeyMap::begin(); }
      iterator end()                                   { return KeyMap::end(); }
      const_iterator end() const                       { return KeyMap::end(); }
      };

}

#endif

