//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __LETRING_H__
#define __LETRING_H__

#include "textlinebase.h"

namespace Ms {

class LetRing;

//---------------------------------------------------------
//   @@ LetRingSegment
//---------------------------------------------------------

class LetRingSegment final : public TextLineBaseSegment {
   public:
      LetRingSegment(Spanner* sp, Score* s) : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)  { }
      virtual ElementType type() const override       { return ElementType::LET_RING_SEGMENT; }
      virtual LetRingSegment* clone() const override  { return new LetRingSegment(*this);    }
      LetRing* letRing() const                        { return (LetRing*)spanner();          }
      virtual void layout() override;

      friend class LetRing;
      };

//---------------------------------------------------------
//   @@ LetRing
//---------------------------------------------------------

class LetRing final : public TextLineBase {
   protected:
      QPointF linePos(Grip, System**) const override;

   public:
      LetRing(Score* s);
      virtual LetRing* clone() const override   { return new LetRing(*this);   }
      virtual ElementType type() const override { return ElementType::LET_RING; }
      virtual void read(XmlReader&) override;
//      virtual void write(XmlWriter& xml) const override;
      LineSegment* createLineSegment();
      virtual QVariant propertyDefault(Pid propertyId) const override;
      virtual Sid getPropertyStyle(Pid) const override;
      };

}     // namespace Ms
#endif

