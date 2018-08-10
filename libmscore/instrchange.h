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

class InstrumentChange final : public TextBase  {
      Instrument* _instrument;  // Staff holds ownership if part of score

   public:
      InstrumentChange(Score*);
      InstrumentChange(const Instrument&, Score*);
      InstrumentChange(const InstrumentChange&);
      ~InstrumentChange();

      virtual InstrumentChange* clone() const override { return new InstrumentChange(*this); }
      virtual ElementType type() const override        { return ElementType::INSTRUMENT_CHANGE; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void read300(XmlReader&) override;

      Instrument* instrument() const        { return _instrument;  }
      void setInstrument(Instrument* i)     { _instrument = i;     }
      void setInstrument(Instrument&& i)    { *_instrument = i;    }
      void setInstrument(const Instrument& i);

      Segment* segment() const              { return toSegment(parent()); }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif
