//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measurenumber.h"
// #include "xml.h"
#include "measure.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   measureNumberStyle
//---------------------------------------------------------

static const ElementStyle measureNumberStyle {
      };

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

MeasureNumber::MeasureNumber(Score* s) : TextBase(s, Tid::MEASURE_NUMBER)
      {
      setFlag(ElementFlag::ON_STAFF, true);
      resetProperty(Pid::PLACEMENT);
      initElementStyle(&measureNumberStyle);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MeasureNumber::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::MEASURE_NUMBER);
            case Pid::PLACEMENT:
                  return int (Placement::ABOVE);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureNumber::layout()
      {
      setPos(QPointF());
      if (!parent())
            setOffset(0.0, 0.0);
      else if (isStyled(Pid::OFFSET))
            setOffset(propertyDefault(Pid::OFFSET).toPointF());

      StaffType* st = staff()->staffType(measure()->tick());
      if (st->lines() == 1 && staff()) {
            rypos() = (placeBelow() ? 2.0 : -2.0) * spatium();
            }
      else {
            if (placeBelow())
                  rypos() = staff() ? staff()->height() : 0.0;
            }
      TextBase::layout1();
      }

}

