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
///   The basic element making measure numbers.
///   Reimplemented by MMRestRange
//---------------------------------------------------------

class MeasureNumber : public TextBase {

      M_PROPERTY (HPlacement, hPlacement, setHPlacement) // Horizontal Placement

   public:
      MeasureNumber(Score* = nullptr, Tid tid = Tid::MEASURE_NUMBER, ElementFlags flags = ElementFlag::NOTHING);
      MeasureNumber(const MeasureNumber& other);

      virtual ElementType type() const override       { return ElementType::MEASURE_NUMBER; }
      virtual MeasureNumber* clone() const override   { return new MeasureNumber(*this); }

      virtual QVariant getProperty(Pid id) const override;
      virtual bool setProperty(Pid id, const QVariant& val) override;
      virtual QVariant propertyDefault(Pid id) const override;

      virtual bool readProperties(XmlReader&) override;

      virtual void layout() override;
      Measure* measure() const { return toMeasure(parent()); }

      virtual bool isEditable() const override { return false; } // The measure numbers' text should not be editable
      };

}     // namespace Ms

#endif
