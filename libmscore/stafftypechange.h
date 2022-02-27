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
      StaffType* m_staffType { nullptr };
      bool m_ownsStaffType = false;
      qreal lw;

      void layout() override;
      void spatiumChanged(qreal oldValue, qreal newValue) override;
      void draw(QPainter*) const override;

   public:
      ~StaffTypeChange() override ;

      StaffTypeChange(Score* = 0);
      StaffTypeChange(const StaffTypeChange&);

      StaffTypeChange* clone() const override   { return new StaffTypeChange(*this); }
      ElementType type() const override         { return ElementType::STAFFTYPE_CHANGE; }

      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      const StaffType* staffType() const     { return m_staffType; }
      void setStaffType(StaffType* st, bool owned);

      Measure* measure() const               { return toMeasure(parent());   }

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms

#endif

