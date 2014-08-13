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
#include "elementlayout.h"
#include "instrument.h"

class QPainter;

namespace Ms {

enum class StaffStateType : char {
      INSTRUMENT, TYPE,
      VISIBLE, INVISIBLE
      };

//---------------------------------------------------------
//   @@ StaffState
//---------------------------------------------------------

class StaffState : public Element {
      Q_OBJECT

      StaffStateType _staffStateType;
      qreal lw;
      QPainterPath path;

      Instrument _instrument;

      virtual void draw(QPainter*) const;
      virtual void layout();

   public:
      StaffState(Score*);
      virtual StaffState* clone() const  { return new StaffState(*this); }
      virtual Element::Type type() const { return Element::Type::STAFF_STATE; }

      void setStaffStateType(const QString&);
      void setStaffStateType(StaffStateType st) { _staffStateType = st; }
      StaffStateType staffStateType() const     { return _staffStateType; }
      QString staffStateTypeName() const;

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };


}     // namespace Ms
#endif
