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
      initSubStyle(SubStyleId::SYSTEM);
      }

SystemText::SystemText(SubStyleId ss, Score* s)
   : TextBase(s, ElementFlag::SYSTEM)
      {
      initSubStyle(ss);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SystemText::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SUB_STYLE:
                  return int(SubStyleId::SYSTEM);
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

