//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __VELO_H__
#define __VELO_H__

#include "fraction.h"
#include "hairpin.h"

/**
 \file
 Definition of classes VeloList.
*/

namespace Ms {

//---------------------------------------------------------
///   VeloEvent
///   item in VeloList
//---------------------------------------------------------

enum class VeloType : char { FIX, RAMP };
enum class VeloDirection : signed char {
      CRESCENDO,
      DIMINUENDO,
      };

class VeloEvent {
      // Despite storing the tick as the key of the VeloEvent in the VeloList, we also store it here.
      // Since we're not going to be changing it (well, not much, anyway), this is good because it provides
      // multiple ways to access the tick. Keeping it in sync isn't much trouble.
      Fraction tick;
      int value;
      VeloType type;
      Fraction etick;
      VeloChangeMethod method;
      int cachedStart   { -1 };
      int cachedEnd     { -1 };
      VeloDirection direction;

   public:
      VeloEvent(Fraction t, int vel) : tick(t), value(vel), type(VeloType::FIX) {}
      VeloEvent(Fraction s, Fraction e, int diff, VeloChangeMethod m, VeloDirection d)
            : tick(s), value(diff), type(VeloType::RAMP), etick(e), method(m), direction(d) {}

      bool operator==(const VeloEvent& event) const;
      bool operator!=(const VeloEvent& event) const;

      friend class VeloList;
      };

//---------------------------------------------------------
///  VeloList
///  List of note velocity changes
//---------------------------------------------------------

class VeloList : public QMultiMap<Fraction, VeloEvent> {
      bool cleanedUp    { false };
      static const int DEFAULT_VELOCITY  { 80 };

   public:
      VeloList() {}
      int velo(Fraction tick);
      std::vector<std::pair<Fraction, Fraction>> changesInRange(Fraction stick, Fraction etick);

      void addFixed(Fraction tick, int velocity);
      void addRamp(Fraction stick, Fraction etick, int velChange, VeloChangeMethod method, VeloDirection direction);
      void cleanup();

      void dump();

      static int interpolateVelocity(VeloEvent& event, Fraction& tick);
      };

}     // namespace Ms
#endif

