//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MEASURENUMBER_H__
#define __MEASURENUMBER_H__

#include "textbase.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

class MeasureNumber final : public TextBase {

   public:
      MeasureNumber(Score* s = 0);

      ElementType type() const override       { return ElementType::MEASURE_NUMBER; }
      MeasureNumber* clone() const override   { return new MeasureNumber(*this); }

      QVariant propertyDefault(Pid id) const override;

      void layout() override;
      Measure* measure() const { return toMeasure(parent()); }
      };

}     // namespace Ms

#endif

