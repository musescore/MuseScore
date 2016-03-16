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

struct VeloEvent {
      VeloType type;
      char val;
      VeloEvent() {}
      VeloEvent(VeloType t, char v) : type(t), val(v) {}
      };

//---------------------------------------------------------
///  VeloList
///  List of note velocity changes
//---------------------------------------------------------

class VeloList : public QMap<int, VeloEvent> {
   public:
      VeloList() {}
      int velo(int tick) const;
      int nextVelo(int tick) const;
      void setVelo(int tick, VeloEvent velo);
      void setVelo(int tick, int velocity);
      };


}     // namespace Ms
#endif

