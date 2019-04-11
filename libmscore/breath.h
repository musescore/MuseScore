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

#ifndef __BREATH_H__
#define __BREATH_H__

#include "element.h"

namespace Ms {

enum class SymId;

//---------------------------------------------------------
//   BreathType
//---------------------------------------------------------

struct BreathType {
      SymId id;
      bool isCaesura;
      qreal pause;
      };

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath final : public Element {
      qreal _pause;
      SymId _symId;

   public:
      Breath(Score* s);
      virtual ElementType type() const override { return ElementType::BREATH; }
      virtual Breath* clone() const override    { return new Breath(*this); }

      virtual qreal mag() const override;

      void setSymId(SymId id)          { _symId = id; }
      SymId symId() const              { return _symId; }
      qreal pause() const              { return _pause; }
      void setPause(qreal v)           { _pause = v; }

      Segment* segment() const         { return (Segment*)parent(); }

      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;
      virtual QPointF pagePos() const override;      ///< position in page coordinates

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      virtual QString accessibleInfo() const override;

      bool isCaesura() const;

      static const std::vector<BreathType> breathList;
      };


}     // namespace Ms
#endif

