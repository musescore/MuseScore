//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STEMSLASH_H__
#define __STEMSLASH_H__

#include "element.h"
#include "stem.h"

namespace Ms {

//---------------------------------------------------------
//   @@ StemSlash
///    used for grace notes of type acciaccatura
//---------------------------------------------------------

class StemSlash final : public Element {
      QLineF line;

   public:
      StemSlash(Score* s = 0) : Element(s)   {}

      qreal mag() const override         { return parent()->mag(); }
      void setLine(const QLineF& l);

      StemSlash* clone() const override  { return new StemSlash(*this); }
      ElementType type() const override  { return ElementType::STEM_SLASH; }
      void draw(QPainter*) const override;
      void layout() override;
      Chord* chord() const               { return (Chord*)parent(); }
      };


}     // namespace Ms
#endif

