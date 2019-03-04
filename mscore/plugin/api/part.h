//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_PART_H__
#define __PLUGIN_API_PART_H__

#include "scoreelement.h"
#include "libmscore/part.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part : public Ms::PluginAPI::ScoreElement {
      Q_OBJECT
      Q_PROPERTY(int                            startTrack          READ startTrack)
      Q_PROPERTY(int                            endTrack            READ endTrack)

   public:
      Part(Ms::Part* p = nullptr, Ownership o = Ownership::SCORE)
         : ScoreElement(p, o) {}

      Ms::Part* part() { return toPart(e); }
      const Ms::Part* part() const { return toPart(e); }

      int startTrack() const { return part()->startTrack(); }
      int endTrack()   const { return part()->endTrack(); }
      };
} // namespace PluginAPI
} // namespace Ms
#endif
