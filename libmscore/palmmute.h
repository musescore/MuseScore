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

      Sid getPropertyStyle(Pid) const override;

   public:
      PalmMuteSegment(Spanner* sp, Score* s) : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)  { }

      ElementType type() const override       { return ElementType::PALM_MUTE_SEGMENT; }
      PalmMuteSegment* clone() const override { return new PalmMuteSegment(*this);    }

      PalmMute* palmMute() const              { return (PalmMute*)spanner();          }

      void layout() override;

      friend class PalmMute;
      };

//---------------------------------------------------------
//   @@ PalmMute
//---------------------------------------------------------

class PalmMute final : public TextLineBase {

      Sid getPropertyStyle(Pid) const override;

   protected:
      QPointF linePos(Grip, System**) const override;

   public:
      PalmMute(Score* s);

      PalmMute* clone() const override  { return new PalmMute(*this);   }
      ElementType type() const override { return ElementType::PALM_MUTE; }

      void read(XmlReader&) override;
//      virtual void write(XmlWriter& xml) const override;

      LineSegment* createLineSegment() override;
      QVariant propertyDefault(Pid propertyId) const override;

      friend class PalmMuteLine;
      };

}     // namespace Ms
#endif

