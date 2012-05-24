//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INSTRCHANGE_H__
#define __INSTRCHANGE_H__

#include "text.h"
#include "instrument.h"

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

class InstrumentChange : public Text  {
      Instrument _instrument;

   public:
      InstrumentChange(Score*);
      virtual InstrumentChange* clone() const { return new InstrumentChange(*this); }
      virtual ElementType type() const        { return INSTRUMENT_CHANGE; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);

      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };

#endif
