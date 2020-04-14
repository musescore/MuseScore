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

#ifndef __REPEAT_H__
#define __REPEAT_H__

#include "text.h"
#include "rest.h"

namespace Ms {

class Score;
class Segment;

//---------------------------------------------------------
//   @@ RepeatMeasure
//---------------------------------------------------------

class RepeatMeasure final : public Rest {
      QPainterPath path;

   public:
      RepeatMeasure(Score*);
      RepeatMeasure &operator=(const RepeatMeasure&) = delete;

      RepeatMeasure* clone() const override   { return new RepeatMeasure(*this); }
      Element* linkedClone() override         { return Element::linkedClone(); }
      ElementType type() const override       { return ElementType::REPEAT_MEASURE; }
      void draw(QPainter*) const override;
      void layout() override;
      Fraction ticks() const override;
      Fraction actualTicks() const { return Rest::ticks(); }

      QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

