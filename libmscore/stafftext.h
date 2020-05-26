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

#ifndef __STAFFTEXT_H__
#define __STAFFTEXT_H__

#include "text.h"
#include "part.h"
#include "staff.h"
#include "stafftextbase.h"

namespace Ms {
//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

class StaffText final : public StaffTextBase
{
    QVariant propertyDefault(Pid id) const override;

public:
    StaffText(Score* s = 0, Tid = Tid::STAFF);

    StaffText* clone() const override { return new StaffText(*this); }
    ElementType type() const override { return ElementType::STAFF_TEXT; }
    void layout() override;
};
}     // namespace Ms
#endif
