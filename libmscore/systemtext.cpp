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
//   systemStyle
//---------------------------------------------------------

static const ElementStyle systemStyle {
      { Sid::systemTextPlacement,                Pid::PLACEMENT              },
      { Sid::systemTextMinDistance,              Pid::MIN_DISTANCE           },
      };

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

SystemText::SystemText(Score* s, Tid tid)
   : StaffTextBase(s, tid, ElementFlag::SYSTEM | ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&systemStyle);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SystemText::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SUB_STYLE:
                  return int(Tid::SYSTEM);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SystemText::layout()
      {
      TextBase::layout();
      autoplaceSegmentElement();
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid SystemText::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::systemTextPosAbove : Sid::systemTextPosBelow;
      return TextBase::getPropertyStyle(pid);
      }

} // namespace Ms

