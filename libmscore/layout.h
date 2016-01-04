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

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

namespace Ms {

class Segment;

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
      Segment* seg;
      qreal stretch;
      qreal fix;
      Spring(Segment* s, qreal str) : seg(s), stretch(str), fix(s->width()) {}
      };

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;

extern qreal sff(qreal x, qreal xMin, const SpringMap& springs);


}     // namespace Ms
#endif

