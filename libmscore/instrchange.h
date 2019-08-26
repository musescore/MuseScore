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
      InstrumentChangeWarning(Score* s = 0, Tid = Tid::INSTRUMENT_CHANGE);
      virtual InstrumentChangeWarning* clone() const override { return new InstrumentChangeWarning(*this); }
      virtual ElementType type() const override { return ElementType::INSTRUMENT_CHANGE_WARNING; }
      virtual void layout() override;
      };

//---------------------------------------------------------
//   @@ InstrumentChange
//---------------------------------------------------------

class InstrumentChange final : public TextBase {
      Instrument* _instrument;  // Staff holds ownership if part of score
      bool _init = false; // Set if the instrument has been set by the user, as there is no other way to tell.
      bool _showWarning = true;
      int _lines = 5;
      StaffGroup _staffGroup = StaffGroup::STANDARD;
      Q_DECLARE_TR_FUNCTIONS(setupInstrument)

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
      void setupInstrument(const Instrument* instrument);
      void setupClefs(const Instrument* instrument, int i, Staff* staff, Spatium clefOffset);

      Spatium setupStaffType(Staff* staff);

      std::vector<KeySig*> keySigs() const;
      std::vector<Clef*> clefs() const;

      bool init() const                     { return _init; }
      void setInit(bool init)               { _init = init; }

      bool showWarning() const              { return _showWarning; }
      void setShowWarning(bool showWarning) { _showWarning = showWarning; }
      void setNextChord(ChordRest* chord);
      void setLines(int lines)              { _lines = lines; }
      int lines() const                     { return _lines; }

      void setStaffGroup(StaffGroup staffGroup) { _staffGroup = staffGroup; }
      StaffGroup staffGroup() const         { return _staffGroup; }

      Segment* segment() const              { return toSegment(parent()); }

      virtual QVariant propertyDefault(Pid) const override;

      virtual bool placeMultiple() const override      { return false; }
      };


}     // namespace Ms
#endif
