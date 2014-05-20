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

#include "mscore.h"
#include "clef.h"

namespace Ms {

class Score;
class Staff;

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

class ClefList {

   protected:
      std::map<int, ClefTypeList>   _list;
      Staff*                        _staff;

   public:
      ClefList(Staff* staff);
//      ClefList(const ClefList& other);
      ClefList& operator=(const ClefList& other);
      ~ClefList();

      void         clear();
      ClefTypeList clef(int tick) const;
      bool         empty()                      { return _list.empty(); }
      void         insertTime(int tick, int len);
      bool         isClefChangeAt(int tick) const;
      void         setClef(int tick, ClefTypeList);
      void         read(XmlReader&, Score*);
      };


}     // namespace Ms
#endif

