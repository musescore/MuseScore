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
      int _repeatMeasureSize;
      int _repeatMeasureSlashes;                 // MusicXML says: "The slashes attribute specifies the number of slashes to use in the repeat sign. It is 1 if not specified."

   public:
      RepeatMeasure(Score*, int repeatMeasureSize = 1, int slashes = 1);
      RepeatMeasure &operator=(const RepeatMeasure&) = delete;
      virtual RepeatMeasure* clone() const override   { return new RepeatMeasure(*this); }
      virtual Element* linkedClone() override         { return Element::linkedClone(); }
      virtual Element::Type type() const override     { return Element::Type::REPEAT_MEASURE; }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual Fraction duration() const override;
      Fraction actualDuration() const { return Rest::duration(); }
      int repeatMeasureSize() const { return _repeatMeasureSize; }
      int repeatMeasureSlashes() const { return _repeatMeasureSlashes; }
      void setRepeatMeasureSize(int repeatMeasureSize) { _repeatMeasureSize = repeatMeasureSize; }

      virtual QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

