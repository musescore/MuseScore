//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFFSTATE_H__
#define __STAFFSTATE_H__

#include "element.h"
#include "instrument.h"

namespace Ms {

enum class StaffStateType : char {
      INSTRUMENT,
      TYPE,
      VISIBLE,
      INVISIBLE
      };

//---------------------------------------------------------
//   @@ StaffState
//---------------------------------------------------------

class StaffState final : public Element {
      StaffStateType _staffStateType { StaffStateType::INVISIBLE };
      qreal lw { 0.0 };
      QPainterPath path;

      Instrument* _instrument { nullptr };

      void draw(QPainter*) const override;
      void layout() override;

   public:
      StaffState(Score*);
      StaffState(const StaffState&);
      ~StaffState();

      StaffState* clone() const override  { return new StaffState(*this); }
      ElementType type() const override   { return ElementType::STAFF_STATE; }

      void setStaffStateType(const QString&);
      void setStaffStateType(StaffStateType st) { _staffStateType = st; }
      StaffStateType staffStateType() const     { return _staffStateType; }
      QString staffStateTypeName() const;

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      Instrument* instrument() const           { return _instrument; }
      void setInstrument(const Instrument* i)  { *_instrument = *i;    }
      void setInstrument(const Instrument&& i) { *_instrument = i;    }
      Segment* segment()                       { return (Segment*)parent(); }
      };


}     // namespace Ms
#endif
