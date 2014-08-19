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

class RepeatMeasure : public Rest {
      Q_OBJECT

      QPainterPath path;

   public:
      RepeatMeasure(Score*);
      RepeatMeasure &operator=(const RepeatMeasure&) = delete;
      virtual RepeatMeasure* clone() const  { return new RepeatMeasure(*this); }
      virtual Element::Type type() const     { return Element::Type::REPEAT_MEASURE; }
      virtual void draw(QPainter*) const;
      virtual void layout();
      virtual Fraction duration() const;

      virtual QString accessibleInfo();
      };


}     // namespace Ms
#endif

