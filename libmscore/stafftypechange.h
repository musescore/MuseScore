//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFFTYPECHANGE_H__
#define __STAFFTYPECHANGE_H__

#include "element.h"

namespace Ms {

class StaffType;

//---------------------------------------------------------
//   @@ StaffTypeChange
//---------------------------------------------------------

class StaffTypeChange final : public Element {
      StaffType* _staffType { 0 };
      qreal lw;

      virtual void layout() override;
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;
      virtual void draw(QPainter*) const override;

   public:
      StaffTypeChange(Score* = 0);
      StaffTypeChange(const StaffTypeChange&);
      virtual StaffTypeChange* clone() const override { return new StaffTypeChange(*this); }

      virtual ElementType type() const override { return ElementType::STAFFTYPE_CHANGE; }

      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      const StaffType* staffType() const     { return _staffType; }
      void setStaffType(StaffType* st)       { _staffType = st; }

      Measure* measure() const         { return toMeasure(parent());   }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms

#endif

