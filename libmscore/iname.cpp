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

namespace Ms {

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : Text(s)
      {
      setInstrumentNameType(InstrumentNameType::SHORT);
      _layoutPos = 0;
      }

//---------------------------------------------------------
//   instrumentNameTypeName
//---------------------------------------------------------

QString InstrumentName::instrumentNameTypeName() const
      {
      if (instrumentNameType() == InstrumentNameType::SHORT)
            return QString("short");
      return QString("long");
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(const QString& s)
      {
      if (s == "short")
            setInstrumentNameType(InstrumentNameType::SHORT);
      if (s == "long")
            setInstrumentNameType(InstrumentNameType::LONG);
      else
            qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
      }

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
      {
      _instrumentNameType = st;
      if (st == InstrumentNameType::SHORT)
            setTextStyleType(TextStyleType::INSTRUMENT_SHORT);
      else
            setTextStyleType(TextStyleType::INSTRUMENT_LONG);
      }

}

