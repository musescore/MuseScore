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

#ifndef __ELEMENTMAP_H__
#define __ELEMENTMAP_H__

namespace Ms {

class Element;

//---------------------------------------------------------
//   ElementMap
//---------------------------------------------------------

class ElementMap : QHash<Element*, Element*> {

   public:
      ElementMap() {}
      Element* findNew(Element* o) const { return value(o); }
      void add(Element* o, Element* n)   { insert(o, n); }
      };


}     // namespace Ms
#endif

