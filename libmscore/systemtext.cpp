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

#include "systemtext.h"

namespace Ms {

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

SystemText::SystemText(Score* s)
   : StaffText(SubStyle::SYSTEM, s)
      {
      setSystemFlag(true);
      }

SystemText::SystemText(SubStyle ss, Score* s)
   : StaffText(ss, s)
      {
      setSystemFlag(true);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SystemText::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::SYSTEM);
            default:
                  return StaffText::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemText::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("SystemText");
      StaffText::writeProperties(xml);
      xml.etag();
      }

} // namespace Ms

