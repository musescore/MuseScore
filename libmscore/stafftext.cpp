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
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : StaffTextBase(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initSubStyle(SubStyleId::STAFF);
      }

StaffText::StaffText(SubStyleId ss, Score* s)
   : StaffTextBase(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initSubStyle(ss);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffText::layout()
      {
      Staff* s = staff();
      qreal y = placeAbove() ? styleP(Sid::staffTextPosAbove) : styleP(Sid::staffTextPosBelow) + (s ? s->height() : 0.0);
      setPos(QPointF(0.0, y));
      TextBase::layout1();
      autoplaceSegmentElement(styleP(Sid::staffTextMinDistance));
      }

}

