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

#include "types.h"
#include "stafftypelist.h"
#include "xml.h"
//#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType& StaffTypeList::staffType(const Fraction& tick) const
      {
      static const StaffType st;
      if (empty())
            return st;
      auto i = upper_bound(TimePosition(tick));
      if (i == begin())
            return st;
      return (--i)->second;
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

StaffType& StaffTypeList::staffType(const Fraction& tick)
      {
      Q_ASSERT(!empty());
      auto i = upper_bound(TimePosition(tick));
      Q_ASSERT(i != begin());
      return (--i)->second;
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* StaffTypeList::setStaffType(const Fraction& tick, const StaffType& st)
      {
      Q_ASSERT(tick >= Fraction(0,1));
      auto i = find(tick);
      StaffType* nst;
      if (i == end()) {
            auto k = insert(std::pair<TimePosition, StaffType>(TimePosition(tick), st));
            nst = &(k.first->second);
            }
      else {
            i->second = st;
            nst = &i->second;
            }
      return nst;
      }

//---------------------------------------------------------
//   StaffTypeList::read
//---------------------------------------------------------

#if 0
void StaffTypeList::read(XmlReader& /*e*/, Score* /*cs*/)
      {
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
      }
#endif

}

