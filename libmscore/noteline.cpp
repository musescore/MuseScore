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

#include "noteline.h"
#include "textline.h"

namespace Ms {

NoteLine::NoteLine(Score* s)
   : TextLineBase(s)
      {
      }

NoteLine::NoteLine(const NoteLine& nl)
   : TextLineBase(nl)
      {
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* NoteLine::createLineSegment()
      {
      TextLineSegment* seg = new TextLineSegment(score());
      return seg;
      }


}     // namespace Ms

