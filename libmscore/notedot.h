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

#ifndef __NOTEDOT_H__
#define __NOTEDOT_H__

#include "element.h"

//---------------------------------------------------------
//   @@ NoteDot
//---------------------------------------------------------

class NoteDot : public Element {
      Q_OBJECT

      int _idx;

   public:
      NoteDot(Score* =0);
      virtual NoteDot* clone() const   { return new NoteDot(*this); }
      virtual ElementType type() const { return NOTEDOT; }
      int idx() const                  { return _idx; }
      void setIdx(int val)             { _idx = val; }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();
      };

#endif

