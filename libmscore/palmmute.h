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

#ifndef __PALM_MUTE_H__
#define __PALM_MUTE_H__

#include "textlinebase.h"

namespace Ms {

class PalmMute;

//---------------------------------------------------------
//   @@ PalmMuteSegment
//---------------------------------------------------------

class PalmMuteSegment final : public TextLineBaseSegment {

      virtual Sid getPropertyStyle(Pid) const override;

   public:
      PalmMuteSegment(Score* s) : TextLineBaseSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)  { }
      virtual ElementType type() const override       { return ElementType::PALM_MUTE_SEGMENT; }
      virtual PalmMuteSegment* clone() const override { return new PalmMuteSegment(*this);    }
      PalmMute* palmMute() const                      { return (PalmMute*)spanner();          }
      virtual void layout() override;

      friend class PalmMute;
      };

//---------------------------------------------------------
//   @@ PalmMute
//---------------------------------------------------------

class PalmMute final : public TextLineBase {

      virtual Sid getPropertyStyle(Pid) const override;

   protected:
      QPointF linePos(Grip, System**) const override;

   public:
      PalmMute(Score* s);
      virtual PalmMute* clone() const override  { return new PalmMute(*this);   }
      virtual ElementType type() const override { return ElementType::PALM_MUTE; }
      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;
      LineSegment* createLineSegment();
      virtual void setYoff(qreal) override;
      virtual QVariant propertyDefault(Pid propertyId) const override;

      friend class PalmMuteLine;
      };

}     // namespace Ms
#endif

