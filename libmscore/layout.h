//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
      int seg;
      qreal stretch;
      qreal fix;
      Spring(int i, qreal s, qreal f) : seg(i), stretch(s), fix(f) {}
      };

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;
typedef SpringMap::iterator iSpring;

extern qreal sff(qreal x, qreal xMin, SpringMap& springs);

#endif

