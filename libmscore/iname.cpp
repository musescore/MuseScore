//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : Text(s)
      {
      setSubtype(INSTRUMENT_NAME_SHORT);
      _layoutPos = 0;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString InstrumentName::subtypeName() const
      {
      if (subtype() == INSTRUMENT_NAME_SHORT)
            return QString("short");
      return QString("long");
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void InstrumentName::setSubtype(const QString& s)
      {
      if (s == "short")
            setSubtype(INSTRUMENT_NAME_SHORT);
      if (s == "long")
            setSubtype(INSTRUMENT_NAME_LONG);
      else
            qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void InstrumentName::setSubtype(InstrumentNameType st)
      {
      _subtype = st;
      if (st == INSTRUMENT_NAME_SHORT)
            setTextStyleType(TEXT_STYLE_INSTRUMENT_SHORT);
      else
            setTextStyleType(TEXT_STYLE_INSTRUMENT_LONG);
      }

