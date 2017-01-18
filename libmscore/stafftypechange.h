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

class StaffTypeChange : public Element {
      Q_OBJECT

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
      virtual bool systemFlag() const override    { return false;  }

      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      StaffType* staffType() const     { return _staffType; }
      void setStaffType(StaffType* st) { _staffType = st; }

      Measure* measure() const            { return (Measure*)parent();   }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms

#endif

