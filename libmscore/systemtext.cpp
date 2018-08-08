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

static const ElementStyle systemStyle {
      { Sid::systemTextPlacement,                Pid::PLACEMENT              },
      };

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

SystemText::SystemText(Score* s)
   : StaffTextBase(s, Tid::SYSTEM, ElementFlag::SYSTEM)
      {
      initElementStyle(&systemStyle);
      }

SystemText::SystemText(Score* s, Tid tid, ElementFlags flags)
   : StaffTextBase(s, tid, flags)
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
      Staff* s = staff();
      qreal y = placeAbove() ? styleP(Sid::systemTextPosAbove) : styleP(Sid::systemTextPosBelow) + (s ? s->height() : 0.0);
      setPos(QPointF(0.0, y));
      TextBase::layout1();
      autoplaceSegmentElement(styleP(Sid::systemTextMinDistance));
      }

} // namespace Ms

