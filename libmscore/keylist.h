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

#include "key.h"
#include "timemap.h"

namespace Ms {

class XmlReader;

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

class KeyList : public TimeMap<KeySigEvent> {
      typedef TimeMap<KeySigEvent> KeyMap;
      static const KeySigEvent defaultKeySig;

   public:
      KeySigEvent key(const Fraction& tick) const { return KeyMap::value(tick, defaultKeySig); }
      void setKey(const Fraction& tick, KeySigEvent key) { KeyMap::insert(tick, key); }
      Fraction nextKeyTick(const Fraction& tick) const { return KeyMap::nextValueTime(tick); }
      Fraction currentKeyTick(const Fraction& tick) const { return KeyMap::currentValueTime(tick); }

      KeyMap::const_iterator lower_bound(const Fraction& tick) const { return KeyMap::lower_bound(tick); }
      KeyMap::const_iterator upper_bound(const Fraction& tick) const { return KeyMap::upper_bound(tick); }

      void read(XmlReader&, Score*);
      };

}

#endif

