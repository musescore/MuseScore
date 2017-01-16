//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "iname.h"
#include "staff.h"
#include "part.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : Text(s)
      {
      setInstrumentNameType(InstrumentNameType::SHORT);
      }

//---------------------------------------------------------
//   instrumentNameTypeName
//---------------------------------------------------------

QString InstrumentName::instrumentNameTypeName() const
      {
      return instrumentNameType() == InstrumentNameType::SHORT ? "short" : "long";
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(const QString& s)
      {
      if (s == "short")
            setInstrumentNameType(InstrumentNameType::SHORT);
      else if (s == "long")
            setInstrumentNameType(InstrumentNameType::LONG);
      else
            qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
      {
      _instrumentNameType = st;
      initSubStyle(st == InstrumentNameType::SHORT ? SubStyle::INSTRUMENT_SHORT : SubStyle::INSTRUMENT_LONG);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void InstrumentName::endEdit()
      {
      Text::endEdit();
      Part* part = staff()->part();
      Instrument* instrument = new Instrument(*part->instrument());

      QString s = plainText();

      if (_instrumentNameType == InstrumentNameType::LONG)
            instrument->setLongName(s);
      else
            instrument->setShortName(s);
      score()->undo(new ChangePart(part, instrument, part->name()));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant InstrumentName::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::INAME_LAYOUT_POSITION:
                  return _layoutPos;
            default:
                  return Text::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool InstrumentName::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::INAME_LAYOUT_POSITION:
                  _layoutPos = v.toInt();
printf("%p set layoutPos %d\n", this, _layoutPos);
                  break;
            default:
                  return Text::setProperty(id, v);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentName::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::INAME_LAYOUT_POSITION:
                  return 0;
            default:
                  return Text::propertyDefault(id);
            }
      }

}

