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

namespace Ms {

class XmlReader;

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

class KeyList : public std::map<const int, KeySigEvent> {

   public:
      KeyList() {}
      KeySigEvent key(int tick) const;
      KeySigEvent prevKey(int tick) const;
      void setKey(int tick, KeySigEvent);
      int nextKeyTick(int tick) const;
      int currentKeyTick(int tick) const;
      void read(XmlReader&, Score*);
      };

}

#endif

