//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

namespace Ms {

//---------------------------------------------------------
//   @@ InstrumentChange
//---------------------------------------------------------

class InstrumentChange : public Text  {
      Q_OBJECT

      Instrument _instrument;

   public:
      InstrumentChange(Score*);
      virtual InstrumentChange* clone() const { return new InstrumentChange(*this); }
      virtual Element::Type type() const      { return Element::Type::INSTRUMENT_CHANGE; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment() const                { return (Segment*)parent(); }
      };


}     // namespace Ms
#endif
