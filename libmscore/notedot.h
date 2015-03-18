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

#ifndef __NOTEDOT_H__
#define __NOTEDOT_H__

#include "element.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ NoteDot
//---------------------------------------------------------

class NoteDot : public Element {
      Q_OBJECT

      int _idx;

   public:
      NoteDot(Score* = 0);
      virtual NoteDot* clone() const override     { return new NoteDot(*this); }
      virtual Element::Type type() const override { return Element::Type::NOTEDOT; }
      int idx() const                    { return _idx; }
      void setIdx(int val)               { _idx = val; }
      virtual qreal mag() const;

      virtual void draw(QPainter*) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;

      Note* note() const { return (Note*)parent(); }
      };


}     // namespace Ms
#endif

