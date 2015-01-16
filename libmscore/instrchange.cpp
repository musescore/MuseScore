//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "instrchange.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "undo.h"
#include "mscore.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::INSTRUMENT_CHANGE);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(Xml& xml) const
      {
      xml.stag("InstrumentChange");
      if (segment())
            staff()->part()->instr(segment()->tick())->write(xml); // _instrument may not reflect mixer changes
      else
            _instrument.write(xml);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument")
                  _instrument.read(e);
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant InstrumentChange::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentChange::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Text::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool InstrumentChange::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            default:
                  return Text::setProperty(propertyId, v);
            }
      return true;
      }

}

