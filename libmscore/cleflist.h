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

#include "clef.h"
#include "fraction.h"
#include "timemap.h"

namespace Ms {

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

class ClefList : public TimeMap<ClefTypeList> {
      typedef TimeMap<ClefTypeList> ClefMap;
      static const ClefTypeList defaultClef;

   public:
      ClefTypeList clef(const Fraction& f) const { return ClefMap::value(f, defaultClef); }
      void setClef(const Fraction& f, ClefTypeList l) { ClefMap::insert(f, l); }
      Fraction nextClefTick(const Fraction& f) const { return ClefMap::nextValueTime(f); }
      Fraction currentClefTick(const Fraction& f) const { return ClefMap::currentValueTime(f); }
      };

}     // namespace Ms
#endif

