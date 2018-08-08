//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "staff.h"
#include "xml.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   staffStyle
//---------------------------------------------------------

static const ElementStyle staffStyle {
      { Sid::staffTextPlacement, Pid::PLACEMENT },
      };

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s, Tid tid)
   : StaffTextBase(s, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&staffStyle);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffText::layout()
      {
      Staff* s = staff();
      qreal y = placeAbove() ? styleP(Sid::staffTextPosAbove) : styleP(Sid::staffTextPosBelow) + (s ? s->height() : 0.0);
      QPointF o(offset() * (offsetType() == OffsetType::SPATIUM ? spatium() : DPI));
      setPos(o + QPointF(0.0, y));
      TextBase::layout1();
      autoplaceSegmentElement(styleP(Sid::staffTextMinDistance));
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant StaffText::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::STAFF);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

