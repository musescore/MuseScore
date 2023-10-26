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

#ifndef __INAME_H__
#define __INAME_H__

#include "text.h"

namespace Ms {

enum class InstrumentNameType : char {
      LONG, SHORT
      };

class System;

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

class InstrumentName final : public TextBase  {
      InstrumentNameType _instrumentNameType;
      int _layoutPos { 0 };

   public:
      InstrumentName(Score*);

      InstrumentName* clone() const override { return new InstrumentName(*this); }
      ElementType type() const override      { return ElementType::INSTRUMENT_NAME; }

      int layoutPos() const      { return _layoutPos; }
      void setLayoutPos(int val) { _layoutPos = val;  }

      QString instrumentNameTypeName() const;
      InstrumentNameType instrumentNameType() const { return _instrumentNameType; }
      void setInstrumentNameType(InstrumentNameType v);
      void setInstrumentNameType(const QString& s);

      System* system() const { return toSystem(parent()); }

      qreal spatium() const override;

      Fraction playTick() const override;
      bool isEditable() const override { return false; }
      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif

