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

#ifndef __PEDAL_H__
#define __PEDAL_H__

#include "textlinebase.h"

namespace Ms {

class Pedal;

//---------------------------------------------------------
//   @@ PedalSegment
//---------------------------------------------------------

class PedalSegment final : public TextLineBaseSegment {
   public:
      PedalSegment(Score* s) : TextLineBaseSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF) {}
      virtual ElementType type() const override       { return ElementType::PEDAL_SEGMENT; }
      virtual PedalSegment* clone() const override    { return new PedalSegment(*this);    }
      Pedal* pedal() const                            { return toPedal(spanner());          }
      virtual void layout() override;

      friend class Pedal;
      };

//---------------------------------------------------------
//   @@ Pedal
//---------------------------------------------------------

class Pedal final : public TextLineBase {
   protected:
      QPointF linePos(Grip, System**) const override;

   public:
      Pedal(Score* s);
      virtual Pedal* clone() const override     { return new Pedal(*this);   }
      virtual ElementType type() const override { return ElementType::PEDAL; }
      virtual void read(XmlReader&) override;
      virtual void read300(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;
      LineSegment* createLineSegment();
      virtual void setYoff(qreal) override;
      virtual QVariant propertyDefault(Pid propertyId) const override;

      friend class PedalLine;
      };

}     // namespace Ms
#endif

