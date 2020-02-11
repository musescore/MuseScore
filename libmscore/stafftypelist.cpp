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

#include "stafftypelist.h"
#include "xml.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType& StaffTypeList::staffType(const Fraction& tick) const
      {
      static const StaffType st;

      if (tick.negative())
            return st;
      if (staffTypeChanges.empty())
            return firstStaffType;

      auto i = staffTypeChanges.upper_bound(tick.ticks());
      if (i == staffTypeChanges.begin())
            return firstStaffType;
      return (--i)->second;
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

StaffType& StaffTypeList::staffType(const Fraction& tick)
      {
      Q_ASSERT(!tick.negative());

      if (staffTypeChanges.empty())
            return firstStaffType;

      auto i = staffTypeChanges.upper_bound(tick.ticks());
      if (i == staffTypeChanges.begin())
            return firstStaffType;
      return (--i)->second;
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* StaffTypeList::setStaffType(const Fraction& tick, const StaffType& st)
      {
      Q_ASSERT(!tick.negative());

      if (tick.isZero()) {
            firstStaffType = st;
            return &firstStaffType;
            }

      auto i = staffTypeChanges.find(tick.ticks());
      StaffType* nst;
      if (i == staffTypeChanges.end()) {
            auto k = staffTypeChanges.insert(std::pair<int, StaffType>(tick.ticks(), st));
            nst = &(k.first->second);
            }
      else {
            i->second = st;
            nst = &i->second;
            }
      return nst;
      }

//---------------------------------------------------------
//   removeStaffType
//---------------------------------------------------------

bool StaffTypeList::removeStaffType(const Fraction& tick)
      {
      Q_ASSERT(!tick.negative());
      if (tick.isZero()) {
            firstStaffType = StaffType();
            return true;
            }

      auto i = staffTypeChanges.find(tick.ticks());

      if (i == staffTypeChanges.end())
            return false;

      staffTypeChanges.erase(i);
      return true;
      }

//---------------------------------------------------------
//   staffTypeRange
//---------------------------------------------------------

std::pair<int, int> StaffTypeList::staffTypeRange(const Fraction& tick) const
      {
      if (tick.negative())
            return { -1, -1 };

      auto i = staffTypeChanges.upper_bound(tick.ticks());
      const int end = (i == staffTypeChanges.end()) ? -1 : i->first;
      const int start = (i == staffTypeChanges.begin()) ? 0 : (--i)->first;

      return { start, end };
      }

//---------------------------------------------------------
//   StaffTypeList::read
//---------------------------------------------------------

void StaffTypeList::read(XmlReader& /*e*/, Score* /*cs*/)
      {
#if 0
      while (e.readNextStartElement()) {
            if (e.name() == "key") {
                  Key k;
                  int tick = e.intAttribute("tick", 0);
                  if (e.hasAttribute("custom"))
                        k = Key::C;      // ke.setCustomType(e.intAttribute("custom"));
                  else
                        k = Key(e.intAttribute("idx"));
                  KeySigEvent ke;
                  ke.setKey(k);
                  (*this)[cs->fileDivision(tick)] = ke;
                  e.readNext();
                  }
            else
                  e.unknown();
            }
#endif
      }

}

