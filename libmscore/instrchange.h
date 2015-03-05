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

      Instrument* _instrument;  // Staff holds ownership if part of score

   public:
      InstrumentChange(Score*);
      InstrumentChange(const Instrument&, Score*);
      InstrumentChange(const InstrumentChange&);
      ~InstrumentChange();

      virtual InstrumentChange* clone() const override { return new InstrumentChange(*this); }
      virtual Element::Type type() const override      { return Element::Type::INSTRUMENT_CHANGE; }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;

      Instrument* instrument() const        { return _instrument;  }
      void setInstrument(Instrument* i)     { _instrument = i;     }
      void setInstrument(Instrument&& i)    { *_instrument = i;    }
      void setInstrument(const Instrument& i);

      Segment* segment() const                { return (Segment*)parent(); }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms
#endif
