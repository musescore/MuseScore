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
      virtual RepeatMeasure* clone() const override   { return new RepeatMeasure(*this); }
      virtual Element* linkedClone() override         { return Element::linkedClone(); }
      virtual ElementType type() const override       { return ElementType::REPEAT_MEASURE; }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual Fraction ticks() const override;
      Fraction actualTicks() const { return Rest::ticks(); }

      virtual QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

