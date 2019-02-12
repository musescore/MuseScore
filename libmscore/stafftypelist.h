//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFFTYPELIST_H__
#define __STAFFTYPELIST_H__

#include "stafftype.h"

namespace Ms {

class XmlReader;

//---------------------------------------------------------
//   StaffTypeList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

class StaffTypeList : public std::map<int, StaffType> {

   public:
      StaffTypeList() {}
      StaffType& staffType(int tick);
      const StaffType& staffType(int tick) const;
      StaffType* setStaffType(int tick, const StaffType&);
      void read(XmlReader&, Score*);
      };

}

#endif


