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
      setInstrumentNameType(INSTRUMENT_NAME_SHORT);
      _layoutPos = 0;
      }

//---------------------------------------------------------
//   instrumentNameTypeName
//---------------------------------------------------------

QString InstrumentName::instrumentNameTypeName() const
      {
      if (instrumentNameType() == INSTRUMENT_NAME_SHORT)
            return QString("short");
      return QString("long");
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(const QString& s)
      {
      if (s == "short")
            setInstrumentNameType(INSTRUMENT_NAME_SHORT);
      if (s == "long")
            setInstrumentNameType(INSTRUMENT_NAME_LONG);
      else
            qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
      }

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
      {
      _instrumentNameType = st;
      if (st == INSTRUMENT_NAME_SHORT)
            setTextStyleType(TEXT_STYLE_INSTRUMENT_SHORT);
      else
            setTextStyleType(TEXT_STYLE_INSTRUMENT_LONG);
      }

}

