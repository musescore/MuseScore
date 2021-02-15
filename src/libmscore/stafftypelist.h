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
//    to keep track of staff type changes
//---------------------------------------------------------

class StaffTypeList
{
    StaffType firstStaffType;   ///< staff type at tick 0
    std::map<int, StaffType> staffTypeChanges;

public:
    StaffTypeList() {}
    StaffType& staffType(const Fraction&);
    const StaffType& staffType(const Fraction& f) const;
    StaffType* setStaffType(const Fraction&, const StaffType&);
    bool removeStaffType(const Fraction&);
    void read(XmlReader&, Score*);

    bool uniqueStaffType() const { return staffTypeChanges.empty(); }
    std::pair<int, int> staffTypeRange(const Fraction&) const;
};
}

#endif
