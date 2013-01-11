//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

enum StaffStateType {
      STAFF_STATE_INSTRUMENT, STAFF_STATE_TYPE,
      STAFF_STATE_VISIBLE, STAFF_STATE_INVISIBLE
      };

//---------------------------------------------------------
//   @@ StaffState
//---------------------------------------------------------

class StaffState : public Element {
      Q_OBJECT

      StaffStateType _subtype;
      qreal lw;
      QPainterPath path;

      Instrument _instrument;

      virtual void draw(QPainter*) const;
      virtual void layout();

   public:
      StaffState(Score*);
      virtual StaffState* clone() const { return new StaffState(*this); }
      virtual ElementType type() const   { return STAFF_STATE; }

      void setSubtype(const QString&);
      void setSubtype(StaffStateType st)    { _subtype = st; }
      StaffStateType subtype() const        { return _subtype; }
      QString subtypeName() const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };

#endif
