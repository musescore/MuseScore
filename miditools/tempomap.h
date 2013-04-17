//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEMPOMAP_H__
#define __TEMPOMAP_H__

#include <map>

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

class TempoMap : public std::map<int, double> {

   public:
      double tempo(int tick) const;
      int time2tick(double, double, int) const;
      };

#endif
