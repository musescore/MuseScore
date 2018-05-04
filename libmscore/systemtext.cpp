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
   : TextBase(s, ElementFlag::SYSTEM)
      {
      init(SubStyle::SYSTEM);
      }

SystemText::SystemText(SubStyle ss, Score* s)
   : TextBase(s, ElementFlag::SYSTEM)
      {
      init(ss);
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
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemText::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(name());
      TextBase::writeProperties(xml);
      xml.etag();
      }

} // namespace Ms

