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


#include "rangeannotation.h"
#include "textannotation.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "property.h"
#include "element.h"
#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "xml.h"
#include "text.h"
#include <QRectF>
#include <QPainter>
namespace Ms {

//---------------------------------------------------------
//   RangeAnnotation
//---------------------------------------------------------

RangeAnnotation::RangeAnnotation(Score* s)
  : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::HAS_TAG);
      _score         = s;
      _startSegment  = 0;
      _endSegment    = 0;
      _staffStart    = 0;
      _staffEnd      = 0;
  /*    QRectF r1(100, 300, 11, 16);
      QPainter painter(this);
      painter.drawRoundedRect(r1, 20.0, 15.0);*/
      }
int RangeAnnotation::tickStart() const
      {
      return _startSegment->tick();
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

int RangeAnnotation::tickEnd() const
      {
      return _endSegment->tick();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void RangeAnnotation::setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd)
      {
      _startSegment  = startSegment;
      _endSegment    = endSegment;
      _staffStart    = staffStart;
      _staffEnd      = staffEnd;
      }
}

