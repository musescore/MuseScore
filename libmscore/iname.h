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

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

class InstrumentName final : public TextBase  {
      InstrumentNameType _instrumentNameType;
      int _layoutPos { 0 };

   public:
      InstrumentName(Score*);
      virtual InstrumentName* clone() const override { return new InstrumentName(*this); }
      virtual ElementType type() const override    { return ElementType::INSTRUMENT_NAME; }

      int layoutPos() const      { return _layoutPos; }
      void setLayoutPos(int val) { _layoutPos = val;  }

      QString instrumentNameTypeName() const;
      InstrumentNameType instrumentNameType() const { return _instrumentNameType; }
      void setInstrumentNameType(InstrumentNameType v);
      void setInstrumentNameType(const QString& s);

      virtual bool isEditable() const override { return false; }
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif

