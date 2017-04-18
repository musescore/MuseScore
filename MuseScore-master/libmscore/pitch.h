//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PITCH_H__
#define __PITCH_H__

namespace Ms {

//---------------------------------------------------------
///  PitchList
///  List of note pitch offsets
//---------------------------------------------------------

class PitchList : public QMap<int, int> {
   public:
      PitchList() {}
      int pitchOffset(int tick) const;
      void setPitchOffset(int tick, int offset) { insert(tick, offset); }
      };


}     // namespace Ms
#endif

