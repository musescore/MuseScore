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

namespace Ms {
//---------------------------------------------------------
//   @@ InstrumentChange
//---------------------------------------------------------

class InstrumentChange final : public TextBase
{
    Q_DECLARE_TR_FUNCTIONS(InstrumentChange)
    Instrument* _instrument;    // Staff holds ownership if part of score
    bool _init = false;   // Set if the instrument has been set by the user, as there is no other way to tell.

public:
    InstrumentChange(Score*);
    InstrumentChange(const Instrument&, Score*);
    InstrumentChange(const InstrumentChange&);
    ~InstrumentChange();

    InstrumentChange* clone() const override { return new InstrumentChange(*this); }
    ElementType type() const override { return ElementType::INSTRUMENT_CHANGE; }

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    void layout() override;

    Instrument* instrument() const { return _instrument; }
    void setInstrument(Instrument* i) { _instrument = i; }
    void setInstrument(Instrument&& i) { *_instrument = i; }
    void setInstrument(const Instrument& i);
    void setupInstrument(const Instrument* instrument);

    std::vector<KeySig*> keySigs() const;
    std::vector<Clef*> clefs() const;

    bool init() const { return _init; }
    void setInit(bool init) { _init = init; }

    Segment* segment() const { return toSegment(parent()); }

    QVariant propertyDefault(Pid) const override;

    bool placeMultiple() const override { return false; }
};
}     // namespace Ms
#endif
