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
#include "clef.h"
#include "stafftext.h"

namespace Ms {

//---------------------------------------------------------
//   @@ InstrumentChangeWarning
//---------------------------------------------------------

class InstrumentChangeWarning final : public StaffTextBase {
      virtual Sid getPropertyStyle(Pid) const override;
      virtual QVariant propertyDefault(Pid id) const override;

   public:
      InstrumentChangeWarning(Score* s = 0, Tid = Tid::STAFF);
      virtual InstrumentChangeWarning* clone() const override { return new InstrumentChangeWarning(*this); }
      virtual ElementType type() const override { return ElementType::INSTRUMENT_CHANGE_WARNING; }
      virtual void layout() override;
};

//---------------------------------------------------------
//   @@ InstrumentChange
//---------------------------------------------------------

class InstrumentChange final : public TextBase {
      Instrument* _instrument;  // Staff holds ownership if part of score
      std::vector<KeySig*> _keySigs;
      std::vector<Clef*> _clefs;
      InstrumentChangeWarning* _warning = nullptr;

   public:
      InstrumentChange(Score*);
      InstrumentChange(const Instrument&, Score*);
      InstrumentChange(const InstrumentChange&);
      ~InstrumentChange();

      virtual InstrumentChange* clone() const override { return new InstrumentChange(*this); }
      virtual ElementType type() const override        { return ElementType::INSTRUMENT_CHANGE; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;

      Instrument* instrument() const        { return _instrument;  }
      void setInstrument(Instrument* i)     { _instrument = i;     }
      void setInstrument(Instrument&& i)    { *_instrument = i;    }
      void setInstrument(const Instrument& i);

      void setNextChord(ChordRest* chord);

      std::vector<KeySig*> keySigs() const  { return _keySigs;     }
      void addKeySig(KeySig* keySig)        { _keySigs.push_back(keySig); }

      std::vector<Clef*> clefs()            { return _clefs;       }
      void addClef(Clef* clef)              { _clefs.push_back(clef); }

      InstrumentChangeWarning* warning() const { return _warning; }
      void setWarning(InstrumentChangeWarning* warning) { _warning = warning; }

      Segment* segment() const              { return toSegment(parent()); }

      virtual QVariant propertyDefault(Pid) const override;

      virtual bool placeMultiple() const override      { return false; }
      };


}     // namespace Ms
#endif
